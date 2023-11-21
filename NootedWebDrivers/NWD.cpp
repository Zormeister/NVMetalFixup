//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//  See LICENSE for details.

#include "NWD.hpp"
#include "GeForce.hpp"
#include "PatcherPlus.hpp"
#include <Headers/kern_api.hpp>
#include <Headers/kern_devinfo.hpp>

static const char *pathGeForce = "/System/Library/Extensions/GeForce.kext/Contents/MacOS/GeForce";
static const char *pathNVDAStartup = "/System/Library/Extensions/NVDAStartup.kext/Contents/MacOS/NVDAStartup";
static const char *pathNVDAGK100Hal = "/System/Library/Extensions/NVDAGK100Hal.kext/Contents/MacOS/NVDAGK100Hal";
static const char *pathNVDAResman = "/System/Library/Extensions/NVDAResman.kext/Contents/MacOS/NVDAResman";

// Yes that's the CFBundleIdentfier, it breaks NVIDIA's convention. Great job once again NVIDIA.
static KernelPatcher::KextInfo kextGeForce {"com.apple.GeForce", &pathGeForce, 1, {}, {}, KernelPatcher::KextInfo::Unloaded};
static KernelPatcher::KextInfo kextNVDAStartup {"com.apple.nvidia.NVDAStartup", &pathNVDAStartup, 1, {}, {}, KernelPatcher::KextInfo::Unloaded};
static KernelPatcher::KextInfo kextNVDAGK100Hal {"com.apple.nvidia.driver.NVDAGK100Hal", &pathNVDAGK100Hal, 1, {}, {}, KernelPatcher::KextInfo::Unloaded};
static KernelPatcher::KextInfo kextNVDAResman {"com.apple.nvidia.driver.NVDAResman", &pathNVDAResman, 1, {}, {}, KernelPatcher::KextInfo::Unloaded};

NWD *NWD::callback = nullptr;

void NWD::init() {
    callback = this;
    SYSLOG("NWD", "-- CHEFKISS INTERNAL --");

    lilu.onKextLoadForce(
        nullptr, 0,
        [](void *user, KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size) {
            static_cast<NWD *>(user)->processKext(patcher, index, address, size);
        },
        this);
    lilu.onKextLoadForce(&kextGeForce);
	
	auto *devInfo = DeviceInfo::create();
	if (devInfo) {
		devInfo->processSwitchOff();

		

		char name[256] = {0};
		for (size_t i = 0; i < devInfo->videoExternal.size(); i++) {
			auto *device = OSDynamicCast(IOPCIDevice, devInfo->videoExternal[i].video);
			if (device->configRead16(kIOPCIConfigVendorID) != WIOKit::VendorID::NVIDIA) {
				SYSLOG("NWD", "PCI Vendor ID is not NVIDIA, ignoring...");
			} else {
				this->gpu = device;
			}
			if (device) {
				snprintf(name, arrsize(name), "GFX%zu", i);
				WIOKit::renameDevice(device, name);
				WIOKit::awaitPublishing(device);
			}
		}

		this->deviceId = WIOKit::readPCIConfigValue(this->gpu, WIOKit::kIOPCIConfigDeviceID);

		DeviceInfo::deleter(devInfo);
	} else {
		SYSLOG("NWD", "Failed to create DeviceInfo");
	}
}

void NWD::processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size) {
	DBGLOG("NWD", "TBD");
	if (kextNVDAGK100Hal.loadIndex == index) {
		RouteRequestPlus request {"__ZN12NVDAGK100HAL5probeEP9IOServicePi", wrapFunctionReturnZero};
		PANIC_COND(!request.route(patcher, index, address, size), "NWD", "Failed to route the GK100Hal symbol!");
	}
}

// NVDAGK100Hal sobbing over losing it's job.
size_t NWD::wrapFunctionReturnZero() { return 0; }
