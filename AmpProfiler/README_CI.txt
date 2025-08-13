GitHub Actions – Auto-build VST3/AU

What this does:
- When you push this folder to a GitHub repo (main/master branch), it automatically builds on Windows and macOS.
- The workflow downloads ONNX Runtime, configures CMake with JUCE, builds, and uploads artifacts (.vst3 and .component) to the Actions run.

How to use:
1) Create a new GitHub repo (public or private).
2) Put the entire AmpProfiler folder in the root of the repo and commit/push.
3) Open the repo → "Actions" tab → select the latest run → "Artifacts" section → download the built plugins.

Notes:
- The workflow uses ONNX Runtime version 1.18.0 (change ORT_VERSION in .github/workflows/build.yml if needed).
- Artifacts are unsigned. You can code-sign them later if you want.
