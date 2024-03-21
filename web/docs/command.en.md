# Command

All commands should add **"bot "** prefix. For example `bot hungry`.

## hungry

Check if player is hungry or not.

## stop all / {user_name}

Stop all players' or specific player's working progress.\
For example `bot stop 211` or `bot stop all`

## start all / {user_name}

Ask specific player or all players start to work.\
For example `bot start 211` or `bot start all`.

## bar

Display working progress bar.

## csafe

Execute csafe command. Used in mcfallout.

## cmd

Execute command in minecraft.\
For example `bot cmd gamemode survival` is equivalent to `/gamemode survival`.

## assign {user_name} {column}

Define specific player's working column. Default value is 0.\
Example: `bot assign chishin 2`.

## worker {number}

Define max worker number. Default value is 1.\
Example: `bot worker 3`.

## duty

Display all players' set assign value and worker value.\
Example: `bot duty`

## detail

Display prettify bot's detail info. Including bot's name, map art file name, duty info and current channel number (in mcfallout). This command is useful in discord bot.\
Example: `bot detail`

## default

Reset all players' assign & worker value.

## ingot

Display current emerald <-> villager ingot exchange rate. Used in mcfallout.

## name

Display bot's name.

## channel

Display player's current channel. Used in mcfallout.

## move {x} {y} {z}

Ask player move to specified position with self defined method.

## bmove {x} {y} {z}

Ask player move to specified position with botcraft GoTo.