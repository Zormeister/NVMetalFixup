//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "kern_nvmf.hpp"
#include <Headers/kern_api.hpp>
#include <Headers/kern_devinfo.hpp>
#include <IOKit/IODeviceTreeSupport.h>

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
        KernelPatcher::SolveRequest requests[] = {
            {"__ZNK19IOAccelDisplayPipe214getFramebufferEv", this->orgIOAccelDisplayPipeGetFramebuffer},
            {"__ZNK16IOAccelVidMemory12getDirtySizeEv", this->orgIOAccelVidMemoryGetDirtySize},
            {"__ZNK15IOAccelContext215contextModeBitsEv", this->orgIOAccelContextContextModeBits},
            {"__ZNK15IOAccelContext216isTripleBufferedEv", this->orgIOAccelContextIsTripleBuffered},
            {"__ZNK15IOAccelSurface216getSurfaceBufferE20eIOAccelGLBufferType",
                this->orgIOAccelSurfaceGetSurfaceBuffer},
            {"__ZNK22IOAccelDisplayMachine219isFramebufferActiveEj",
                this->orgIOAccelDisplayMachine2IsFramebufferActive},
            {"__ZN18IOAccelStatistics227_RESERVEDIOAccelStatistics4Ev",
                this->orgIOAccelStatistics2_RESERVEDIOAccelStatistics4},
            {"__ZNK22IOAccelDisplayMachine214getDisplayPipeEj", this->orgIOAccelDisplayMachine2GetDisplayPipe},
            {"__ZNK17IOAccelGLContext220requiresBackingStoreEv", this->orgIOAccelGLContext2RequiresBackingStore},
            {"__ZNK17IOAccelGLContext215contextModeBitsEv", this->orgIOAccelGLContext2ContextModeBits},
            {"__ZNK22IOAccelDisplayMachine217getScanoutSurfaceEj", this->orgIOAccelDisplayMachineGetScanoutSurface},
            {"__ZNK22IOAccelDisplayMachine220getFullScreenSurfaceEj",
                this->orgIOAccelDisplayMachine2GetFullScreenSurface},
            {"__ZNK17IOAccelGLContext217getSurfaceReqBitsEv", this->orgIOAccelGLContextGetSurfaceReqBits},
            {"__ZNK17IOAccelGLContext216isTripleBufferedEv", this->orgIOAccelGLContextIsTripleBuffered},
            {"__ZN19IOAccelCommandQueue19retireCommandBufferEP17IOAccelEventFence",
                this->orgIOAccelCommandQueueRetireCommandBuffer},
            {"__ZNK15IOAccelContext219allowsExclusiveModeEv", this->orgIOAccelContextAllowsExclusiveMode},
            {"__ZNK16IOAccelResource224getClientSharedROPrivateEv", this->orgIOAccelResourceGetClientSharedROPrivate},
            {"__ZNK16IOAccelResource224getClientSharedRWPrivateEv", this->orgIOAccelResourceGetClientSharedRWPrivate},
            {"__ZNK14IOAccelShared217lookupDeviceShmemEj", this->orgIOAccelSharedLookupDeviceShmem},
            {"__ZNK15IOAccelContext220requiresBackingStoreEv", this->orgIOAccelContextRequiresBackingStore},
            {"__ZNK16IOAccelSysMemory12getDirtySizeEv", this->orgIOAccelSysMemoryGetDirtySize},
            {"__ZNK17IOAccelGLContext219allowsExclusiveModeEv", this->orgIOAccelGLContextAllowsExclusiveMode},
            {"__ZNK16IOAccelVidMemory15getResidentSizeEv", this->orgIOAccelVidMemoryGetResidentSize},
            {"__ZN18IOAccelStatistics227_RESERVEDIOAccelStatistics4Ev",
                this->orgIOAccelStatistics2_RESERVEDIOAccelStatistics4},
            {"__ZNK15IOAccelContext217getSurfaceReqBitsEv", this->orgIOAccelContextGetSurfaceReqBits},
            {"__ZN19IOAccelCommandQueue29_RESERVEDIOAccelCommandQueue3Ev",
                this->orgIOAccelCommandQueue_RESERVEDIOAccelCommandQueue3},
            {"__ZN22IOGraphicsAccelerator231_RESERVEDIOGraphicsAccelerator1Ev",
                this->orgIOGraphicsAccelerator_RESERVEDIOGraphicsAccelerator1},
            {"__ZNK22IOAccelDisplayMachine222getFramebufferResourceEjj",
                this->orgIOAccelDisplayMachineGetFramebufferResource},
            {"__ZNK16IOAccelSysMemory15getResidentSizeEv", this->orgIOAccelSysMemoryGetResidentSize},
            {"__ZNK17IOAccel2DContext219allowsExclusiveModeEv", this->orgIOAccel2DContextAllowsExclusiveMode},
            {"__ZNK19IOAccelDisplayPipe218getScanoutResourceEj", this->orgIOAccelDisplayPipeGetScanoutResource},
            {"__ZNK16IOAccelResource213requirePageonEv", this->orgIOAccelResourceRequirePageon},
            {"__ZNK22IOAccelDisplayMachine216getIOFramebufferEj", this->orgIOAccelDisplayMachineGetIOFramebuffer},
            {"__ZNK15IOAccelSurface227convertGLIndexToBufferIndexE20eIOAccelGLBufferType",
                this->orgIOAccelSurfaceConvertGLIndexToBufferIndex},
            {"__ZN19IOAccelCommandQueue29_RESERVEDIOAccelCommandQueue3Ev",
                this->orgIOAccelCommandQueue_RESERVEDIOAccelCommandQueue2},
            {"__"
             "ZN16IOAccelResource222newResourceWithOptionsEP22IOGraphicsAccelerator2P14IOAccelShared215eIOAccelResTypey"
             "jyPyj",
                this->orgIOAccelResourceNewResourceWithOptions},
            {"__ZNK17IOAccel2DContext217getSurfaceReqBitsEv", this->orgIOAccel2DContextGetSurfaceReqBits},
            {"__ZNK22IOAccelDisplayMachine219getFramebufferCountEv", this->orgIOAccelDisplayMachineGetFramebufferCount},
            {"__ZTV18IOAccelStatistics2", this->orgIOAccelStatisticsVTable},
            {"__ZTV19IOAccelCommandQueue", this->orgIOAccelCommandQueueVTable},
            {"__ZTV22IOGraphicsAccelerator2", this->orgIOGraphicsAccelVTable},
        };
        PANIC_COND(!patcher.solveMultiple(index, requests, address, size), "nvmf",
            "Failed to solve IOAccelFamily2 symbols");
    } else if (kextGeForceWeb.loadIndex == index) {
        mach_vm_address_t *orgNVStatisticsVTable {nullptr}, *orgNVCommandQueueVTable {nullptr},
            *orgNVAccelParent {nullptr};

        KernelPatcher::SolveRequest requests[] = {
            {"__ZTV12nvStatistics", orgNVStatisticsVTable},
            {"__ZTV14nvCommandQueue", orgNVCommandQueueVTable},
            {"__ZTV19nvAcceleratorParent", orgNVAccelParent},
        };
        PANIC_COND(!patcher.solveMultiple(index, requests, address, size), "nvmf",
            "Failed to solve GeForceWeb symbols");

        /**
         * Apple added a few new methods. This is fine; they added them over their _RESERVED placeholders.
         */
        memcpy(orgNVStatisticsVTable + 39, callback->orgIOAccelStatisticsVTable + 39, sizeof(mach_vm_address_t) * 4);
        memcpy(orgNVCommandQueueVTable + 325, callback->orgIOAccelCommandQueueVTable + 325,
            sizeof(mach_vm_address_t) * 2);
        memcpy(orgNVCommandQueueVTable + 340, callback->orgIOAccelCommandQueueVTable + 340, sizeof(mach_vm_address_t));
    }
}
