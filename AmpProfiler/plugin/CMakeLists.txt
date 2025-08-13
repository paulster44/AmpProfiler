cmake_minimum_required(VERSION 3.22)

project(AmpProfiler VERSION 0.2.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# JUCE via FetchContent
include(FetchContent)
FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.0
)
FetchContent_MakeAvailable(juce)

# ONNX Runtime discovery
if(NOT DEFINED ONNXRUNTIME_ROOT AND DEFINED ENV{ONNXRUNTIME_ROOT})
  set(ONNXRUNTIME_ROOT $ENV{ONNXRUNTIME_ROOT})
endif()

find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_cxx_api.h HINTS "${ONNXRUNTIME_ROOT}/include")
find_library(ONNXRUNTIME_LIB NAMES onnxruntime HINTS "${ONNXRUNTIME_ROOT}/lib")

if(NOT ONNXRUNTIME_INCLUDE_DIR OR NOT ONNXRUNTIME_LIB)
  message(FATAL_ERROR "ONNX Runtime not found. Set ONNXRUNTIME_ROOT to the folder with 'include' and 'lib'.")
endif()

# Plugin target (VST3 only for CI)
set(AMPPLUGIN_FORMATS VST3)

juce_add_plugin(AmpProfiler
    COMPANY_NAME "YourName"
    BUNDLE_ID com.yourname.ampprofiler
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    MICROPHONE_PERMISSION_ENABLED TRUE
    FORMATS ${AMPPLUGIN_FORMATS}
    PRODUCT_NAME "AmpProfiler"
)

juce_generate_juce_header(AmpProfiler)

# Sources
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
)
target_sources(AmpProfiler PRIVATE ${SRC_FILES})

# Compile options / includes / links
target_compile_definitions(AmpProfiler PRIVATE
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_DARK_SPLASH_SCREEN=1
    JUCE_VST3_CAN_REPLACE_VST2=0   # stop JUCE from pulling old VST2 headers
)

target_include_directories(AmpProfiler PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/JuceLibraryCode"
    "${ONNXRUNTIME_INCLUDE_DIR}"
)

target_link_libraries(AmpProfiler PRIVATE
    "${ONNXRUNTIME_LIB}"
    juce::juce_audio_utils
    juce::juce_dsp
)

# Linux: link libcurl (fixes undefined curl_* on Ubuntu)
if(UNIX AND NOT APPLE)
  find_package(CURL)
  if (CURL_FOUND)
    target_link_libraries(AmpProfiler PRIVATE CURL::libcurl)
  else()
    target_link_libraries(AmpProfiler PRIVATE curl)
  endif()
endif()

# Post-build copies (skip Models in CI to avoid flakiness)
add_custom_command(TARGET AmpProfiler POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:AmpProfiler>"
)

if(NOT DEFINED ENV{GITHUB_ACTIONS})
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/Models")
    add_custom_command(TARGET AmpProfiler POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:AmpProfiler>/Models"
      COMMAND ${CMAKE_COMMAND} -E copy_directory
              "${CMAKE_CURRENT_SOURCE_DIR}/Source/Models"
              "$<TARGET_FILE_DIR:AmpProfiler>/Models"
      COMMENT "Copying Models directory"
    )
  endif()
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/default_cab_ir.wav")
  add_custom_command(TARGET AmpProfiler POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${CMAKE_CURRENT_SOURCE_DIR}/resources/default_cab_ir.wav"
              "$<TARGET_FILE_DIR:AmpProfiler>/default_cab_ir.wav"
      COMMENT "Copying default_cab_ir.wav"
  )
endif()
