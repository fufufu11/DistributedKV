$BuildDir = "build"
$CmakePath = "cmake"

# --- Find CMake ---
if (!(Get-Command $CmakePath -ErrorAction SilentlyContinue)) {
    $CommonPaths = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
        "C:\Program Files\CMake\bin\cmake.exe"
    )
    foreach ($Path in $CommonPaths) {
        if (Test-Path $Path) {
            $CmakePath = $Path
            break
        }
    }
}

if (!(Get-Command $CmakePath -ErrorAction SilentlyContinue)) {
    Write-Host "CMake not found. Please install CMake and add it to PATH." -ForegroundColor Red
    exit 1
}
Write-Host "Using CMake: $CmakePath" -ForegroundColor Cyan

if ($args -contains "--clean") {
    if (Test-Path $BuildDir) { Remove-Item -Recurse -Force $BuildDir }
}
if (!(Test-Path $BuildDir)) { New-Item -ItemType Directory -Path $BuildDir | Out-Null }
Push-Location $BuildDir

$CmakeArgs = @("..", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DCMAKE_TLS_VERIFY=OFF")

# --- Generator Selection ---
if (!(Test-Path "CMakeCache.txt")) {
    if (Get-Command ninja -ErrorAction SilentlyContinue) {
        Write-Host "Found Ninja, using Ninja generator." -ForegroundColor Green
        $CmakeArgs += "-G", "Ninja"
    }
    elseif (Get-Command mingw32-make -ErrorAction SilentlyContinue) {
        Write-Host "Found mingw32-make, using MinGW Makefiles." -ForegroundColor Green
        $CmakeArgs += "-G", "MinGW Makefiles"
    }
    elseif (Get-Command make -ErrorAction SilentlyContinue) {
        # Check if it's MSYS make or something else compatible
        Write-Host "Found make, attempting to use MinGW Makefiles." -ForegroundColor Green
        $CmakeArgs += "-G", "MinGW Makefiles"
        # Note: If make is strictly Unix make, this might fail, but usually on Windows 'make' with g++ implies MinGW/MSYS
    }
    else {
        Write-Host "Ninja/Make not found. Using default generator (likely Visual Studio)." -ForegroundColor Yellow
    }
}

& $CmakePath $CmakeArgs
if ($LASTEXITCODE -eq 0) {
    & $CmakePath --build . -j 8
    if ($LASTEXITCODE -eq 0) {
        if (Test-Path "compile_commands.json") {
            Copy-Item -Path "compile_commands.json" -Destination ".." -Force
        }
        $CtestPath = $CmakePath.Replace("cmake.exe", "ctest.exe")
        if (!(Test-Path $CtestPath)) { 
            if (Get-Command ctest -ErrorAction SilentlyContinue) { $CtestPath = "ctest" }
        }
        if (Get-Command $CtestPath -ErrorAction SilentlyContinue) {
            & $CtestPath --output-on-failure
        }
    }
}
Pop-Location
