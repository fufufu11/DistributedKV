$BuildDir = "build"

if (Test-Path $BuildDir) {
    Write-Host "Cleaning old build directory..."
    Remove-Item -Recurse -Force $BuildDir
}

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
Set-Location -Path $BuildDir

Write-Host "Configuring Project with GCC..."
# If cmake is not in PATH, user needs to add it or use the absolute path
cmake -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++ ..

Write-Host "Building project..."
cmake --build .

if (Test-Path "DistributedKV_bin.exe") {
    Write-Host "Running tests..."
    ./DistributedKV_bin.exe
}

Set-Location -Path ..
