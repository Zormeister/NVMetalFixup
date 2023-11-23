//! Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//! See LICENSE for details.

#pragma once
#include "NWD.hpp"

using t_makeSpace = IOReturn (*)(void *that, UInt32 space);

class Pascal {
    public:
    static Pascal *callback;
    void init();
    void processKext(KernelPatcher &patcher, size_t id, mach_vm_address_t address, size_t size);

    private:
    t_makeSpace orgMakeSpace = {nullptr};

    static void wrapInitMemToMemCaps(void *that);
    static void wrapInvalidateMMU(void *that, void *fifoChannel);
    static void wrapFlushGlobalCache(void *that, void *fifoChannel, UInt32 flushMode);
};
