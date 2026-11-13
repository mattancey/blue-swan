cmake_minimum_required(VERSION 3.22)
project(BlueSwan VERSION 0.7.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version" FORCE)
    set(CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Xcode deployment target" FORCE)
endif()

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6 
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(BlueSwan
    COMPANY_NAME "Personal"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Pers
    PLUGIN_CODE BlSw
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Blue Swan"
)

target_sources(BlueSwan PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Dsp/Resonator.cpp
    Source/Tests/ResonatorTest.cpp
)

target_compile_definitions(BlueSwan PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_UNIT_TESTS=1
)

target_link_libraries(BlueSwan PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
