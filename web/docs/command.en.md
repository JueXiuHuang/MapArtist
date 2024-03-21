# Command

All commands should add **"bot"** prefix.  
For example, `bot hungry`.

---

## :fontawesome-solid-scroll: hungry

Check if player is hungry or not.

```txt
bot hungry
```

---

## :fontawesome-solid-scroll: stop

Stop all players' or specific player's working progress.

```txt
bot stop all
bot stop {user_name}
```

---

## :fontawesome-solid-scroll: start

Ask specific player or all players start to work.

```txt
bot start all
bot start {user_name}
```

---

## :fontawesome-solid-scroll: bar

Display working progress bar.

```txt
bot bar
```

---

## :fontawesome-solid-scroll: csafe

Execute csafe command, it's equivalent to `bot cmd csafe`.

:warning: It can only be used in mcfallout.

```txt
bot csafe
```

---

## :fontawesome-solid-scroll: cmd

Execute command in minecraft.  
For example `bot cmd gamemode survival` is equivalent to `/gamemode survival`.

```txt
bot cmd {user_name} {command}

# Example
bot cmd player211 gamemode survival
```

---

## :fontawesome-solid-scroll: assign

Set specific player's working column. Default value is 0.

```txt
bot assign {user_name} {column}

# Example
bot assign player211 1
```

---

## :fontawesome-solid-scroll: worker

Set max worker number. Default value is 1.

```txt
bot worker {number}

# Example
bot worker 3
```

---

## :fontawesome-solid-scroll: duty

Display all players' set assign value and worker value.

```txt
bot duty
```

---

## :fontawesome-solid-scroll: detail

Display bot's detail info. Useful in discord bot.

```txt
bot detail
```

---

## :fontawesome-solid-scroll: default

Reset all players' assign & worker value.

```txt
bot default
```

---

## :fontawesome-solid-scroll: ingot

Display current emerald <-> villager ingot exchange rate.

:warning: It can only be used in mcfallout.

```txt
bot ingot
```

---

## :fontawesome-solid-scroll: name

Display bot's name.

```txt
bot name
```

---

## :fontawesome-solid-scroll: channel

Display player's current channel.  

:warning: It can only be used in mcfallout.

```txt
bot channel
```

---

## :fontawesome-solid-scroll: move

Ask player move to specified position with self defined method.

```txt
bot move {x} {y} {z}

# Example
bot move 110, 65, 35
```

---

## :fontawesome-solid-scroll: bmove

Ask player move to specified position with botcraft GoTo.

```txt
bot bmove {x} {y} {z}

# Example
bot bmove 110, 65, 35
```
