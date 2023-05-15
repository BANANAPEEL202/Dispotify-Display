# Dispotify Display
Serves as a minimalistic clock display, Discord call tracker, and Spotify tracker. When you're using Discord, it'll display the users in the same call as you, whether they're muted or deafened, and how long you've been in the call. In Spotify mode, it'll show you the current song playing and how long you've been listening to Spotify.

## Usage
### Discord

https://user-images.githubusercontent.com/67805049/235053712-805bd2f1-c291-4648-be54-de53a5852389.mp4

### Spotify

https://github.com/BANANAPEEL202/Dispotify-Display/assets/67805049/c7822113-62e8-4664-a488-7b646ac8c4d6


## Installation

1. Install ESP board for Arduino as outlined on the [official documentation](https://arduino-esp8266.readthedocs.io/en/latest/installing.html)

2. Create a standard Discord bot at https://discord.com/developers/applications. Add this bot to whichever servers you would like to keep track of.  

3. Create a standard Spotify app at https://developer.spotify.com/dashboard

2. Set configuration in `config.h` and `privateConfig.h`.

    |Name|Value|Location|
    |-|-|-|
    |`wifi_ssid`|SSID of the wifi to connect to|`privateConfig.h`|
    |`wifi_password`|Password for the wifi to connect to|`privateConfig.h`|
    |`bot_token`|Token to authenticate websocket connection<br/> Tokens can be accessed under Discord Developer Portal->App->Bot->Reset Token] |`privateConfig.h`|
    |`USER_ID`|Your Discord account ID. Can be accessed through Discord->User Settings->My Account->Copy User ID|`config.h`|
    |`clientID`|Client ID of your Spotify app (found under the Settings section of your app)|`privateConfig.h`|
    |`clientSecret`|Client Secret of your Spotify app (found under the Settings section of your app)|`privateConfig.h`|
    

4. Build and upload to ESP board

- Heavily based on work done by this repo: https://github.com/Cimera42/esp-discord-client

