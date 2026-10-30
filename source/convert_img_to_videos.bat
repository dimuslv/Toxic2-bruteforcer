"C:\Users\dimus\Downloads\LosslessCut-win-x64\resources\ffmpeg.exe" -framerate 30 -i img\%%d.png -c:v libx264rgb -qp 0 -preset veryslow vid_lossless.mp4
"C:\Users\dimus\Downloads\LosslessCut-win-x64\resources\ffmpeg.exe" -framerate 30 -i img\%%d.png -c:v libx264 -pix_fmt yuv420p -preset veryslow vid_lossy.mp4
pause