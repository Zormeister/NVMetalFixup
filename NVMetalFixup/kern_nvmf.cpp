//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "kern_nvmf.hpp"
#include <Headers/kern_api.hpp>
#include <sys/sysctl.h>

static const char *pathIOAccelFamily2 =
    "/System/Library/Extensions/IOAcceleratorFamily2.kext/Contents/MacOS/IOAcceleratorFamily2";
static const char *pathGeForceWeb[] = {
    "/Library/Extensions/GeForceWeb.kext/Contents/MacOS/GeForceWeb",
    "/System/Library/Extensions/GeForceWeb.kext/Contents/MacOS/GeForceWeb",
};
static const char *pathNVDAStartupWeb[] = {
    "/Library/Extensions/NVDAStartupWeb.kext/Contents/MacOS/NVDAStartupWeb",
    "/System/Library/Extensions/NVDAStartupWeb.kext/Contents/MacOS/NVDAStartupWeb",
};
static const char *pathIONDRVSupport = "/System/Library/Extensions/IONDRVSupport.kext/IONDRVSupport";

static KernelPatcher::KextInfo kextIOAcceleratorFamily2 {"com.apple.iokit.IOAcceleratorFamily2", &pathIOAccelFamily2, 1,
    {}, {}, KernelPatcher::KextInfo::Unloaded};
static KernelPatcher::KextInfo kextGeForceWeb {"com.nvidia.web.GeForceWeb", pathGeForceWeb, arrsize(pathGeForceWeb), {},
    {}, KernelPatcher::KextInfo::Unloaded};
static KernelPatcher::KextInfo kextNVDAStartupWeb {"com.nvidia.NVDAStartupWeb", pathNVDAStartupWeb,
    arrsize(pathNVDAStartupWeb), {}, {}, KernelPatcher::KextInfo::Unloaded};
static KernelPatcher::KextInfo kextIONDRVSupport {"com.apple.iokit.IONDRVSupport", &pathIONDRVSupport, 1, {}, {},
    KernelPatcher::KextInfo::Unloaded};

NVMF *NVMF::callback = nullptr;

void NVMF::init() {
    callback = this;
    SYSLOG("nvmf", "Please don't support tonymacx86.com!");

    lilu.onKextLoadForce(
        nullptr, 0,
        [](void *user, KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size) {
            static_cast<NVMF *>(user)->processKext(patcher, index, address, size);
        },
        this);
    lilu.onKextLoadForce(&kextIOAcceleratorFamily2);
    lilu.onKextLoadForce(&kextGeForceWeb);
    lilu.onKextLoadForce(&kextNVDAStartupWeb);
    lilu.onKextLoadForce(&kextIONDRVSupport);
}

void NVMF::processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size) {
    if (kextIOAcceleratorFamily2.loadIndex == index) {
        KernelPatcher::SolveRequest solveRequests[] = {
            {"__ZTV18IOAccelStatistics2", this->orgIOAccelStatisticsVTable},
            {"__ZTV19IOAccelCommandQueue", this->orgIOAccelCommandQueueVTable},
            {"__ZTV22IOGraphicsAccelerator2", this->orgIOGraphicsAccelVTable},
        };
        PANIC_COND(!patcher.solveMultiple(index, solveRequests, address, size), "nvmf",
            "Failed to solve IOAcceleratorFamily2 symbols");

        KernelPatcher::RouteRequest requests[] = {
            {"__ZN24IOAccelSharedUserClient212new_resourceEP22IOAccelNewResourceArgsP28IOAccelNewResourceReturnDatayPj",
                wrapSharedUCNewResource, this->orgSharedUCNewResource},
            {"__"
             "ZN16IOAccelResource222newResourceWithOptionsEP22IOGraphicsAccelerator2P14IOAccelShared215eIOAccelResTypey"
             "jyPyj",
                wrapNewResourceWithOptions, this->orgNewResourceWithOptions},
        };
        PANIC_COND(patcher.routeMultipleLong(index, requests, address, size) != KERN_SUCCESS, "nvmf",
            "Failed to route IOAcceleratorFamily2 symbols");
    } else if (kextGeForceWeb.loadIndex == index) {
        mach_vm_address_t *orgNVStatisticsVTable = nullptr, *orgNVCommandQueueVTable = nullptr,
                          *orgNVAccelParentVTable = nullptr;

        KernelPatcher::SolveRequest solveRequests[] = {
            {"__ZTV12nvStatistics", orgNVStatisticsVTable},
            {"__ZTV14nvCommandQueue", orgNVCommandQueueVTable},
            {"__ZTV19nvAcceleratorParent", orgNVAccelParentVTable},
        };
        PANIC_COND(!patcher.solveMultiple(index, solveRequests, address, size), "nvmf",
            "Failed to solve GeForceWeb symbols");

        KernelPatcher::RouteRequest requests[] = {
            {"__ZN19nvAcceleratorParent18SetAccelPropertiesEv", wrapSetAccelPropertiesWeb,
                this->orgSetAccelPropertiesWeb},
        };
        PANIC_COND(patcher.routeMultipleLong(index, requests, address, size) != KERN_SUCCESS, "nvmf",
            "Failed to route GeForceWeb symbols");

        /**
         * Apple added a few new methods. This is fine; they added them over their _RESERVED placeholders.
         */
        PANIC_COND(MachInfo::setKernelWriting(true, KernelPatcher::kernelWriteLock) != KERN_SUCCESS, "nvmf",
            "Failed to enable kernel writing");
        memcpy(orgNVStatisticsVTable + 39, callback->orgIOAccelStatisticsVTable + 39, sizeof(mach_vm_address_t) * 4);
        memcpy(orgNVCommandQueueVTable + 325, callback->orgIOAccelCommandQueueVTable + 325,
            sizeof(mach_vm_address_t) * 2);
        memcpy(orgNVAccelParentVTable + 340, callback->orgIOGraphicsAccelVTable + 340, sizeof(mach_vm_address_t));
        MachInfo::setKernelWriting(false, KernelPatcher::kernelWriteLock);
    } else if (kextNVDAStartupWeb.loadIndex == index) {
        KernelPatcher::RouteRequest request("__ZN14NVDAStartupWeb5probeEP9IOServicePi", wrapStartupWebProbe,
            orgStartupWebProbe);
        PANIC_COND(patcher.routeMultiple(index, &request, 1, address, size) != KERN_SUCCESS, "nvmf",
            "Failed to route NVDAStartupWeb symbols");
    } else if (kextIONDRVSupport.loadIndex == index) {
        KernelPatcher::RouteRequest request("__ZN17IONDRVFramebuffer10_doControlEPS_jPv", wrapNdrvDoControl,
            orgNdrvDoControl);
        PANIC_COND(patcher.routeMultiple(index, &request, 1, address, size) != KERN_SUCCESS, "nvmf",
            "Failed to route IONDRVSupport symbols");
    }
}

