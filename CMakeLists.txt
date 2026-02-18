cmake_minimum_required(VERSION 3.22)
project(knightmiditest VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG master
)
FetchContent_MakeAvailable(JUCE)

juce_add_gui_app(knightmiditest
    PRODUCT_NAME "KnightMIDI Test"
)

target_sources(knightmidi-test PRIVATE
    Main.cpp
    MainContentComponent.cpp
)

target_link_libraries(knightmiditest PRIVATE
    juce::juce_audio_utils
)

target_compile_definitions(knightmiditest PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)