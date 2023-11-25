//! Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//! See LICENSE for details.

#include "NWD.hpp"
#include "Maxwell.hpp"
#include "Pascal.hpp"
#include "PatcherPlus.hpp"
#include <Headers/kern_api.hpp>
#include <Headers/kern_devinfo.hpp>

static const char *pathNVDAStartup = "/System/Library/Extensions/NVDAStartup.kext/Contents/MacOS/NVDAStartup";
static const char *pathGeForce = "/System/Library/Extensions/GeForce.kext/Contents/MacOS/GeForce";
static const char *pathNVDAGK100Hal = "/System/Library/Extensions/NVDAGK100Hal.kext/Contents/MacOS/NVDAGK100Hal";
static const char *pathNVDAResman = "/System/Library/Extensions/NVDAResman.kext/Contents/MacOS/NVDAResman";

//! Yes that's the CFBundleIdentfier, it breaks NVIDIA's convention. Great job once again NVIDIA.
static KernelPatcher::KextInfo kextNVDAStartup {"com.apple.nvidia.NVDAStartup", &pathNVDAStartup, 1, {}, {},
    KernelPatcher::KextInfo::Unloaded};
static KernelPatcher::KextInfo kextGeForce {"com.apple.GeForce", &pathGeForce, 1, {}, {},
    KernelPatcher::KextInfo::Unloaded};
static KernelPatcher::KextInfo kextNVDAGK100Hal {"com.apple.nvidia.driver.NVDAGK100Hal", &pathNVDAGK100Hal, 1, {}, {},
    KernelPatcher::KextInfo::Unloaded};
static KernelPatcher::KextInfo kextNVDAResman {"com.apple.nvidia.driver.NVDAResman", &pathNVDAResman, 1, {}, {},
    KernelPatcher::KextInfo::Unloaded};

//! How do the drivers attach?
//! NVDAStartup allocates IOServices, 'NVDAgl', 'NVDAinitgl', 'NVDAHal'
//! NVArch and NVType dictate what kernel extensions load
//!
//! Load table:
//! NVType | Official | Web
//! -------------------------
//! NVArch |  GF100   | GF100
//! NVArch |  GK100   | GK100
//! NVArch |          | GM100
//! NVArch |          | GP100
//! NVArch |          | GV100
//!
//! NVDAStartup sets NVArch or NVType to 'Unsupported' if the GPU doesn't match the PMC_BOOT_42 value expected
//!
//! The kext 'GeForce' attaches when 'NVDAgl' has NVDAType 'Official' and either of the two NVArch architectures listed

static Pascal pascal;
static Maxwell maxwell;

NWD *NWD::callback = nullptr;

void NWD::init() {
    SYSLOG("NWD", "-- CHEFKISS INTERNAL --");
    callback = this;

    lilu.onKextLoadForce(&kextNVDAStartup);
	lilu.onKextLoadForce(&kextNVDAGK100Hal);
	lilu.onKextLoadForce(&kextNVDAResman);
    lilu.onKextLoadForce(&kextGeForce);

    lilu.onPatcherLoadForce(
        [](void *user, KernelPatcher &patcher) { static_cast<NWD *>(user)->processPatcher(patcher); }, this);
    lilu.onKextLoadForce(
        nullptr, 0,
        [](void *user, KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size) {
            static_cast<NWD *>(user)->processKext(patcher, index, address, size);
        },
        this);
	
	pascal.init();
	maxwell.init();
}

void NWD::processPatcher(KernelPatcher &) {
    auto *devInfo = DeviceInfo::create();
    if (devInfo) {
        devInfo->processSwitchOff();

        char name[256] = {0};
        for (size_t i = 0; i < devInfo->videoExternal.size(); i++) {
            auto *device = OSDynamicCast(IOPCIDevice, devInfo->videoExternal[i].video);
            if (!device) { continue; }
            snprintf(name, arrsize(name), "GFX%zu", i);
            WIOKit::renameDevice(device, name);
            WIOKit::awaitPublishing(device);
            if (WIOKit::readPCIConfigValue(this->gpu, WIOKit::kIOPCIConfigVendorID) == WIOKit::VendorID::NVIDIA) {
                this->gpu = device;
                break;
            }
        }
        PANIC_COND(!this->gpu, "NWD", "No compatible GPU found");

        this->deviceId = WIOKit::readPCIConfigValue(this->gpu, WIOKit::kIOPCIConfigDeviceID);

        DeviceInfo::deleter(devInfo);
    } else {
        SYSLOG("NWD", "Failed to create DeviceInfo");
    }
}

