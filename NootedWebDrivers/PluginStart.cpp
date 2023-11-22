//! Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.5.
//! See LICENSE for details.

#include "NWD.hpp"
#include <Headers/kern_api.hpp>
#include <Headers/plugin_start.hpp>

static NWD nwd;

static const char *bootargDebug[] = {
    "-NWDDebug",
};

PluginConfiguration ADDPR(config) {
    xStringify(PRODUCT_NAME),
    parseModuleVersion(xStringify(MODULE_VERSION)),
    LiluAPI::AllowNormal | LiluAPI::AllowSafeMode,
    nullptr,
    0,
    bootargDebug,
    arrsize(bootargDebug),
    nullptr,
    0,
    KernelVersion::HighSierra,
    KernelVersion::BigSur,
    []() { nwd.init(); },
};
