# MapArtist

- [MapArtist](#mapartist)
  - [How to install MapArtist](#how-to-install-mapartist)
    - [Dependencies](#dependencies)
    - [Steps](#steps)
  - [Tips](#tips)
    - [Simple batch file makes everything easier](#simple-batch-file-makes-everything-easier)
  - [How to use MapArtist (After installation)](#how-to-use-mapartist-after-installation)
  - [Commands](#commands)
    - [hungry](#hungry)
    - [stop all / {user\_name}](#stop-all--user_name)
    - [start all / {user\_name}](#start-all--user_name)
    - [bar](#bar)
    - [csafe](#csafe)
    - [cmd](#cmd)
    - [assign {user\_name} {column}](#assign-user_name-column)
    - [worker {number}](#worker-number)
    - [duty](#duty)
    - [detail](#detail)
    - [default](#default)
    - [ingot](#ingot)
    - [name](#name)
    - [channel](#channel)
    - [move {x} {y} {z}](#move-x-y-z)
    - [bmove {x} {y} {z}](#bmove-x-y-z)

## How to install MapArtist

### Dependencies

1. cmake
2. MSVC

### Steps

1. Download latest build from release.

1. Execute mapArtist.exe in `MapArtist/bin`, use `mapArtist.exe --help` to see available arguments.

## Tips

### Simple batch file makes everything easier

You can write a simple batch file to start up the bot, for example:

```bat
IF EXIST log.txt DEL /F log.txt
mapArtist.exe | tee -a log.txt
pause
```

this script will delete old log file and execute the bot with logging.

## How to use MapArtist (After installation)
1. Modify config file `bin/config_online.txt` to fit your minecraft environment.
2. Prepare map art nbt file and put it under `bin/` folder.
3. Before you start the bot, you can only leave shears, axe, pickaxe, shovel in your inventory.
4. You can start the bot with `mapArtist.exe`, and use commands to control it.
5. If you want to control bot with discord, you can create a discord bot yourself. [link](https://dpp.dev/creating-a-bot-application.html)

## Commands

All commands should add **"bot"** prefix.  
For example `bot hungry`.

### hungry

Check if player is hungry or not.

### stop all / {user_name}

Stop all players' or specific player's working progress.

### start all / {user_name}

Ask specific player or all players start to work.\
For example `bot start 211` or `bot start all`.

### bar

Display working progress bar.

### csafe

Execute csafe command.

### cmd

Execute command in minecraft.\
For example `cmd gamemode survival` is equivalent to `/gamemode survival`.

### assign {user_name} {column}

Define specific player's working column. Default value is 0.\
Example: `assign chishin 2`.

### worker {number}

Define max worker number. Default value is 1.\
Example: `worker 3`.

### duty

Display all players' set assign value and worker value.

### detail

Display bot's detail info. Useful in discord bot.

### default

Reset all players' assign & worker value.

### ingot

Display current emerald <-> villager ingot exchange rate. Used in mcfallout.

### name

Display bot's name.

### channel

Display player's current channel. Used in mcfallout.

### move {x} {y} {z}

Ask player move to specified position with self defined method.

### bmove {x} {y} {z}

Ask player move to specified position with botcraft GoTo.
