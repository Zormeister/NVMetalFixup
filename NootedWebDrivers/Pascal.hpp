//! Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//! See LICENSE for details.

#pragma once
#include "NWD.hpp"

class Pascal {
    public:
    void init();
    void processKext(KernelPatcher &patcher, size_t id, mach_vm_address_t slide, size_t size);

    private:
    static Pascal *callback;

    t_makeSpace orgMakeSpace {nullptr};
    mach_vm_address_t orgInitCaps {0};
    mach_vm_address_t orgGetFirstClassSupported {0};

    static void wrapInitMemToMemCaps(void *that);
    static void wrapInvalidateMMU(void *that, void *fifoChannel);
    static void wrapFlushGlobalCache(void *that, void *fifoChannel, UInt32 flushMode);
    static void wrapInitCaps(void *that);
    static UInt32 wrapGetFirstClassSupported(void *that, const UInt32 *classes, UInt32 numClasses);
};
