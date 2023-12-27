IF EXIST log.txt DEL /F log.txt
mapArtist.exe | tee -a log.txt
pause