@echo off
chcp 65001 >nul
echo ============================================
echo  SOC Sim - Build Script
echo ============================================

set ROOT=%~dp0
set ALG=%ROOT%algorithm
set CHAT=%ROOT%chatgpt
set SIMULINK=%ROOT%simulink
set BUILD=%ROOT%build

echo.
echo [1/2] Setting up MSVC x64 environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if %ERRORLEVEL% neq 0 (
    echo FAILED: MSVC environment setup
    pause
    exit /b 1
)
echo OK

echo.
echo [2/2] Compiling...
if not exist "%BUILD%" mkdir "%BUILD%"

cl /O2 /utf-8 /W3 /Fo"%BUILD%/" /Fe"%BUILD%\soc_sim.exe" ^
    /I"%ALG%" /I"%SIMULINK%" /I"%CHAT%" ^
    "%ALG%\lib.c" ^
    "%ALG%\data.c" ^
    "%ALG%\toolfun.c" ^
    "%ALG%\batterychgdata.c" ^
    "%ALG%\soc.c" ^
    "%CHAT%\maintask.c" ^
    "%SIMULINK%\sim.c" ^
    "%ROOT%\sim_main.c" ^
    /link /NODEFAULTLIB:libcmtd

if %ERRORLEVEL% equ 0 (
    echo.
    echo ============================================
    echo  BUILD SUCCESS
    echo  Output: %BUILD%soc_sim.exe
    echo ============================================
) else (
    echo.
    echo ============================================
    echo  BUILD FAILED (error level %ERRORLEVEL%)
    echo ============================================
)

echo.
echo To run: type "%BUILD%soc_sim.exe" ^< data.csv
pause
