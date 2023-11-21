//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//  See LICENSE for details.

#pragma once
#include <libkern/c++/OSObject.h>
#include <libkern/OSTypes.h>

// Part of when we take over nvKeplerHAL::InvalidateMMU
// Or Thermi? I need to double-check.
// Pascal really do be Kepler++.
class nvPushBuffer : public OSObject {
	OSDeclareAbstractStructors(nvPushBuffer);
	
	// Ghidra froze, I'll fill it in eventually.
	virtual void free() APPLE_KEXT_OVERRIDE;
	virtual bool init(void *interface, void *accel, UInt32 space); // ?
	// CreateContext...
	// AssertMapped...
	virtual bool AllocateMemory(UInt32 space) = 0; // Pure-virtual function
	// FreeMemory...
	
	// MakeSpace...
};

// Can't wait to have to do MTLDriver.

