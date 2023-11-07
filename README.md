# MapArtist

- [How to install MapArtist](#how-to-install-mapartist)
- [Tips](#tips)
  - [Simple batch file makes everything easier](#simple-batch-file-makes-everything-easier)
- [Commands](#commands)
  - [hungry](#hungry)
  - [stop all / \<user\_name\>](#stop-all--user_name)
  - [start all / \<user\_name\>](#start-all--user_name)
  - [bar](#bar)
  - [csafe](#csafe)
  - [cmd](#cmd)
  - [assign](#assign)
  - [worker](#worker)
  - [duty](#duty)
  - [default](#default)
  - [ingot](#ingot)
  - [channel](#channel)
  - [move](#move---)
  - [bmove](#bmove---)

## How to install MapArtist

### Dependencies

1. git
2. cmake

### Steps

1. Clone [MapArtist](https://github.com/JueXiuHuang/MapArtist.git)
  
    ```bash
    git clone https://github.com/JueXiuHuang/MapArtist.git
    ```

2. Change directory to botcraft, execute below command to build botcraft, it will take a few minutes to run.

    ```bash
    cd MapArtist
    cmake -S . -B build -G "MinGW Makefiles"
    cmake --build build
    ```

3. Executable file will in `MapArtist/bin`, use `mapArtist.exe --help` to see available arguments.

## Tips

### Simple batch file makes everything easier

You can write a simple batch file to start up the bot, for example:

```bash
IF EXIST log.txt DEL /F log.txt
mapArtist.exe | tee -a log.txt
pause
```

this script will delete old log file and execute the bot with logging.

## Commands

All commands should add **"bot"** prefix.  
For example `bot hungry`.

### hungry

Check if player is hungry or not.

### stop all / <user_name>

Stop all players' or specific player's working progress.

### start all / <user_name>

Ask specific player or all players start to work.  
For example `bot start 211` or `bot start all`.

### bar

Display working progress bar.

### csafe

Execute csafe command.

### cmd

Execute command in minecraft.  
For example `cmd gamemode survival` is equivalent to `/gamemode survival`.

### assign

Define specific player's working column. Default value is 0.  
Example: `assign chishin 2`.

### worker

Define max worker number. Default value is 1.  
Example: `worker 3`.

### duty

Display all players' set assign value and worker value.

### default

Reset all players' assign & worker value

### ingot

Display current emerald <-> villager ingot exchange rate. Used in mcfallout.

### channel

Display player's current channel. Used in mcfallout.

### move <x\> <y\> <z\>

Ask player move to specified position with self defined method.

### bmove <x\> <y\> <z\>

Ask player move to specified position with botcraft GoTo.
