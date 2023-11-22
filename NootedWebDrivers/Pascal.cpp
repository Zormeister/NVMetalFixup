//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//  See LICENSE for details.

#include "Pascal.hpp"
#include "PatcherPlus.hpp"
#include "GeForce.hpp"

void Pascal::init() {
	callback = this;
}

// Thermi and Kepler share a lot of logic, Jesus.
bool Pascal::processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t, size_t size) {
	RouteRequestPlus requests[] = {
		{"__ZN10nvFermiHAL13InvalidateMMUEP15nvGpFifoChannel", wrapInvalidateMMU},
		{"__ZN19nvFermiSharedPixels16InitMemToMemCapsEv", wrapInitMemToMemCaps},
	};
	return true;
}

// the only diff between nvKeplerSharedPixels and nvMaxwellSharedPixels
void Pascal::wrapInitMemToMemCaps(void *that) {
	getMember<UInt64>(that, 0xC) = 0x3FFFFF00010000;
	getMember<UInt64>(that, 0x14) = 0x10000000010000;
	getMember<UInt64>(that, 0x1C) = 0x1000000010000;
}

void Pascal::wrapInvalidateMMU(void *that, void *fifo) {
	// Obtain the PushBuffer
	nvPushBuffer *buffer = getMember<nvPushBuffer *>(fifo, 0x150);
	buffer->MakeSpace(0x5);
	
	// Input the values
	*buffer->buffer = 0x2004000A;
	*(buffer->buffer + 0x4) = 0;
	*(buffer->buffer + 0x8) = 0;
	*(buffer->buffer + 0xC) = 1;
	*(buffer->buffer + 0x10) = 0x48000000;
	
	// A value gets set here, research required.
}
