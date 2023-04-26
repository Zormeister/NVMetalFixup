//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "kern_nvmf.hpp"
#include <Headers/kern_api.hpp>

static const char *pathIOAcceleratorFamily2 =
    "/System/Library/Extensions/IOAcceleratorFamily2.kext/Contents/MacOS/IOAcceleratorFamily2";
static const char *pathGeForceWeb[] = {
    "/Library/Extensions/GeForceWeb.kext/Contents/MacOS/GeForceWeb",
    "/System/Library/Extensions/GeForceWeb.kext/Contents/MacOS/GeForceWeb",
};

static KernelPatcher::KextInfo kextIOAcceleratorFamily2 {"com.apple.iokit.IOAcceleratorFamily2",
    &pathIOAcceleratorFamily2, 1, {}, {}, KernelPatcher::KextInfo::Unloaded};
static KernelPatcher::KextInfo kextGeForceWeb {"com.nvidia.web.GeForceWeb", pathGeForceWeb, arrsize(pathGeForceWeb), {},
    {}, KernelPatcher::KextInfo::Unloaded};

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
}

void NVMF::processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size) {
    if (kextIOAcceleratorFamily2.loadIndex == index) {
        KernelPatcher::RouteRequest requests[] = {
            {"__ZN24IOAccelSharedUserClient212new_resourceEP22IOAccelNewResourceArgsP28IOAccelNewResourceReturnDatayPj",
                wrapSharedUCNewResource, this->orgSharedUCNewResource},
            {"__"
             "ZN16IOAccelResource222newResourceWithOptionsEP22IOGraphicsAccelerator2P14IOAccelShared215eIOAccelResTypey"
             "jyPyj",
                wrapNewResourceWithOptions, this->orgNewResourceWithOptions},
        };
        patcher.clearError();
        PANIC_COND(patcher.routeMultipleLong(index, requests, address, size) != KERN_SUCCESS, "nvmf",
            "Failed to route IOAcceleratorFamily2 symbols");
    } else if (kextGeForceWeb.loadIndex == index) {
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
