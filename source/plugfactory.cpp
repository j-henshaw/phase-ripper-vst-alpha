#include "public.sdk/source/main/pluginfactory.h"

#include "../include/plugcontroller.hpp"
#include "../include/plugprocessor.hpp"
#include "../include/plugids.hpp"
#include "../include/version.hpp"

BEGIN_FACTORY_DEF(stringCompanyName, stringCompanyWeb, stringCompanyEmail)
    DEF_CLASS2(INLINE_UID_FROM_FUID(ProcessorUID),
            PClassInfo::kManyInstances,
            kVstAudioEffectClass,
            stringPluginName,
            Vst::kDistributable,
            Vst::PlugType::kFxInstrument,
            FULL_VERSION_STR,
            kVstVersionString,
            PlugProcessor::createInstance)

    DEF_CLASS2(INLINE_UID_FROM_FUID(ControllerUID),
            PClassInfo::kManyInstances,
            kVstComponentControllerClass,
            stringPluginName "Controller",
            0,
            "",
            FULL_VERSION_STR,
            kVstVersionString,
            PlugController::createInstance)

END_FACTORY


bool InitModule() {
    return true;
}

bool DeinitModule() {
    return true;
}
