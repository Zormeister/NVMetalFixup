//! Copyright © 2022-2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//! See LICENSE for details.

#pragma once
#include <Headers/kern_patcher.hpp>
#include <Headers/kern_util.hpp>

class DYLDPatch {
    const void *find {nullptr}, *findMask {nullptr};
    const void *replace {nullptr}, *replaceMask {nullptr};
    const size_t size {0};
    const char *comment {nullptr};

    public:
    DYLDPatch(const void *find, const void *replace, size_t size, const char *comment)
        : find {find}, replace {replace}, size {size}, comment {comment} {}

    DYLDPatch(const void *find, const void *findMask, const void *replace, const void *replaceMask, size_t size,
        const char *comment)
        : find {find}, findMask {findMask}, replace {replace}, replaceMask {replaceMask}, size {size},
          comment {comment} {}

    DYLDPatch(const void *find, const void *findMask, const void *replace, size_t size, const char *comment)
        : find {find}, findMask {findMask}, replace {replace}, size {size}, comment {comment} {}

    template<typename T, size_t N>
    DYLDPatch(const T (&find)[N], const T (&replace)[N], const char *comment)
        : DYLDPatch(find, replace, N * sizeof(T), comment) {}

    template<typename T, size_t N>
    DYLDPatch(const T (&find)[N], const T (&findMask)[N], const T (&replace)[N], const T (&replaceMask)[N],
        const char *comment)
        : DYLDPatch(find, findMask, replace, replaceMask, N * sizeof(T), comment) {}

    template<typename T, size_t N>
    DYLDPatch(const T (&find)[N], const T (&findMask)[N], const T (&replace)[N], const char *comment)
        : DYLDPatch(find, findMask, replace, N * sizeof(T), comment) {}

    inline void apply(void *data, size_t size) const {
        if (UNLIKELY(KernelPatcher::findAndReplaceWithMask(data, size, this->find, this->size, this->findMask,
                this->findMask ? this->size : 0, this->replace, this->size, this->replaceMask,
                this->replaceMask ? this->size : 0))) {
            DBGLOG("DYLD", "Applied '%s' patch", this->comment);
        }
    }

    static inline void applyAll(const DYLDPatch *patches, size_t count, void *data, size_t size) {
        for (size_t i = 0; i < count; i++) { patches[i].apply(data, size); }
    }

    template<size_t N>
    static inline void applyAll(const DYLDPatch (&patches)[N], void *data, size_t size) {
        applyAll(patches, N, data, size);
    }
};

class DYLDPatches {
    public:
    static DYLDPatches *callback;

    void init();
    void processPatcher(KernelPatcher &patcher);

    private:
    static void apply(char *path, void *data, size_t size);

    mach_vm_address_t orgCsValidatePage {0};
    static void wrapCsValidatePage(vnode *vp, memory_object_t pager, memory_object_offset_t page_offset,
        const void *data, int *validated_p, int *tainted_p, int *nx_p);
};

//! VideoToolbox DRM model check
static const char kVideoToolboxDRMModelOriginal[] = "MacPro5,1\0MacPro6,1\0IOService";

static const char kHwGvaId[] = "Mac-7BA5B2D9E42DDD94";

//! AppleGVA model check
static const char kAGVABoardIdOriginal[] = "board-id\0hw.model";
static const char kAGVABoardIdPatched[] = "hwgva-id\0hw.model";

static const char kCoreLSKDMSEPath[] = "/System/Library/PrivateFrameworks/CoreLSKDMSE.framework/Versions/A/CoreLSKDMSE";
static const char kCoreLSKDPath[] = "/System/Library/PrivateFrameworks/CoreLSKD.framework/Versions/A/CoreLSKD";

static const UInt8 kCoreLSKDOriginal[] = {0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x0F, 0xA2};
static const UInt8 kCoreLSKDPatched[] = {0xC7, 0xC0, 0xC3, 0x06, 0x03, 0x00, 0x66, 0x90};

//! AppleGVAHEVCEncoder model check
static const char kHEVCEncBoardIdOriginal[] = "vendor8bit\0IOService\0board-id";
static const char kHEVCEncBoardIdPatched[] = "vendor8bit\0IOService\0hwgva-id";

//! Theoretically should match with 18G103
//! this can DEFINITELY be optimised w/ Masks n' stuff.

/**
 * `NVMTLDevice::initWithAcceleratorPort`
 * GeForceMTLDriver.bundle
 */

