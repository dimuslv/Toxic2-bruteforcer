mkdir vidimg

"C:\Users\dimus\Downloads\LosslessCut-win-x64\resources\ffmpeg.exe" -i %1 -start_number 0 "vidimg\%%d.png"
pause