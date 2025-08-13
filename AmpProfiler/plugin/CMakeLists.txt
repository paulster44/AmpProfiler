cmake_minimum_required(VERSION 3.22)

project(AmpProfiler VERSION 0.2.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# -----------------------
# JUCE via FetchContent
# -----------------------
include(FetchContent)
FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.0
)
FetchContent_MakeAvailable(juce)

# -----------------------
# ONNX Runtime discovery
# -----------------------
if(NOT DEFINED ONNXRUNTIME_ROOT AND DEFINED ENV{ONNXRUNTIME_ROOT})
  set(ONNXRUNTIME_ROOT $ENV{ONNXRUNTIME_ROOT})
endif()

find_path(ONNXRUNTIME_INCLUDE_DIR
  onnxruntime_cxx_api.h
  HINTS "${ONNXRUNTIME_ROOT}/include"
)

find_library(ONNXRUNTIME_LIB
  NAMES onnxruntime
  HINTS "${ONNXRUNTIME_ROOT}/lib"
)

if(NOT ONNXRUNTIME_INCLUDE_DIR OR NOT ONNXRUNTIME_LIB)
  message(FATAL_ERROR "ONNX Runtime not found. Set ONNXRUNTIME_ROOT to the folder with 'include' and 'lib'.")
endif()

# -----------------------
# JUCE plugin target
# -----------------------
set(AMPPLUGIN_FORMATS VST3)
# If you're NOT on CI, you can enable AU locally by adding AU to the list above.

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

# Generate JuceHeader.h
juce_generate_juce_header(AmpProfiler)

# -----------------------
# Sources
# -----------------------
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
)
target_sources(AmpProfiler PRIVATE ${SRC_FILES})

# -----------------------
# Compile options / includes / links
# -----------------------
target_compile_definitions(AmpProfiler PRIVATE
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_DARK_SPLASH_SCREEN=1
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

# -----------------------
# Post-build: copy resources (skip Models in CI)
# -----------------------
# Always ensure the destination exists so other copies succeed
add_custom_command(TARGET AmpProfiler POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:AmpProfiler>"
)

# Only copy the Models directory when not running in GitHub Actions CI
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

# Copy default IR if present (this one is safe to do in CI)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/default_cab_ir.wav")
  add_custom_command(TARGET AmpProfiler POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${CMAKE_CURRENT_SOURCE_DIR}/resources/default_cab_ir.wav"
              "$<TARGET_FILE_DIR:AmpProfiler>/default_cab_ir.wav"
      COMMENT "Copying default_cab_ir.wav"
  )
endif()
