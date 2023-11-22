//! Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//! See LICENSE for details.

#include "Pascal.hpp"
#include "GeForce.hpp"
#include "PatcherPlus.hpp"

void Pascal::init() { callback = this; }

void Pascal::processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size) {
    //! Fermi and Kepler share a lot of logic, Jesus.
    RouteRequestPlus requests[] = {
        {"__ZN10nvFermiHAL13InvalidateMMUEP15nvGpFifoChannel", wrapInvalidateMMU},
        {"__ZN19nvFermiSharedPixels16InitMemToMemCapsEv", wrapInitMemToMemCaps},
        {"__ZN10nvFermiHAL16FlushGlobalCacheEP15nvGpFifoChannel11nvFlushMode", wrapFlushGlobalCache},
    };
    PANIC_COND(!RouteRequestPlus::routeAll(patcher, index, requests, address, size), "Pascal",
        "Failed to route symbols");
}

//! The only difference between `nvKeplerSharedPixels` and `nvMaxwellSharedPixels`
void Pascal::wrapInitMemToMemCaps(void *that) {
    getMember<UInt64>(that, 0xC) = 0x3FFFFF00010000;
    getMember<UInt64>(that, 0x14) = 0x10000000010000;
    getMember<UInt64>(that, 0x1C) = 0x1000000010000;
}

void Pascal::wrapInvalidateMMU(void *, void *fifoChannel) {
    nvPushBuffer *pushBuffer = getMember<nvPushBuffer *>(fifoChannel, 0x150);
    pushBuffer->MakeSpace(0x5);
    pushBuffer->data[0] = 0x2004000A;
    pushBuffer->data[1] = 0x00000000;
    pushBuffer->data[2] = 0x00000000;
    pushBuffer->data[3] = 0x00000001;
    pushBuffer->data[4] = 0x48000000;
    pushBuffer->data += 5;
    getMember<UInt32>(fifoChannel, 0x4A0) = getMember<UInt32>(getMember<void *>(fifoChannel, 0x80), 0x384);
}

void Pascal::wrapFlushGlobalCache(void *that, void *fifoChannel, UInt32 flushMode) {
    if (flushMode != 1 && flushMode != 4) {
        nvPushBuffer *pushBuffer = getMember<nvPushBuffer *>(fifoChannel, 0x150);
        pushBuffer->MakeSpace(0x5);
        pushBuffer->data[0] = 0x2004000A;
        pushBuffer->data[1] = 0x00000000;
        pushBuffer->data[2] = 0x00000000;
        pushBuffer->data[3] = 0x00000000;
        pushBuffer->data[4] = 0x70000000;
        pushBuffer->data += 5;
        getMember<UInt16>(that, 0x74) = 0x100;
    }
}