//! Got lucky with how the code blocks are structured, should just move on from this chunk to the next, I think.
static const UInt8 kNVMTLDeviceNVClassBypassOriginalMojave[] = {0x3D, 0x97, 0xA0, 0x00, 0x00, 0x74, 0x0E, 0x3D, 0x97, 0xA2, 0x00, 0x00, 0x74, 0x07, 0x3D, 0x97, 0xA1, 0x00, 0x00, 0x75, 0x56};
static const UInt8 kNVMTLDeviceNVClassBypassPatchedMojave[] = {0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x90};

//! force the if condition via swapping a JNZ to a JZ, please do not use a Kepler card with this kext fyi
static const UInt8 kNVMTLDeviceNVClassBypassOriginalMojave2[] = {0x81, 0xF9, 0x97, 0xA1, 0x00, 0x00, 0x0F, 0x85, 0x5E, 0xFE, 0xFF, 0xFF};
static const UInt8 kNVMTLDeviceNVClassBypassPatchedMojave2[] = {0x81, 0xF9, 0x97, 0xA1, 0x00, 0x00, 0x0F, 0x84, 0x5E, 0xFE, 0xFF, 0xFF};

//! same here
static const UInt8 kNVMTLTextureNVClassBypassOriginalMojave[] = {0x3D, 0x97, 0xA1, 0x00, 0x00, 0x75, 0x4F};
static const UInt8 kNVMTLTextureNVClassBypassPatchedMojave[] = {0x3D, 0x97, 0xA1, 0x00, 0x00, 0x74, 0x4F};

//! same here
static const UInt8 kNVMTLTextureNVClassBypassOriginalMojave2[] = {0x81, 0xf9, 0x97, 0xa1, 0x00, 0x00, 0x75, 0x72};
static const UInt8 kNVMTLTextureNVClassBypassPatchedMojave2[] = {0x81, 0xf9, 0x97, 0xa1, 0x00, 0x00, 0x74, 0x72};

static const UInt8 kNVMTLTextureNVClassBypassOriginalMojave3[] = {0x3D, 0x97, 0xA1, 0x00, 0x00, 0x75, 0x6F};
static const UInt8 kNVMTLTextureNVClassBypassPatchedMojave3[] = {0x3D, 0x97, 0xA1, 0x00, 0x00, 0x74, 0x6F};

static const UInt8 kNVMTLTextureNVClassBypassOriginalMojave4[] = {0x3D, 0x97, 0xA1, 0x00, 0x00, 0x0F, 0x85, 0xC9, 0x00, 0x00, 0x00};
static const UInt8 kNVMTLTextureNVClassBypassPatchedMojave4[] = {0x3D, 0x97, 0xA1, 0x00, 0x00, 0x0F, 0x84, 0xC9, 0x00, 0x00, 0x00};

static const UInt8 kNVMTLTextureNVClassBypassOriginalMojave5[] = {0x81, 0xF9, 0x97, 0xA1, 0x00, 0x00, 0x0F, 0x85, 0x81, 0x00, 0x00, 0x00};
static const UInt8 kNVMTLTextureNVClassBypassPatchedMojave5[] = {0x81, 0xF9, 0x97, 0xA1, 0x00, 0x00, 0x0F, 0x84, 0x81, 0x00, 0x00, 0x00};

static const UInt8 kNVMTLTextureNVClassBypassOriginalMojave6[] = {0x81, 0xF9, 0x97, 0xA1, 0x00, 0x00, 0x75, 0x78};
static const UInt8 kNVMTLTextureNVClassBypassPatchedMojave6[] = {0x81, 0xF9, 0x97, 0xA1, 0x00, 0x00, 0x74, 0x78};

static const UInt8 kUnknownClassNVClassBypassOriginalMojave[] = {0x3D, 0x97, 0xA1, 0x00, 0x00, 0x74, 0x10};
static const UInt8 kUnknownClassNVClassBypassPatchedMojave[] = {0x66, 0x90, 0x66, 0x90, 0x90, 0xEB, 0x10};

/**
 * `NVMTLCommandBuffer`
 * another bypass
 */

static const UInt8 kNVMTLCmdBufNVClassBypassOriginalMojave[] = {0x3D, 0x97, 0xA1, 0x00, 0x00, 0x74, 0x29};
static const UInt8 kNVMTLCmdBufNVClassBypassPatchedMojave[] = {0x66, 0x90, 0x66, 0x90, 0x90, 0xEB, 0x29};

static const UInt8 kNVMTLCmdBufNVClassBypassOriginalMojave2[] = {0x3D, 0x97, 0xA1, 0x00, 0x00, 0x74, 0x26};
static const UInt8 kNVMTLCmdBufNVClassBypassPatchedMojave2[] = {0x66, 0x90, 0x66, 0x90, 0x90, 0xEB, 0x26};
