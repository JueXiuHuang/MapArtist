# 指令

所有指令都應該加上 **"bot "** 前綴，比如 `bot hungry`。

## hungry

確認 BOT 是否處於飢餓狀態。

## stop all / {user_name}

停止特定或全部 BOT 的工作進程。\
使用範例：`bot stop 211` 或 `bot stop all`

## start all / {user_name}

讓特定或全部 BOT 開始工作進程。\
使用範例：`bot start 211` 或 `bot start all`

## bar

顯示 BOT 當前的工作進度。

## csafe

執行 csafe 指令，主要用於廢土伺服器。

## cmd

執行 minceaft 的指令。\
比如 `bot cmd gamemode survival` 就等於在遊戲內執行 `/gamemode survival`。

## assign {user_name} {column}

設定特定玩家的工作列（column），預設值是 0。\
使用範例：`bot assign chishin 2`

## worker {number}

設定 BOT 的數量，預設值為 1。\
使用範例：`bot worker 3`.

## duty

顯示所有 BOT 負責的工作列以及 BOT 總數。\
使用範例：`bot duty`

## detail

顯示排版過的 BOT 詳細資訊。其中包含 BOT 名稱、地圖畫檔案名稱、負責的工作列、BOT 總數以及當前所在分流（用於廢土伺服器）。這個指令搭配 DC Bot 有奇效。\
使用範例：`bot detail`

## default

初始化 BOT 的工作列與總數設定。

## ingot

顯示當前的綠寶石與村民錠交換匯率，用於廢土伺服器。

## name

顯示 BOT 名稱。

## channel

顯示 BOT 所處分流，用於廢土伺服器。

## move {x} {y} {z}

讓 BOT 移動到指定座標。

## bmove {x} {y} {z}

使用 Botcraft 的 GoTo 讓 BOT 移動到指定座標。