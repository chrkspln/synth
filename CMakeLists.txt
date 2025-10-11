cmake_minimum_required(VERSION 3.28)
project(SoundStuff VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG master)
FetchContent_MakeAvailable(JUCE)

juce_add_gui_app(SoundStuff
    PRODUCT_NAME "SoundStuff"
    VERSION 69)

target_sources(SoundStuff PRIVATE
    main.cpp)

target_link_libraries(SoundStuff PRIVATE
    juce::juce_core
    juce::juce_gui_basics
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_utils
)

target_include_directories(SoundStuff PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(SoundStuff PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0)