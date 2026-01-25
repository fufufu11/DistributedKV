$BuildDir = "build"
$CmakePath = "cmake"
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
    Write-Host "CMake not found" -ForegroundColor Red
    exit 1
}
if ($args -contains "--clean") {
    if (Test-Path $BuildDir) { Remove-Item -Recurse -Force $BuildDir }
}
if (!(Test-Path $BuildDir)) { New-Item -ItemType Directory -Path $BuildDir | Out-Null }
Push-Location $BuildDir
$CmakeArgs = @("..", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
if (!(Test-Path "CMakeCache.txt")) {
    if (Get-Command g++ -ErrorAction SilentlyContinue) {
        $CmakeArgs += "-G", "MinGW Makefiles"
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
        if (!(Test-Path $CtestPath)) { $CtestPath = "ctest" }
        & $CtestPath --output-on-failure
    }
}
Pop-Location
