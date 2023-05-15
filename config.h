#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "privateConfig.h"
#include "GatewayIntents.h"

// Intent options can be found in GatewayIntents.h
const uint16_t gateway_intents = GUILD_MESSAGES_INTENT | GUILD_MESSAGE_TYPING_INTENT | GUILD_VOICE_STATES_INTENT;

// discord.gg certificate fingerprint
const char * certificateFingerprint = "f3 9a e0 66 27 b5 7d d3 3f 20 fd f4 46 37 64 56 18 34 b8 a3";

//to fix connection issuees with WebSocket, set tools/debug_port to serial and set tools/debug_level to SSL. 
//Check that the certificate fingerprint received is the same as the one above. 


static const char* USER_ID = "YourDiscordUserID";
//static const uint8_t LED_PIN = 2;

#endif //CONFIG_H
