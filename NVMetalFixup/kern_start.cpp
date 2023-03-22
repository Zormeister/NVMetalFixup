//  Copyright © 2022-2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

#include "kern_nvmf.hpp"
#include <Headers/kern_api.hpp>
#include <Headers/plugin_start.hpp>

static NVMF nvmf;

static const char *bootargOff[] = {
    "-nvmfoff",
};

static const char *bootargDebug[] = {
    "-nvmfdbg",
};

static const char *bootargBeta[] = {
    "-nvmfbeta",
};

PluginConfiguration ADDPR(config) {
    xStringify(PRODUCT_NAME),
    parseModuleVersion(xStringify(MODULE_VERSION)),
    LiluAPI::AllowNormal | LiluAPI::AllowSafeMode,
    bootargOff,
    arrsize(bootargOff),
    bootargDebug,
    arrsize(bootargDebug),
    bootargBeta,
    arrsize(bootargBeta),
    KernelVersion::Mojave,
    KernelVersion::Ventura,
    []() { nvmf.init(); },
};
