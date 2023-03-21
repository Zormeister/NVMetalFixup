//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "kern_nvmf.hpp"
#include <Headers/kern_api.hpp>

static const char *pathIOAccelFamily2 =
    "/System/Library/Extensions/IOAcceleratorFamily2.kext/Contents/MacOS/IOAcceleratorFamily2";
static const char *pathGeForceWeb = "/System/Library/Extensions/GeForceWeb.kext/Contents/MacOS/GeForceWeb";

static KernelPatcher::KextInfo kextIOAccelFamily2 {"com.apple.iokit.IOAcceleratorFamily2", &pathIOAccelFamily2, 1, {},
    {}, KernelPatcher::KextInfo::Unloaded};
static KernelPatcher::KextInfo kextGeForceWeb {"com.nvidia.web.GeForceWeb", &pathGeForceWeb, 1, {}, {},
    KernelPatcher::KextInfo::Unloaded};

NVMF *NVMF::callback {nullptr};

void NVMF::init() {
    callback = this;
    SYSLOG("nvmf", "Please don't support tonymacx86.com!");

    lilu.onKextLoadForce(
        nullptr, 0,
        [](void *user, KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size) {
            static_cast<NVMF *>(user)->processKext(patcher, index, address, size);
        },
        this);
    lilu.onKextLoadForce(&kextIOAccelFamily2);
    lilu.onKextLoadForce(&kextGeForceWeb);
}

void NVMF::processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size) {
    if (kextIOAccelFamily2.loadIndex == index) {
        KernelPatcher::SolveRequest solveRequests[] = {
            {"__ZTV18IOAccelStatistics2", this->orgIOAccelStatisticsVTable},
            {"__ZTV19IOAccelCommandQueue", this->orgIOAccelCommandQueueVTable},
            {"__ZTV22IOGraphicsAccelerator2", this->orgIOGraphicsAccelVTable},
        };
        SYSLOG_COND(!patcher.solveMultiple(index, solveRequests, address, size), "nvmf",
            "Failed to solve IOAcceleratorFamily2 symbols");

        KernelPatcher::RouteRequest requests[] = {
            {"__"
             "ZN16IOAccelResource222newResourceWithOptionsEP22IOGraphicsAccelerator2P14IOAccelShared215eIOAccelResTypey"
             "jyPyj",
                wrapNewResourceWithOptions, this->orgNewResourceWithOptions},
        };
        SYSLOG_COND(!patcher.routeMultipleLong(index, requests, address, size), "nvmf",
            "Failed to route IOAcceleratorFamily2 symbols");
    } else if (kextGeForceWeb.loadIndex == index) {
        mach_vm_address_t *orgNVStatisticsVTable {nullptr}, *orgNVCommandQueueVTable {nullptr},
            *orgNVAccelParentVTable {nullptr};

        KernelPatcher::SolveRequest solveRequests[] = {
            {"__ZTV12nvStatistics", orgNVStatisticsVTable},
            {"__ZTV14nvCommandQueue", orgNVCommandQueueVTable},
            {"__ZTV19nvAcceleratorParent", orgNVAccelParentVTable},
        };
        SYSLOG_COND(!patcher.solveMultiple(index, solveRequests, address, size, true, true), "nvmf",
            "Failed to solve GeForceWeb symbols");

        /**
         * Apple added a few new methods. This is fine; they added them over their _RESERVED placeholders.
         */
        if (orgNVStatisticsVTable && callback->orgIOAccelStatisticsVTable) {
            DBGLOG("nvmf", "Patching nvStatistics VTable");
            memcpy(orgNVStatisticsVTable + 39, callback->orgIOAccelStatisticsVTable + 39, sizeof(mach_vm_address_t) * 4);
        }
        if (orgNVCommandQueueVTable && callback->orgIOAccelCommandQueueVTable) {
            DBGLOG("nvmf", "Patching nvCommandQueue VTable");
            memcpy(orgNVCommandQueueVTable + 325, callback->orgIOAccelCommandQueueVTable + 325,
            sizeof(mach_vm_address_t) * 2);
        }
        if (orgNVAccelParentVTable && callback->orgIOGraphicsAccelVTable) {
            DBGLOG("nvmf", "Patching nvAcceleratorParent VTable");
            memcpy(orgNVAccelParentVTable + 340, callback->orgIOGraphicsAccelVTable + 340, sizeof(mach_vm_address_t));
        }
    }
}

void *NVMF::wrapNewResourceWithOptions(void *accel, void *accelShared, uint32_t resType, IOByteCount count,
    IOOptionBits options, IOByteCount param6, mach_vm_address_t *param7) {
    return reinterpret_cast<t_newResourceWithOptionsNew>(callback->orgNewResourceWithOptions)(accel, accelShared,
        resType, count, options, param6, param7, 0);    // Apple added a namespace ID parameter.
}
