//  Copyright © 2022-2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#ifndef kern_nvmf_hpp
#define kern_nvmf_hpp
#include <Headers/kern_patcher.hpp>

class NVMF {
    public:
    static NVMF *callback;

    void init();

    private:
    void processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size);

    mach_vm_address_t *orgIOAccelStatisticsVTable {nullptr};
    mach_vm_address_t *orgIOAccelCommandQueueVTable {nullptr};
    mach_vm_address_t *orgIOGraphicsAccelVTable {nullptr};
};

#endif /* kern_nvmf.hpp */
