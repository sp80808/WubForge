cmake_minimum_required(VERSION 3.15)
project(WubForge VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE Configuration
add_subdirectory(JUCE)
add_subdirectory(chowdsp_utils)
add_subdirectory(JUMP)
# Add MDA plugins as shared modules (using a custom approach since they're individual projects)
# We'll include their source directly
enable_testing()

# Plugin Target
juce_add_plugin(WubForge
    # VST3 and AU formats
    PLUGIN_FORMATS VST3 AU
    PLUGIN_NAME "WubForge"
    PLUGIN_CODE WbFg
    PLUGIN_MANUFACTURER_CODE WbFg
    BUNDLE_ID "com.wubforge.wubforge"
    PLUGIN_MANUFACTURER "WubForge"
    PLUGIN_DESCRIPTION "Spectral Bass Processor"
    PLUGIN_IS_SYNTH FALSE
    PLUGIN_MIDI_INPUT TRUE
    PLUGIN_MIDI_OUTPUT FALSE
    PLUGIN_MIDI_THROUGH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES "Fx"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
    VST3_COPY_DIR ""
    AU_COPY_DIR ""
)

# Plugin Sources
target_sources(WubForge
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/CombStack.cpp
        Source/DistortionForge.cpp
        Source/BitCrusher.cpp
        Source/KeyTracker.cpp
        Source/FormantTracker.cpp
        Source/FormantTrackerModule.cpp
        Source/SpectralMorphingModule.cpp
        Source/FractalFilter.cpp
        Source/FractalFilterModule.cpp
        Source/SpectrogramComponent.cpp
        Source/ChowEQModule.cpp
        Source/MDASubSynthModuleDirect.cpp
        Source/Presets.cpp
)

# Plugin Headers
target_include_directories(WubForge
    PRIVATE
        Source/
)

# JUCE Modules
target_compile_definitions(WubForge
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_WINRT_MIDI=0
        JUCE_USE_DIRECTSOUND=0
        JUCE_USE_WINDOWS_MEDIA_FORMAT=0
        JUCE_USE_FLAC=0
        JUCE_USE_OGGVORBIS=0
        JUCE_USE_MP3AUDIOFORMAT=0
        JUCE_USE_LAME_AUDIO_FORMAT=0
        JUCE_USE_WINDOWS_MEDIA_FORMAT=0
        JUCE_USE_CDREADER=0
        JUCE_USE_CDBURNER=0
)

# Link JUCE modules
target_link_libraries(WubForge
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
        chowdsp::chowdsp_dsp_utils
        chowdsp::chowdsp_eq
        chowdsp::chowdsp_filters
        chowdsp::chowdsp_waveshapers
        foleys_gui_magic
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Platform-specific settings
if(APPLE)
    target_compile_definitions(WubForge PRIVATE JUCE_MAC=1)
    set_target_properties(WubForge PROPERTIES
        XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME "NO"
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
    )
elseif(WIN32)
    target_compile_definitions(WubForge PRIVATE JUCE_WINDOWS=1 _WINDOWS=1)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(WubForge PRIVATE JUCE_LINUX=1)
endif()

# Build output
set_target_properties(WubForge PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