void NWD::setArchitecture() {
    this->gpu->setMemoryEnable(true);
    IOMemoryMap *map = this->gpu->mapDeviceMemoryWithRegister(0x10);
    volatile UInt8 *addr = reinterpret_cast<UInt8 *>(map->getVirtualAddress());

    UInt8 pmcBoot42 = (addr[0xA03] & 0x1F) ?: (addr[0x3] & 0x1F);
    DBGLOG("NWD", "PMC_BOOT_42: 0x%X", pmcBoot42);

    if ((pmcBoot42 - 0x11) < 0x2) {
        this->gfxGen = NVGen::GM100;
        DBGLOG("NWD", "Identified GPU as GM100");
    } else if (pmcBoot42 == 0x13) {
        this->gfxGen = NVGen::GP100;
        DBGLOG("NWD", "Identified GPU as GP100");
    } else {
        PANIC("NWD", "Failed to identify GPU");
    }

    OSSafeReleaseNULL(map);
    this->gpu->setMemoryEnable(false);
}

void NWD::processKext(KernelPatcher &patcher, size_t id, mach_vm_address_t address, size_t size) {
    if (kextNVDAStartup.loadIndex == id) {
        callback->setArchitecture();
        const LookupPatchPlus patch {&kextNVDAStartup, kNVDAStartupForceGK100Original, kNVDAStartupForceGK100Patched,
            1};
        PANIC_COND(!patch.apply(patcher, address, size), "NWD", "Failed to force GK100 NVArch!");
    } else if (kextNVDAGK100Hal.loadIndex == id) {
        // Hopefully should encourage NVDAResmanWeb and NVDAGP100HalWeb to load
        RouteRequestPlus request {"__ZN12NVDAGK100HAL5probeEP9IOServicePi", wrapProbeFailButChangeNVTypeAndArch};
        PANIC_COND(!request.route(patcher, id, address, size), "NWD", "Failed to route the GK100Hal symbol!");
    } else if (kextNVDAResman.loadIndex == id) {
        // Hopefully should encourage NVDAResmanWeb and NVDAGP100HalWeb to load
        RouteRequestPlus request {"__ZN4NVDA5probeEP9IOServicePi", wrapProbeFailButChangeNVTypeAndArch};
        PANIC_COND(!request.route(patcher, id, address, size), "NWD", "Failed to route the NVDAResman symbol!");
    } else if (kextGeForce.loadIndex == id) {
		if (this->gfxGen == NVGen::GP100 || this->gfxGen == NVGen::GV100) {
			//! GV10X uses Pascal logic? Investigation required.
            pascal.processKext(patcher, id, address, size);
            DBGLOG("NWD", "Processed GeForce");
		} else {
			maxwell.processKext(patcher, id, address, size);
			DBGLOG("NWD", "Processed GeForce");
		}
    }
}

IOService *NWD::wrapProbeFailButChangeNVTypeAndArch(IOService *, IOService *provider) {
    OSString *value = OSString::withCString("Web");
    provider->setProperty("NVDAType", value);
    OSSafeReleaseNULL(value);
    value = OSString::withCString(callback->getArchString());
    provider->setProperty("NVDAArch", value);
    OSSafeReleaseNULL(value);
    return nullptr;
}

IOService *NWD::wrapProbeFailButRevert(IOService *, IOService *provider) {
    OSString *value = OSString::withCString("Official");
    provider->setProperty("NVDAType", value);
    OSSafeReleaseNULL(value);
    value = OSString::withCString("GK100");
    provider->setProperty("NVDAArch", value);
    OSSafeReleaseNULL(value);
    return nullptr;
}

//! NVDAGK100Hal sobbing over losing its job.
IOService *NWD::wrapFunctionReturnZero(IOService *, IOService *) { return nullptr; }
