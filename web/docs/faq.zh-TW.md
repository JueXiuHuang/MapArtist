# 常見問題

## 要如何登入 Minecraft 帳號?

當你第一次啟動 MapArtist，你會看到如下的訊息要求你打開網頁並輸入序號來登入你的 Microsoft 帳號。

```bash hl_lines="7-8"
[2024-04-06 23:13:54.115] [INFO] [main(6664)] Authentifier.cpp(102): Trying to get Microsoft access token...
[2024-04-06 23:13:54.115] [ERROR] [main(6664)] Authentifier.cpp(696): Error trying to get cached Microsoft credentials
[2024-04-06 23:13:54.116] [INFO] [main(6664)] Authentifier.cpp(698): Starting authentication process...
[Sat Apr  6 23:13:55 2024] DEBUG: Cluster: 996 of 1000 session starts remaining
[Sat Apr  6 23:13:55 2024] INFO: Auto Shard: Bot requires 1 shard
[Sat Apr  6 23:13:55 2024] DEBUG: Starting with 1 shards...
To sign in, use a web browser to open the page https://www.microsoft.com/link and enter the code FH5F8SF6 to authenticate.
```

複製連結和序號，並使用瀏覽器開啟連結，你會看到如下的畫面，接著將序號輸入到中間的輸入框內，接著會要求你登入 Microsoft 帳號並授權給 Botcraft，當一切都結束後，你可以關閉瀏覽器，並回到 MapArtist。

![Login](site:images/login.png)

MapArtist 將會自動接續流程並連接 Minecraft 伺服器，所有令牌將會儲存在 `botcraft_cached_credentials.json`。你可以開始享受 bot 了！

---

## 如何設定 Discord 機器人

1. 建立 Discord 機器人: 前往 [Discord Create App](https://discord.com/developers/applications?new_application=true) 並建立一個 Discord 機器人。

    ![Login](site:images/create_app.png)

2. 取得邀請連結: 切換到 OAuth2 頁面並設定權限。你會看到頁面最下方有邀請連結。

    ![Login](site:images/OAuth2.png)
    ![Login](site:images/permission.png)

3. 使用邀請連結邀請你的 Discord 機器人到你的伺服器。

4. 取得令牌: 按下 "Reset Token" 按鈕。 記得打開 "MESSAGE CONTENT INTENT"。

    ![Login](site:images/token.png)

5. 將你的令牌加入到私人設定檔中。
