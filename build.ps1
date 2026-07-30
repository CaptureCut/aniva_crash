Write-Host "[BUILD] Removing old build..."
if (Test-Path "build") {
    Remove-Item -Recurse -Force "build"
}

# Добавляем snap cmake в PATH
$env:PATH = "/snap/bin:" + $env:PATH

Write-Host "[BUILD] Configuring..."
cmake -S . -B build

Write-Host "[BUILD] Building engine..."
cmake --build build --target aniva_crash --parallel

Write-Host "[BUILD] Building bootstrap..."
cmake --build build --target bootstrap --parallel

Write-Host "[BUILD] Done."
