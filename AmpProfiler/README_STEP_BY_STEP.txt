AmpProfiler – Super Short Guide
1) Go to trainer/: create a venv, install requirements, run train.py, then export.py to get modelA.onnx + profile.json.
2) Put modelA.onnx and profile.json into plugin/Source/Models/ (cabA.wav optional).
3) Set ONNXRUNTIME_ROOT to your unzipped ONNX Runtime folder (contains include/ and lib/).
4) Run the one-click script for your OS in plugin/: build_win.ps1 (Windows) or build_mac.sh (macOS).
5) Load AmpProfiler.vst3 in your DAW and riff.
