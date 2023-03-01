cmake_minimum_required(VERSION 3.4.3)

project(PhaseRipper)

set(CMAKE_CXX_STANDARD 17)

set(plugin_sources
    include/plugcontroller.hpp
    include/ContinuousAutomationHandler.hpp
    include/plugids.hpp
    include/plugprocessor.hpp
    include/version.hpp
    source/HarmonicFlipper.cpp
    source/NoteBank.cpp
    source/plugfactory.cpp
    source/plugcontroller.cpp
    source/plugprocessor.cpp
    source/ContinuousAutomationHandler.cpp
)

set(target PhaseRipper)

#--- HERE define the location of the VST3 SDK source
set(VST_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../vst3sdk")

#--- HERE define the location of the VST3 SDK build
set(VST_SDK_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../build")

link_directories(${VST_SDK_BUILD_DIR}/lib/Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${VST_SDK_DIR}/cmake/modules")

include(Global)
include(AddVST3Library)
include(Bundle)
include(ExportedSymbols)
include(PrefixHeader)
include(PlatformIOS)
include(PlatformToolset)
include(CoreAudioSupport)
include(AAXSupport)
include(VstGuiSupport)
include(UniversalBinary)
include(AddVST3Options)

setupPlatformToolset()

set(SDK_ROOT "${VST_SDK_DIR}")
set(VSTGUI_ROOT "${VST_SDK_DIR}")
set(public_sdk_SOURCE_DIR ${SDK_ROOT}/public.sdk)
set(pluginterfaces_SOURCE_DIR ${SDK_ROOT}/pluginterfaces)

add_compile_definitions(DEVELOPMENT=1)

include_directories("ext/q/q_lib/include")
include_directories("ext/q/infra/include")

include_directories(${SDK_ROOT})
set(SMTG_RUN_VST_VALIDATOR OFF)

smtg_add_vst3plugin(${target} ${plugin_sources})
target_link_libraries(${target} PRIVATE expat base sdk vstgui_support base pluginterfaces vstgui_support sdk pluginterfaces vstgui_uidescription vstgui)
#target_include_directories(${target} PRIVATE ${SDK_ROOT/base})
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)

if(SMTG_MAC)
    smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
    target_link_libraries(${target} PRIVATE "-framework CoreFoundation" "-framework Cocoa" "-framework OpenGL" "-framework Accelerate" "-framework QuartzCore" "-framework Carbon")
elseif(SMTG_WIN)
    target_sources(${target} PRIVATE resource/plug.rc)
endif()

if(SMTG_LINUX)
    target_link_libraries(${target} PRIVATE xcb xcb-util xcb-cursor xcb-keysyms xcb xcb-util xcb-cursor xcb-keysyms xcb-xkb xkbcommon xkbcommon-x11 xkbcommon xkbcommon-x11 cairo fontconfig)
endif()

#set(test_sources
#    tests/test.cpp
#)

#add_executable(tests ${test_sources})
#set_target_properties(tests PROPERTIES CXX_STANDARD 17)
#target_link_libraries(tests PRIVATE base sdk pluginterfaces)
