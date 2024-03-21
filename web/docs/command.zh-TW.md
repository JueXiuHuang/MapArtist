# 指令

所有指令都應該加上 **" bot "** 前綴，比如 `bot hungry`。

---

## :fontawesome-solid-scroll: assign

設定特定玩家的工作列（column），預設值是 0。

```txt
bot assign {user_name} {column}

# 範例
bot assign player211 1
```

---

## :fontawesome-solid-scroll: bar

顯示 BOT 當前的工作進度。

```txt
bot bar
```

---

## :fontawesome-solid-scroll: bmove

使用 Botcraft 的 GoTo 讓 BOT 移動到指定座標。

```txt
bot bmove {x} {y} {z}

# 範例
bot bmove 110, 65, 35
```

---

## :fontawesome-solid-scroll: channel

顯示 BOT 所處分流。

:warning: 用於廢土伺服器。

```txt
bot channel
```

---

## :fontawesome-solid-scroll: cmd

執行 minceaft 的指令。

比如 `bot cmd gamemode survival` 就等於在遊戲內執行 `/gamemode survival`。

```txt
bot cmd {user_name} {command}

# 範例
bot cmd player211 gamemode survival
```

---

## :fontawesome-solid-scroll: csafe

執行 csafe 指令。

:warning: 用於廢土伺服器。

```txt
bot csafe
```

---

## :fontawesome-solid-scroll: default

初始化 BOT 的工作列與總數設定。

```txt
bot default
```

---

## :fontawesome-solid-scroll: detail

顯示排版過的 BOT 詳細資訊。其中包含 BOT 名稱、地圖畫檔案名稱、負責的工作列、BOT 總數以及當前所在分流（限定廢土伺服器）。

:star: 這個指令搭配 DC Bot 有奇效。

```txt
bot detail
```

---

## :fontawesome-solid-scroll: duty

顯示所有 BOT 負責的工作列以及 BOT 總數。

```txt
bot duty
```

---

## :fontawesome-solid-scroll: hungry

確認 BOT 是否處於飢餓狀態。

```txt
bot hungry
```

---

## :fontawesome-solid-scroll: ingot

顯示當前的綠寶石與村民錠交換匯率。

:warning: 用於廢土伺服器。

```txt
bot ingot
```

---

## :fontawesome-solid-scroll: move

讓 BOT 移動到指定座標。

```txt
bot move {x} {y} {z}

# 範例
bot move 110, 65, 35
```

---

## :fontawesome-solid-scroll: name

顯示 BOT 名稱。

```txt
bot name
```

---

## :fontawesome-solid-scroll: start

讓特定或全部 BOT 開始工作進程。

```txt
bot start all
bot start {user_name}
```

---

## :fontawesome-solid-scroll: stop

停止特定或全部 BOT 的工作進程。

```txt
bot stop all
bot stop {user_name}
```

---

## :fontawesome-solid-scroll: worker

設定 BOT 的數量，預設值為 1。

```txt
bot worker {number}

# 範例
bot worker 3
```
