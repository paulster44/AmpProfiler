param([string]$OrtRoot = "$env:ONNXRUNTIME_ROOT")
if (-not $OrtRoot) { Write-Error "Set ONNXRUNTIME_ROOT or pass -OrtRoot"; exit 1 }
cd $PSScriptRoot
cmake -B build -G "Visual Studio 17 2022" -A x64 -DONNXRUNTIME_ROOT="$OrtRoot"
cmake --build build --config Release
Write-Host "Built. Find AmpProfiler.vst3 under plugin/build/Release/"
