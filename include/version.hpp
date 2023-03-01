#pragma once

#include "pluginterfaces/base/fplatform.h"

#define FULL_VERSION_STR "0.0.0.0"
#define VERSION_STR "0.0.0"

#define stringPluginName    "Phase Ripper"
#define stringOriginalFilename "PhaseRipper.vst3"

#if SMTG_PLATFORM_64
    #define stringFileDescription stringPluginName" VST3-SDK (64Bit)"
#else
    #define stringFileDescription stringPluginName" VST3-SDK"
#endif

#define stringCompanyName "No Company Name"
#define stringCompanyWeb "No Company Website"
#define stringCompanyEmail "No Company Email"


