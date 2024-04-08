# FAQ

## How can I login my Minecraft account?

When you run MapArtist for the first time, you will see the message that asks you to open the page and enter the code to sign in to your Microsoft account, just like the message below.

```bash hl_lines="7-8"
[2024-04-06 23:13:54.115] [INFO] [main(6664)] Authentifier.cpp(102): Trying to get Microsoft access token...
[2024-04-06 23:13:54.115] [ERROR] [main(6664)] Authentifier.cpp(696): Error trying to get cached Microsoft credentials
[2024-04-06 23:13:54.116] [INFO] [main(6664)] Authentifier.cpp(698): Starting authentication process...
[Sat Apr  6 23:13:55 2024] DEBUG: Cluster: 996 of 1000 session starts remaining
[Sat Apr  6 23:13:55 2024] INFO: Auto Shard: Bot requires 1 shard
[Sat Apr  6 23:13:55 2024] DEBUG: Starting with 1 shards...
To sign in, use a web browser to open the page https://www.microsoft.com/link and enter the code FH5F8SF6 to authenticate.
```

Copy the link and paste it to the browser search bar, then press Enter. You will see something similar to the image below. Then, paste the code into the field. The following steps will require you to log in Microsoft account and authorize Botcraft. Finally, when everything is done. You can close the browser and return to MapArtist.

![Login](site:images/login.png)

MapArtist will automatically continue running and connect to Minecraft. All tokens will be stored in `botcraft_cached_credentials.json`. Now you can enjoy the bot!

---

## How to set the Discord bot

1. Create a Discord bot: Go to [Discord Create App](https://discord.com/developers/applications?new_application=true) and create a Discord bot.

    ![Login](site:images/create_app.png)

2. Get the invitation link: Switch to OAuth2 tab and select the permission. You will find the invitation link at the bottom.

    ![Login](site:images/OAuth2.png)
    ![Login](site:images/permission.png)

3. Invite the bot to your Discord server.

4. Get the token: Press "Reset Token" button. Remember to turn "MESSAGE CONTENT INTENT" on.

    ![Login](site:images/token.png)

5. Add the token to your private configuration file.

---

## Error: You are already connected to this proxy

If you see this message in the console, it means that your account is already connected to the server somewhere else. Please make sure that you have closed all connections before running the bot.

```text
[INFO] [NetworkPacketProcessing - player(21620)] ConnectionClient.cpp(86): Disconnect during login with reason: {"extra":[{"color":"red","text":"You are already connected to this proxy!"}],"text":""}
```
