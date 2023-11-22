//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//  See LICENSE for details.

#pragma once
#include "NWD.hpp"

class Pascal {
public:
	void init();
	
	bool processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size);
	
	Pascal *callback;
	
	static void wrapInitMemToMemCaps(void *that);
	static void wrapInvalidateMMU(void *that, void *fifo);
	static void wrapFlushGlobalCache(void *that, void *fifo, UInt32 flushMode);
};
