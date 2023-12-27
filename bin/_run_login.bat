IF EXIST log_online.txt DEL /F log_online.txt
mapArtist.exe -a jp.mcfallout.net -m -c config_online.txt | tee -a log_online.txt
pause