static bool inSharedUCNewResource = false;

void *NVMF::wrapSharedUCNewResource(void *that, void *param1, void *param2, uint64_t param3, uint32_t *param4) {
    inSharedUCNewResource = true;
    auto *ret =
        FunctionCast(wrapSharedUCNewResource, callback->orgSharedUCNewResource)(that, param1, param2, param3, param4);
    inSharedUCNewResource = false;
    return ret;
}

void *NVMF::wrapNewResourceWithOptions(void *accel, void *accelShared, uint32_t resType, IOByteCount count,
    IOOptionBits options, IOByteCount param6, mach_vm_address_t *param7, uint32_t namespaceId) {
    // Apple added a namespace ID parameter
    if (inSharedUCNewResource) {
        return FunctionCast(wrapNewResourceWithOptions, callback->orgNewResourceWithOptions)(accel, accelShared,
            resType, count, options, param6, param7, namespaceId);
    }
    return FunctionCast(wrapNewResourceWithOptions, callback->orgNewResourceWithOptions)(accel, accelShared, resType,
        count, options, param6, param7, 0);
}

void NVMF::wrapSetAccelPropertiesWeb(IOService *that) {
    DBGLOG("nvmf", "nvAcceleratorParent::SetAccelProperties <<");
    FunctionCast(wrapSetAccelPropertiesWeb, callback->orgSetAccelPropertiesWeb)(that);

    if (!that->getProperty("IOVARendererID")) {
        uint8_t rendererId[] = {0x08, 0x00, 0x04, 0x01};
        that->setProperty("IOVARendererID", rendererId, arrsize(rendererId));
        DBGLOG("nvmf", "Set 'IOVARendererID' to 08 00 04 01");
    }

    if (!that->getProperty("IOVARendererSubID")) {
        uint8_t rendererSubId[] = {0x03, 0x00, 0x00, 0x00};
        that->setProperty("IOVARendererSubID", rendererSubId, arrsize(rendererSubId));
        DBGLOG("nvmf", "Set 'IOVARendererSubID' to value 03 00 00 00");
    }

    auto gfx = that->getParentEntry(gIOServicePlane);
    int gl = gfx && gfx->getProperty("disable-metal");
    PE_parse_boot_argn("ngfxgl", &gl, sizeof(gl));

    if (gl) {
        DBGLOG("nvmf", "Disabling metal support");
        that->removeProperty("MetalPluginClassName");
        that->removeProperty("MetalPluginName");
        that->removeProperty("MetalStatisticsName");
    }
    DBGLOG("nvmf", "nvAcceleratorParent::SetAccelProperties >> (void)");
}

IOService *NVMF::wrapStartupWebProbe(IOService *that, IOService *provider, SInt32 *score) {
    DBGLOG("nvmf", "NVDAStartupWeb::probe <<");

    char osversion[40] = {0};
    size_t size = arrsize(osversion);
    sysctlbyname("kern.osversion", osversion, &size, NULL, 0);
    DBGLOG("nvmf", "Ignoring driver compatibility requirements with %s OS", osversion);
    that->setProperty("NVDARequiredOS", osversion);

    return FunctionCast(wrapStartupWebProbe, callback->orgStartupWebProbe)(that, provider, score);
}

// This is a hack to let us access protected properties.
struct NDRVFramebufferViewer : public IONDRVFramebuffer {
    static UInt32 &getState(IONDRVFramebuffer *fb) { return static_cast<NDRVFramebufferViewer *>(fb)->ndrvState; }
};

IOReturn NVMF::wrapNdrvDoControl(IONDRVFramebuffer *fb, UInt32 code, void *params) {
    // Suppress debug(?) noise from IONDRVFramebuffer::_doControl( IONDRVFramebuffer * fb, UInt32 code, void * params )
    // when function was compiled with #define IONDRVCHECK 1.
    // Ref: https://opensource.apple.com/source/IOGraphics/IOGraphics-585/IONDRVSupport/IONDRVFramebuffer.cpp.auto.html

    if (code == cscSetHardwareCursor || code == cscDrawHardwareCursor) {
        if (NDRVFramebufferViewer::getState(fb) == 0) return kIOReturnNotOpen;

        IONDRVControlParameters pb;
        pb.code = code;
        pb.params = params;
        return fb->doDriverIO(/*ID*/ 1, &pb, kIONDRVControlCommand, kIONDRVImmediateIOCommandKind);
    }

    return FunctionCast(wrapNdrvDoControl, callback->orgNdrvDoControl)(fb, code, params);
}
