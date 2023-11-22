//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//  See LICENSE for details.

#pragma once
#include <Headers/kern_patcher.hpp>
#include <Headers/kern_iokit.hpp>
#include <Headers/kern_util.hpp>
#include <IOKit/IOService.h>
#include <IOKit/ndrvsupport/IONDRVFramebuffer.h>
#include <IOKit/pci/IOPCIDevice.h>

static const char *NVIDIAArchStrings[] = {"GF100", "GK100", "GM100", "GP100", "GV100"};

// Methodology:
// Step 1: Spoof everything but NVHal to GK100
// Step 2: Patch/Wrap in the differences
// Step 3: Force the correct HAL kernel extension
// Step 4: Profit???

class NWD {
	friend class Pascal;
    public:
    static NWD *callback;
	
	enum struct NVGen {
		GM100,
		GP100,
		GV100, // Needs GV100HALWeb binary from THAT Web Drivers package.
		Unknown,
	};
	
	const char *NVGen2Arch(NVGen gen) {
		switch (gen) {
			case NVGen::GM100:
				return "GM100";
			case NVGen::GP100:
				return "GP100";
			case NVGen::GV100:
				return "GV100";
		}
		return "Unsupported";
	}
	
	UInt32 deviceId {0};
	IOPCIDevice *gpu {nullptr};
	NVGen gfxGen = NVGen::Unknown;

    void init();
	void setArchitecture();

    private:
    void processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size);
	void processPatcher(KernelPatcher &patcher);
	
	static size_t wrapFunctionReturnZero();
	
	static size_t wrapReturnZeroButChangeNVTypeAndArch(IOService *provider) {
		provider->setProperty("NVDAType", OSString::withCString("Web"));
		provider->setProperty("NVDAArch", OSString::withCString(callback->NVGen2Arch(callback->gfxGen)));
		return 0;
	}
	
	static size_t wrapReturnZeroButRevert(IOService *provider) {
		provider->setProperty("NVDAType", OSString::withCString("Official"));
		provider->setProperty("NVArch", OSString::withCString("GK100"));
		return 0;
	}
};

static constexpr UInt8 kNVDAStartupForceGK100Original[] = {0x41, 0x8D, 0x44, 0x24, 0xF2, 0x83, 0xF8, 0x03, 0x73, 0x10};
static constexpr UInt8 kNVDAStartupForceGK100Patched[] = {0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90};
