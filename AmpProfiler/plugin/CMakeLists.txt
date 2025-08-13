cmake_minimum_required(VERSION 3.22)
project(AmpProfiler VERSION 0.2.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.0)
FetchContent_MakeAvailable(juce)

find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_cxx_api.h HINTS ${ONNXRUNTIME_ROOT}/include)
find_library(ONNXRUNTIME_LIB NAMES onnxruntime HINTS ${ONNXRUNTIME_ROOT}/lib)

if(NOT ONNXRUNTIME_LIB)
  message(FATAL_ERROR "Set ONNXRUNTIME_ROOT to your onnxruntime folder")
endif()

juce_add_plugin(AmpProfiler COMPANY_NAME "YourName" BUNDLE_ID com.yourname.ampprofiler IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE MICROPHONE_PERMISSION_ENABLED TRUE FORMATS VST3 AU PRODUCT_NAME "AmpProfiler")

file(GLOB SRC_FILES Source/*.cpp Source/*/*.cpp Source/*.h Source/*/*.h)
target_sources(AmpProfiler PRIVATE ${SRC_FILES})
target_compile_definitions(AmpProfiler PRIVATE JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_WEB_BROWSER=0 JUCE_USE_DARK_SPLASH_SCREEN=1)
target_include_directories(AmpProfiler PRIVATE ${ONNXRUNTIME_INCLUDE_DIR})
target_link_libraries(AmpProfiler PRIVATE ${ONNXRUNTIME_LIB} juce::juce_audio_utils juce::juce_dsp)

add_custom_command(TARGET AmpProfiler POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Source/Models $<TARGET_FILE_DIR:AmpProfiler>/Models)
add_custom_command(TARGET AmpProfiler POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/resources/default_cab_ir.wav $<TARGET_FILE_DIR:AmpProfiler>/default_cab_ir.wav)
