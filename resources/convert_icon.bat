@echo off
echo Converting PNG to ICO format...

rem This script requires ImageMagick to be installed
rem Download from: https://imagemagick.org/script/download.php

if exist "icon.png" (
    magick convert icon.png -define icon:auto-resize=256,128,64,48,32,16 icon.ico
    if exist "icon.ico" (
        echo Successfully created icon.ico
    ) else (
        echo Failed to create icon.ico
        echo Please install ImageMagick or use an online converter
        echo Online converter: https://convertio.co/png-ico/
    )
) else (
    echo icon.png not found!
)

pause
