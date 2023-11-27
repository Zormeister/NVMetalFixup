//! Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//! See LICENSE for details.

#pragma once
#include <Headers/kern_iokit.hpp>
#include <Headers/kern_patcher.hpp>
#include <Headers/kern_util.hpp>
#include <IOKit/IOService.h>
#include <IOKit/ndrvsupport/IONDRVFramebuffer.h>
#include <IOKit/pci/IOPCIDevice.h>

enum IOGraphicsAcceleratorClient : UInt32 {
    kIOGraphicsAcceleratorClientSurface = 0,
    kIOGraphicsAcceleratorClient2DContext = 2,
    kIOGraphicsAcceleratorClientDisplayPipe = 4,
    kIOGraphicsAcceleratorClientDevice,
    kIOGraphicsAcceleratorClientShared,
    kIOGraphicsAcceleratorClientMemoryInfo,
    kIOGraphicsAcceleratorClientCommandQueue = 9,
};

using t_makeSpace = IOReturn (*)(void *that, UInt32 space);

//! Methodology:
//! Step 1: Spoof everything but NVHal to GK100
//! Step 2: Patch/Wrap in the differences
//! Step 3: Force the correct HAL kernel extension
//! Step 4: Profit???

enum struct NVGen {
    GM100 = 0,
    GP100,
    //! Needs GV100HALWeb binary from THAT Web Drivers package.
    GV100,
    Unknown,
};

class NWD {
    friend class Pascal;
    friend class Maxwell;

    public:
    void init();

    private:
    static NWD *callback;

    UInt32 deviceId {0};
    IOPCIDevice *gpu {nullptr};
    NVGen gfxGen {NVGen::Unknown};

    void setArchitecture();

    const char *getArchString() {
        static const char *arches[] = {"GM100", "GP100", "GV100", "Unsupported"};
        return arches[static_cast<int>(this->gfxGen)];
    }

    void processKext(KernelPatcher &patcher, size_t id, mach_vm_address_t slide, size_t size);
    void processPatcher(KernelPatcher &patcher);

    static IOService *wrapProbeFailButChangeNVTypeAndArch(IOService *that, IOService *provider);

    //! NVIDIA's drivers have a logic bug where the contextStop method isn't called on failure.
    mach_vm_address_t orgNewUserClient {0};
    static IOReturn wrapNewUserClient(void *that, task_t owningTask, void *securityID, UInt32 type,
        OSDictionary *handler, IOUserClient **properties);
};

static const UInt8 kNVDAStartupForceGK100Original[] = {0x41, 0x8D, 0x44, 0x24, 0xF2, 0x83, 0xF8, 0x03, 0x73, 0x10};
static const UInt8 kNVDAStartupForceGK100Patched[] = {0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90};
