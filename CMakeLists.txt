cmake_minimum_required(VERSION 3.22)
project(SimpleMIDISynth VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG master
)
FetchContent_MakeAvailable(JUCE)

juce_add_gui_app(SimpleMIDISynth
    PRODUCT_NAME "Simple MIDI Synth"
)

target_sources(SimpleMIDISynth PRIVATE
    Main.cpp
    MainContentComponent.cpp
)

target_link_libraries(SimpleMIDISynth PRIVATE
    juce::juce_audio_utils
)

target_compile_definitions(SimpleMIDISynth PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)