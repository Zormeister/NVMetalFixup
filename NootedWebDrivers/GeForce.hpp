//! Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//! See LICENSE for details.

#pragma once
#include <IOKit/IOReturn.h>
#include <libkern/OSTypes.h>
#include <libkern/c++/OSObject.h>

class nvPushBuffer : public OSObject {
    OSDeclareAbstractStructors(nvPushBuffer);

    public:
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual bool init(class nvChannelInterface *interface, class nvAcceleratorParent *accelParent, UInt32 space);
    virtual IOReturn CreateContextDma(UInt32 space) = 0;
    virtual void AssertMapped();
    virtual IOReturn AllocateMemory(UInt32 space) = 0;
    virtual IOReturn FreeMemory();

    void Flush();
    IOReturn MakeSpace(UInt32 space);

    UInt32 *data;
};

//! Can't wait to have to do MTLDriver.
