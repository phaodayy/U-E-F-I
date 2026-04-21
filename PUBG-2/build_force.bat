@echo off
taskkill /F /IM GameOverlay.exe /T 2>nul
set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe"
"%MSBUILD_PATH%" phao_final.sln /p:Configuration=Release /p:Platform=x64 /t:GameOverlay
if %ERRORLEVEL% EQU 0 (
    if not exist "..\bin" mkdir "..\bin"
    copy /Y "bin\GameOverlay.exe" "..\bin\GameOverlay.exe" >nul 2>&1
    echo [+++] GameOverlay EXE BUILT AND STAGED IN: bin\GameOverlay.exe [+++]
    echo [OK] SUCCESS
) else (
    echo [FAIL] MSBuild Error
)
pause
