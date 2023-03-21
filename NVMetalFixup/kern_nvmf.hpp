//  Copyright © 2022-2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#ifndef kern_nvmf_hpp
#define kern_nvmf_hpp
#include <Headers/kern_patcher.hpp>

using t_newResourceWithOptionsNew = void *(*)(void *accel, void *accelShared, uint32_t resType, IOByteCount count,
    IOOptionBits options, IOByteCount param6, mach_vm_address_t *param7, uint32_t namespaceId);

class NVMF {
    public:
    static NVMF *callback;

    void init();

    private:
    void processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size);

    mach_vm_address_t *orgIOAccelStatisticsVTable {nullptr};
    mach_vm_address_t *orgIOAccelCommandQueueVTable {nullptr};
    mach_vm_address_t *orgIOGraphicsAccelVTable {nullptr};

    mach_vm_address_t orgNewResourceWithOptions {0};
    static void *wrapNewResourceWithOptions(void *accel, void *accelShared, uint32_t resType, IOByteCount count,
        IOOptionBits options, IOByteCount param6, mach_vm_address_t *param7);
};

#endif /* kern_nvmf.hpp */
