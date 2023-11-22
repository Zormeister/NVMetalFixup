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
	
public:
	// Ghidra froze, I'll fill it in eventually.
	virtual void free() APPLE_KEXT_OVERRIDE;
	virtual bool init(class nvChannelInterface *interface, class nvAcceleratorParent *accel, UInt32 space); // ?
	virtual IOReturn CreateContextDma(UInt32 space) = 0; // ?
	virtual void AssertMapped();
	virtual IOReturn AllocateMemory(UInt32 space) = 0; // Pure-virtual function
	virtual IOReturn FreeMemory();
	
	void Flush();
	IOReturn MakeSpace(UInt32 space);
	
	void *idk;
	UInt32 *buffer; // field 0x8, offset from the pointer is 0x10, ask Ghidra I guess.
};

// Can't wait to have to do MTLDriver.

