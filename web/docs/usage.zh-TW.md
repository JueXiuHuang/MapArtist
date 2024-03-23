# 使用方法

本篇將會告訴你如何使用 MapArtist！

---

## 設定檔案

我們會使用設定檔來設定所有的參數，以下是設定檔的範例。

``` toml title='config.toml'
[server]
# 伺服器地址
address = "127.0.0.1:25565"
# 玩家名稱 (只在 online 為 false 時使用)
playerName = "test"
# 是否為需要線上驗證的伺服器
online = false
# 是否在斷線時重新連線
reconnect = true

[private]
# 私人設定檔案的設定檔案路徑
name = "private.toml"

[nbt]
# NBT 檔案的第一個方塊的起始位置
anchor = "10,60,-15"

# NBT 檔案
name = "test.nbt"

# [棄用]
tempblock = "minecraft:cobblestone"

[other]
# 卡住時，傳送的 TP 指令
home = "tp @p -70 -60 -80"


[algorithm]
# 可用的演算法: slice_dfs
method = "slice_dfs"

# 動作的嘗試次數
retry = 12


# 下方為物品箱子的座標列表
# 回收箱為回收所有材料的地方
# 你可以使用分類器的收集箱當作回收箱
[[chests]]
name = "recycle"
coordinate = ["-62,-50,-91", "-62,-50,-92", "-62,-50,-93", "-62,-50,-94", "-62,-50,-95", "-62,-50,-96"]
# 食物
[[chests]]
name = "minecraft:cooked_beef"
coordinate = ["-68,-60,-92"]
# 材料
[[chests]]
name = "minecraft:white_wool"
coordinate = ["-61,-59,-93", "-61,-58,-93"]

```

``` toml title='private.toml'
discord_enable = true
discord_token = "{Discord Bot 權杖}"
discord_channel = "{Discord 頻道 ID}"
```

---

## 執行

雙擊 MapArtist.exe 就會運行，MapArtist 會讀取 `config_local.toml` 設定檔，如果你想要使用其他設定檔，可以參考下方的指令。

```shell
mapArtist.exe --config {custom config file}
# 或
mapArtist.exe -c {custom config file}

# 範例
mapArtist.exe -c my_config.toml
```

---

## 紀錄檔

在執行期間，MapArtist 會印出許多的訊息，如果你想要追蹤某些資訊，你可以將訊息重新導向到檔案中並記錄起來，這裡我們提供你一個在終端機中重新導向的方法。

```shell
mapArtist.exe > {file name}

# 範例
mapArtist.exe > log.txt
```

---

## 其他重點

- 在你開始 Bot 之前，你身上只能有剪刀、斧頭、鎬子和鏟子。

- 如果你想要使用 Discord 操控 Bot，你可以建立一個 Discord Bot [連結](https://dpp.dev/creating-a-bot-application.html)。