//  Copyright © 2022-2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#ifndef kern_nvmf_hpp
#define kern_nvmf_hpp
#include <Headers/kern_iokit.hpp>
#include <IOKit/acpi/IOACPIPlatformExpert.h>
#include <IOKit/graphics/IOFramebuffer.h>
#include <IOKit/pci/IOPCIDevice.h>

class NVMF {
    public:
    static NVMF *callback;

    void init();

    mach_vm_address_t orgIOAccelDisplayPipeGetFramebuffer {0};
    mach_vm_address_t orgIOAccelVidMemoryGetDirtySize {0};
    mach_vm_address_t orgIOAccelContextContextModeBits {0};
    mach_vm_address_t orgIOAccelContextIsTripleBuffered {0};
    mach_vm_address_t orgIOAccelSurfaceGetSurfaceBuffer {0};
    mach_vm_address_t orgIOAccelDisplayMachine2IsFramebufferActive {0};
    mach_vm_address_t orgIOAccelStatistics2_RESERVEDIOAccelStatistics4 {0};
    mach_vm_address_t orgIOAccelDisplayMachine2GetDisplayPipe {0};
    mach_vm_address_t orgIOAccelGLContext2RequiresBackingStore {0};
    mach_vm_address_t orgIOAccelGLContext2ContextModeBits {0};
    mach_vm_address_t orgIOAccelDisplayMachineGetScanoutSurface {0};
    mach_vm_address_t orgIOAccelDisplayMachine2GetFullScreenSurface {0};
    mach_vm_address_t orgIOAccelGLContextGetSurfaceReqBits {0};
    mach_vm_address_t orgIOAccelGLContextIsTripleBuffered {0};
    mach_vm_address_t orgIOAccelCommandQueueRetireCommandBuffer {0};
    mach_vm_address_t orgIOAccelContextAllowsExclusiveMode {0};
    mach_vm_address_t orgIOAccelResourceGetClientSharedROPrivate {0};
    mach_vm_address_t orgIOAccelResourceGetClientSharedRWPrivate {0};
    mach_vm_address_t orgIOAccelSharedLookupDeviceShmem {0};
    mach_vm_address_t orgIOAccelContextRequiresBackingStore {0};
    mach_vm_address_t orgIOAccelSysMemoryGetDirtySize {0};
    mach_vm_address_t orgIOAccelGLContextAllowsExclusiveMode {0};
    mach_vm_address_t orgIOAccelVidMemoryGetResidentSize {0};
    mach_vm_address_t orgIOAccelContextGetSurfaceReqBits {0};
    mach_vm_address_t orgIOAccelCommandQueue_RESERVEDIOAccelCommandQueue3 {0};
    mach_vm_address_t orgIOGraphicsAccelerator_RESERVEDIOGraphicsAccelerator1 {0};
    mach_vm_address_t orgIOAccelDisplayMachineGetFramebufferResource {0};
    mach_vm_address_t orgIOAccelSysMemoryGetResidentSize {0};
    mach_vm_address_t orgIOAccel2DContextAllowsExclusiveMode {0};
    mach_vm_address_t orgIOAccelDisplayPipeGetScanoutResource {0};
    mach_vm_address_t orgIOAccelResourceRequirePageon {0};
    mach_vm_address_t orgIOAccelDisplayMachineGetIOFramebuffer {0};
    mach_vm_address_t orgIOAccelSurfaceConvertGLIndexToBufferIndex {0};
    mach_vm_address_t orgIOAccelCommandQueue_RESERVEDIOAccelCommandQueue2 {0};
    mach_vm_address_t orgIOAccelResourceNewResourceWithOptions {0};
    mach_vm_address_t orgIOAccel2DContextGetSurfaceReqBits {0};
    mach_vm_address_t orgIOAccelDisplayMachineGetFramebufferCount {0};

    private:
    void processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size);

    mach_vm_address_t *orgIOAccelStatisticsVTable {nullptr};
    mach_vm_address_t *orgIOAccelCommandQueueVTable {nullptr};
    mach_vm_address_t *orgIOGraphicsAccelVTable {nullptr};
};

#endif /* kern_nvmf.hpp */
