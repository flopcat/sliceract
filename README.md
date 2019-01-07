# sliceract
Slicing tool for tesseract.

Sometimes tesseract can give sub-optimal results for multi-column text.  This tool is designed for the user to select custom regions of an image and feed them to tesseract, so a more specific form of ocr may be used.  The ocr output ends up in the clipboard.

Designed and tested on Linux.  Presumably it will also work on Windows if `tesseract` is in the path.
