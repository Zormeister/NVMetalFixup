//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//  See LICENSE for details.

#pragma once
#include <Headers/kern_patcher.hpp>
#include <Headers/kern_iokit.hpp>
#include <Headers/kern_util.hpp>
#include <IOKit/IOService.h>
#include <IOKit/ndrvsupport/IONDRVFramebuffer.h>
#include <IOKit/pci/IOPCIDevice.h>

const char *NVIDIAArchStrings[] = {"GF100", "GK100", "GM100", "GP100", "GV100"};

// Methodology:
// Step 1: Spoof everything but NVHal to GK100
// Step 2: Patch/Wrap in the differences
// Step 3: Force the correct HAL kernel extension
// Step 4: Profit???

class NWD {
    public:
    static NWD *callback;
	
	bool isMobileGpu = false;
	
	enum struct NVGen {
		GM100,
		GP100,
		GV100, // Needs GV100HALWeb binary from THAT Web Drivers package.
		Unknown,
	};
	
	UInt32 deviceId {0};
	IOPCIDevice *gpu {nullptr};
	NVGen gfxGen = NVGen::Unknown;

    void init();

    private:
    void processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size);
	void processPatcher(KernelPatcher &patcher);
	
	static size_t wrapFunctionReturnZero();
};
