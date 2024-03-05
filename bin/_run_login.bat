IF EXIST log_online.txt DEL /F log_online.txt
mapArtist.exe -a sg.mcfallout.net -m -c config_online.toml | tee -a log_online.txt
pause