# IMPORTANT: Icon File Required

The file `icon.ico` is required for building the application but is not included in the repository.

## Quick Solution

**Option 1: Use Online Converter (Fastest)**
1. Go to https://convertio.co/png-ico/
2. Upload `icon.png` from this folder
3. Download the converted `icon.ico` and place it here

**Option 2: Use ImageMagick**
```powershell
# Install ImageMagick from https://imagemagick.org/script/download.php
# Then run:
magick icon.png -define icon:auto-resize=256,128,64,48,32,16 icon.ico
```

**Option 3: Temporary Workaround**
For testing purposes, you can use any small ICO file or create a dummy one.
The build will work with any valid ICO file.

## After Creating icon.ico

Once you have `icon.ico` in this folder, you can build the project normally.
