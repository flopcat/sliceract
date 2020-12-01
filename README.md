# sliceract
Slicing tool for tesseract.

Sometimes tesseract can give sub-optimal results for multi-column text.  This tool is designed for the user to select custom regions of an image and feed them to tesseract, so a more specific form of ocr may be used.  The ocr output ends up in the clipboard.

Designed and tested on Linux.  Presumably it will also work on Windows if `tesseract` is in the path.

## Usage

Click "Browse" to select an image.  Click "Load" to load the image into the program.  Once loaded, select a rectangle with mouse's left and right buttons, where each button corresponds to a corner of the rectangle.  Click "Slice to clipboard" to run tesseract on the selected region; the output of tesseract will be placed in the clipboard.  Paste into your favorite editor.  The clipboard output usually has a trailing invisibile character, so hit backspace once after paste.
