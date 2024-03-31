# Usage

Here we will tell you how to use MapArtist to build your owesome map art !

---

## Config File

We use config file to set every parameters. Below is an example of config file.  

Minimum Example Config:

``` toml title='config.toml'
[server]
# server ip
address = "127.0.0.1:25565"
# player name (only used when online is false)
playerName = "test"
# whether the server needs online validation
online = false
# whether reconnect to the server after disconnecting
reconnect = true

[private]
# the path of another config file storing more private setting
name = "private.toml"

[nbt]
# anchor means the first block in your schematic file (nbt)
anchor = "10,60,-15"

# the nbt file you want to build
name = "test.nbt"

# [deprecated]
tempblock = "minecraft:cobblestone"

[other]
# the command to tp yourself if you are stuck.
home = "tp @p -70 -60 -80"


[algorithm]
# available algorithm: slice_dfs
method = "slice_dfs"

# retry is the number of times we retry an action
retry = 12

[move]
# whether use flash instead of walking 
use_flash = true


# The fields below represent each block and the position of the chest they are stored in.
# Recycle is the chest you can put everything in it.
# You can use item classifier's source chest as recycle chest.
[[chests]]
name = "recycle"
coordinate = ["-62,-50,-91", "-62,-50,-92", "-62,-50,-93", "-62,-50,-94", "-62,-50,-95", "-62,-50,-96"]
# Food
[[chests]]
name = "minecraft:cooked_beef"
coordinate = ["-68,-60,-92"]
# Mertials
[[chests]]
name = "minecraft:white_wool"
coordinate = ["-61,-59,-93", "-61,-58,-93"]

```

``` toml title='private.toml'
discord_enable = true
discord_token = "{discord bot token}"
discord_channel = "{discord channel id}"
```

---

## Run

Double click the executable file. MapArtist will look for `config_local.toml` and read the setting from it. You can also specify the config file you want by the below command.

```shell
mapArtist.exe --config {custom config file}
# or
mapArtist.exe -c {custom config file}

# Example
mapArtist.exe -c my_config.toml
```

---

## Log

During execution, MapArtist prints a lot of message. If you want to trace some information, it's better to redirect log message to a file. Here we list an approach to redirect all message to a file in a shell.

```shell
mapArtist.exe > {file name}

# Example
mapArtist.exe > log.txt
```

---

## Note

- Before you start the bot, you can only leave shears, axe, pickaxe, shovel in your inventory.

- If you want to control bot with discord, you can create a discord bot yourself. [link](https://dpp.dev/creating-a-bot-application.html)