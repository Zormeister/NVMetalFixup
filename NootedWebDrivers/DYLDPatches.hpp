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
	static boolean_t wrapCsValidateRange(vnode_t vp, memory_object_t pager, memory_object_offset_t offset, const void *data, vm_size_t size, unsigned *result);
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

static const UInt8 kNVMTLClassPatch1Original[] = {0x3D, 0x97, 0xA1, 0x00, 0x00, 0x75, 0x00};
static const UInt8 kNVMTLClassPatch1FindMask[] = {0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0x00};
static const UInt8 kNVMTLClassPatch1Patched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x74, 0x00};
static const UInt8 kNVMTLClassPatch1ReplMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00};

static const UInt8 kNVMTLClassPatch2Original[] = {0x3D, 0x97, 0xA1, 0x00, 0x00, 0x74, 0x00};
static const UInt8 kNVMTLClassPatch2FindMask[] = {0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0x00};
static const UInt8 kNVMTLClassPatch2Patched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x00};
static const UInt8 kNVMTLClassPatch2ReplMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00};

static const UInt8 kNVMTLClassPatch3Original[] = {0x81, 0xF9, 0x97, 0xA1, 0x00, 0x00, 0x0F, 0x85, 0x00, 0x00, 0x00, 0x00};
static const UInt8 kNVMTLClassPatch3FindMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
static const UInt8 kNVMTLClassPatch3Replaced[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00};
static const UInt8 kNVMTLClassPatch3ReplMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00};
