//! Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//! See LICENSE for details.

#include "Pascal.hpp"
#include "PatcherPlus.hpp"

Pascal *Pascal::callback = nullptr;

void Pascal::init() { callback = this; }

//! Fermi and Kepler share a lot of logic, Jesus.
void Pascal::processKext(KernelPatcher &patcher, size_t id, mach_vm_address_t slide, size_t size) {
    SolveRequestPlus solveRequest {"__ZN12nvPushBuffer9MakeSpaceEj", this->orgMakeSpace};
    PANIC_COND(!solveRequest.solve(patcher, id, slide, size), "Pascal", "Failed to solve MakeSpace");

    RouteRequestPlus requests[] = {
        {"__ZN10nvFermiHAL13InvalidateMMUEP15nvGpFifoChannel", wrapInvalidateMMU},
        {"__ZN19nvFermiSharedPixels16InitMemToMemCapsEv", wrapInitMemToMemCaps},
        {"__ZN10nvFermiHAL16FlushGlobalCacheEP15nvGpFifoChannel11nvFlushMode", wrapFlushGlobalCache},
        {"__ZN10nvFermiHAL8InitCapsEv", wrapInitCaps, this->orgInitCaps},
        {"__ZN5nvHAL22GetFirstClassSupportedEPKjj", wrapGetFirstClassSupported, this->orgGetFirstClassSupported},
    };
    PANIC_COND(!RouteRequestPlus::routeAll(patcher, id, requests, slide, size), "Pascal", "Failed to route symbols");
}

//! The only difference between `nvKeplerSharedPixels` and `nvMaxwellSharedPixels`
void Pascal::wrapInitMemToMemCaps(void *that) {
    getMember<UInt64>(that, 0xC) = 0x3FFFFF00010000;
    getMember<UInt64>(that, 0x14) = 0x10000000010000;
    getMember<UInt64>(that, 0x1C) = 0x1000000010000;
}

void Pascal::wrapInvalidateMMU(void *that, void *fifoChannel) {
    void *pushBuffer = getMember<void *>(fifoChannel, 0x150);
    callback->orgMakeSpace(pushBuffer, 5);
    auto *&data = getMember<UInt32 *>(that, 0x10);
    data[0] = 0x2004000A;
    data[1] = 0x00000000;
    data[2] = 0x00000000;
    data[3] = 0x00000001;
    data[4] = 0x48000000;
    data += 5;
    getMember<UInt32>(fifoChannel, 0x4A0) = getMember<UInt32>(getMember<void *>(fifoChannel, 0x80), 0x384);
}

void Pascal::wrapFlushGlobalCache(void *that, void *fifoChannel, UInt32 flushMode) {
    if (flushMode != 1 && flushMode != 4) {
        void *pushBuffer = getMember<void *>(fifoChannel, 0x150);
        callback->orgMakeSpace(pushBuffer, 5);
        auto *&data = getMember<UInt32 *>(that, 0x10);
        data[0] = 0x2004000A;
        data[1] = 0x00000000;
        data[2] = 0x00000000;
        data[3] = 0x00000000;
        data[4] = 0x70000000;
        data += 5;
        getMember<UInt16>(that, 0x74) = 0x100;
    }
}

void Pascal::wrapInitCaps(void *that) {
    auto *info = getMember<void *>(that, 0x40);
    const auto class_ = getMember<UInt32>(info, 0xFE0);
    switch (class_) {
        case 0xC097:    //! PASCAL_A
            DBGLOG("Pascal", "Class is Pascal A");
            getMember<UInt32>(info, 0x808) = 0x5204;
            break;
        case 0xC197:    //! PASCAL_B
            DBGLOG("Pascal", "Class is Pascal B");
            getMember<UInt32>(info, 0x808) = 0x5404;
            break;
        default:
            DBGLOG("Pascal", "Unknown class 0x%X", class_);
            getMember<UInt32>(info, 0x808) = 0x2005004;
            break;
    }
    FunctionCast(wrapInitCaps, callback->orgInitCaps)(that);
}

UInt32 Pascal::wrapGetFirstClassSupported(void *that, const UInt32 *classes, UInt32 numClasses) {
    auto ret = FunctionCast(wrapGetFirstClassSupported, callback->orgGetFirstClassSupported)(that, classes, numClasses);
    if (!ret && numClasses) {
        ret = classes[0];
        DBGLOG("Pascal", "GetFirstClassSupported returned 0, returning 0x%X", ret);
    }
    return ret;
}
