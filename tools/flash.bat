@echo off
REM One-command build + flash for the CYD D&D firmware (Windows).
REM Installs PlatformIO if it's missing, then builds and uploads over USB.
REM Run by double-clicking this file, or from a terminal: tools\flash.bat
cd /d "%~dp0.."

where pio >nul 2>nul
if errorlevel 1 (
    echo PlatformIO not found - installing ^(pip install -U platformio^)...
    pip install -U platformio
)

echo Building and flashing... (connect the board over USB first)
pio run -t upload

echo.
echo Done. Don't forget to copy sd_card_data\ onto the microSD card - see README.md.
pause
