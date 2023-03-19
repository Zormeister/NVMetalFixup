//  Copyright © 2022-2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "kern_nvmf.hpp"

#define SYMBOL_STUB(ORIGINAL, TARGET) \
    EXPORT extern "C" [[gnu::naked]] void ORIGINAL() { asm volatile("jmp *%0" ::"r"(NVMF::callback->TARGET)); }

SYMBOL_STUB(_ZN14IOAccelShared217lookupDeviceShmemEj, orgIOAccelSharedLookupDeviceShmem);
SYMBOL_STUB(_ZN15IOAccelContext215contextModeBitsEv, orgIOAccelContextContextModeBits);
SYMBOL_STUB(_ZN15IOAccelContext216isTripleBufferedEv, orgIOAccelContextIsTripleBuffered);
SYMBOL_STUB(_ZN15IOAccelContext217getSurfaceReqBitsEv, orgIOAccelContextGetSurfaceReqBits);
SYMBOL_STUB(_ZN15IOAccelContext219allowsExclusiveModeEv, orgIOAccelContextAllowsExclusiveMode);
SYMBOL_STUB(_ZN15IOAccelContext220requiresBackingStoreEv, orgIOAccelContextRequiresBackingStore);
SYMBOL_STUB(_ZN15IOAccelSurface216getSurfaceBufferE20eIOAccelGLBufferType, orgIOAccelSurfaceGetSurfaceBuffer);
SYMBOL_STUB(_ZN15IOAccelSurface227convertGLIndexToBufferIndexE20eIOAccelGLBufferType,
    orgIOAccelSurfaceConvertGLIndexToBufferIndex);
SYMBOL_STUB(_ZN16IOAccelResource213requirePageonEv, orgIOAccelResourceRequirePageon);
SYMBOL_STUB(
    _ZN16IOAccelResource222newResourceWithOptionsEP22IOGraphicsAccelerator2P14IOAccelShared215eIOAccelResTypeyjyPy,
    orgIOAccelResourceNewResourceWithOptions);
SYMBOL_STUB(_ZN16IOAccelResource224getClientSharedROPrivateEv, orgIOAccelResourceGetClientSharedROPrivate);
SYMBOL_STUB(_ZN16IOAccelResource224getClientSharedRWPrivateEv, orgIOAccelResourceGetClientSharedRWPrivate);
SYMBOL_STUB(_ZN16IOAccelSysMemory12getDirtySizeEv, orgIOAccelSysMemoryGetDirtySize);
SYMBOL_STUB(_ZN16IOAccelSysMemory15getResidentSizeEv, orgIOAccelSysMemoryGetResidentSize);
SYMBOL_STUB(_ZN16IOAccelVidMemory12getDirtySizeEv, orgIOAccelVidMemoryGetDirtySize);
SYMBOL_STUB(_ZN16IOAccelVidMemory15getResidentSizeEv, orgIOAccelVidMemoryGetResidentSize);
SYMBOL_STUB(_ZN17IOAccel2DContext217getSurfaceReqBitsEv, orgIOAccel2DContextGetSurfaceReqBits);
SYMBOL_STUB(_ZN17IOAccel2DContext219allowsExclusiveModeEv, orgIOAccel2DContextAllowsExclusiveMode);
SYMBOL_STUB(_ZN17IOAccelGLContext215contextModeBitsEv, orgIOAccelGLContext2ContextModeBits);
SYMBOL_STUB(_ZN17IOAccelGLContext216isTripleBufferedEv, orgIOAccelGLContextIsTripleBuffered);
SYMBOL_STUB(_ZN17IOAccelGLContext217getSurfaceReqBitsEv, orgIOAccelGLContextGetSurfaceReqBits);
SYMBOL_STUB(_ZN17IOAccelGLContext219allowsExclusiveModeEv, orgIOAccelGLContextAllowsExclusiveMode);
SYMBOL_STUB(_ZN17IOAccelGLContext220requiresBackingStoreEv, orgIOAccelGLContext2RequiresBackingStore);
SYMBOL_STUB(_ZN18IOAccelStatistics227_RESERVEDIOAccelStatistics0Ev, orgIOAccelStatistics2_RESERVEDIOAccelStatistics4);
SYMBOL_STUB(_ZN18IOAccelStatistics227_RESERVEDIOAccelStatistics1Ev, orgIOAccelStatistics2_RESERVEDIOAccelStatistics4);
SYMBOL_STUB(_ZN18IOAccelStatistics227_RESERVEDIOAccelStatistics2Ev, orgIOAccelStatistics2_RESERVEDIOAccelStatistics4);
SYMBOL_STUB(_ZN18IOAccelStatistics227_RESERVEDIOAccelStatistics3Ev, orgIOAccelStatistics2_RESERVEDIOAccelStatistics4);
SYMBOL_STUB(_ZN19IOAccelCommandQueue19retireCommandBufferEP17IOAccelBlockFence,
    orgIOAccelCommandQueueRetireCommandBuffer);
SYMBOL_STUB(_ZN19IOAccelCommandQueue29_RESERVEDIOAccelCommandQueue1Ev,
    orgIOAccelCommandQueue_RESERVEDIOAccelCommandQueue3);
SYMBOL_STUB(_ZN19IOAccelCommandQueue29_RESERVEDIOAccelCommandQueue2Ev,
    orgIOAccelCommandQueue_RESERVEDIOAccelCommandQueue2);
SYMBOL_STUB(_ZN19IOAccelDisplayPipe214getFramebufferEv, orgIOAccelDisplayPipeGetFramebuffer);
SYMBOL_STUB(_ZN22IOAccelDisplayMachine214getDisplayPipeEj, orgIOAccelDisplayMachine2GetDisplayPipe);
SYMBOL_STUB(_ZN22IOAccelDisplayMachine216getIOFramebufferEj, orgIOAccelDisplayMachineGetIOFramebuffer);
SYMBOL_STUB(_ZN22IOAccelDisplayMachine217getScanoutSurfaceEj, orgIOAccelDisplayMachineGetScanoutSurface);
SYMBOL_STUB(_ZN22IOAccelDisplayMachine218getScanoutResourceEjj, orgIOAccelDisplayPipeGetScanoutResource);
SYMBOL_STUB(_ZN22IOAccelDisplayMachine219getFramebufferCountEv, orgIOAccelDisplayMachineGetFramebufferCount);
SYMBOL_STUB(_ZN22IOAccelDisplayMachine219isFramebufferActiveEj, orgIOAccelDisplayMachine2IsFramebufferActive);
SYMBOL_STUB(_ZN22IOAccelDisplayMachine220getFullScreenSurfaceEj, orgIOAccelDisplayMachine2GetFullScreenSurface);
SYMBOL_STUB(_ZN22IOAccelDisplayMachine222getFramebufferResourceEjj, orgIOAccelDisplayMachineGetFramebufferResource);
SYMBOL_STUB(_ZN22IOGraphicsAccelerator231_RESERVEDIOGraphicsAccelerator0Ev,
    orgIOGraphicsAccelerator_RESERVEDIOGraphicsAccelerator1);
