//! Copyright © 2022-2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//! See LICENSE for details.

#include "DYLDPatches.hpp"
#include "NWD.hpp"
#include <Headers/kern_api.hpp>
#include <Headers/kern_devinfo.hpp>
#include <IOKit/IODeviceTreeSupport.h>

static const char *MTLDriverPath = "/System/Library/Extensions/GeForceMTLDriver.bundle/Contents/MacOS/GeForceMTLDriver";

DYLDPatches *DYLDPatches::callback = nullptr;

void DYLDPatches::init() { callback = this; }

void DYLDPatches::processPatcher(KernelPatcher &patcher) {
    KernelPatcher::RouteRequest request {"_cs_validate_range", wrapCsValidateRange, this->orgCsValidatePage};

    PANIC_COND(!patcher.routeMultipleLong(KernelPatcher::KernelID, &request, 1), "DYLD",
        "Failed to route kernel symbols");
}

boolean_t DYLDPatches::wrapCsValidateRange(vnode_t vp, memory_object_t pager, memory_object_offset_t page_offset, const void *data, vm_size_t size, unsigned int *result) {
	auto ret = FunctionCast(wrapCsValidateRange, callback->orgCsValidatePage)(vp, pager, page_offset, data, size, result);
	char path[PATH_MAX];
	int pathlen = PATH_MAX;
	if (ret && vn_getpath(vp, path, &pathlen) == 0) {
		if (UNLIKELY(!strncmp(path, MTLDriverPath, strlen(MTLDriverPath)))) {
			//! Wait wait, does this actually patch every instance or not? I'm confused.
			//! There's around a fair few instances of each patch.
			//! Ghidra query for GeForceMTLDriver: `97 a1 00 00`
			DYLDPatch patches[] {
				{kNVMTLClassPatch1Original, kNVMTLClassPatch1FindMask, kNVMTLClassPatch1Patched, kNVMTLClassPatch1ReplMask, "Force 0xA197 (1/3)"},
				{kNVMTLClassPatch2Original, kNVMTLClassPatch2FindMask, kNVMTLClassPatch2Original, kNVMTLClassPatch2ReplMask, "Force 0xA197 (2/3)"},
				{kNVMTLClassPatch3Original, kNVMTLClassPatch3FindMask, kNVMTLClassPatch3Original, kNVMTLClassPatch3ReplMask, "Force 0xA197 (3/3)"},
			};
			DYLDPatch::applyAll(patches, const_cast<void *>(data), size);
		}
	}
	return ret;
}

void DYLDPatches::wrapCsValidatePage(vnode *vp, memory_object_t pager, memory_object_offset_t page_offset,
    const void *data, int *validated_p, int *tainted_p, int *nx_p) {
    FunctionCast(wrapCsValidatePage, callback->orgCsValidatePage)(vp, pager, page_offset, data, validated_p, tainted_p,
        nx_p);

    char path[PATH_MAX];
    int pathlen = PATH_MAX;
    if (vn_getpath(vp, path, &pathlen) != 0) { return; }

    if (!UserPatcher::matchSharedCachePath(path)) {
        if (LIKELY(strncmp(path, kCoreLSKDMSEPath, arrsize(kCoreLSKDMSEPath))) ||
            LIKELY(strncmp(path, kCoreLSKDPath, arrsize(kCoreLSKDPath)))) {
            return;
        }
        const DYLDPatch patch = {kCoreLSKDOriginal, kCoreLSKDPatched, "CoreLSKD streaming CPUID to Haswell"};
        patch.apply(const_cast<void *>(data), PAGE_SIZE);
        return;
    }

    if (UNLIKELY(KernelPatcher::findAndReplace(const_cast<void *>(data), PAGE_SIZE, kVideoToolboxDRMModelOriginal,
            arrsize(kVideoToolboxDRMModelOriginal), BaseDeviceInfo::get().modelIdentifier, 20))) {
        DBGLOG("DYLD", "Applied 'VideoToolbox DRM model check' patch");
    }

    const DYLDPatch patches[] = {
        {kAGVABoardIdOriginal, kAGVABoardIdPatched, "iMacPro1,1 spoof (AppleGVA)"},
        {kHEVCEncBoardIdOriginal, kHEVCEncBoardIdPatched, "iMacPro1,1 spoof (AppleGVAHEVCEncoder)"},
    };
    DYLDPatch::applyAll(patches, const_cast<void *>(data), PAGE_SIZE);
}
