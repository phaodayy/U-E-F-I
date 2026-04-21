@echo off
taskkill /F /IM GameOverlay.exe /T 2>nul
set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe"
"%MSBUILD_PATH%" phao_final.sln /p:Configuration=Release /p:Platform=x64 /t:GameOverlay
if %ERRORLEVEL% EQU 0 (
    echo [OK] SUCCESS
) else (
    echo [FAIL] MSBuild Error
)
pause
