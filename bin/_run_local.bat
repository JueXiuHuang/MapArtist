IF EXIST log.txt DEL /F log.txt
mapArtist.exe -c config_local.toml | tee -a log.txt
pause