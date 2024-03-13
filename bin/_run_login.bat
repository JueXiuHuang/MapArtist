IF EXIST log_online.txt DEL /F log_online.txt
mapArtist.exe -c config_online.toml | tee -a log_online.txt
pause