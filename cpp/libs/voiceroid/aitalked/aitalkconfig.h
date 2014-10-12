#pragma once

// aitalkconfig.h
// 10/11/2014 jichi

#define AITALK_ENCODING "SHIFT-JIS"

// These values are got by debugging AITalkAPI_Init using OllyDbg at runtime
#define AITALK_CONFIG_FREQ          0x5622          // = 22050 Hz, AITalk_TConfig::hzVoiceDB, AITalkUtil::voiceSamplesPerSec
#define AITALK_CONFIG_TIMEOUT       1000            // = 1000 msec, AITalk_TConfig::msecTimeout
#define AITALK_CONFIG_CODEAUTHSEED  "NqKN148elpuO2tmdCMCU" // AITalk_TConfig::codeAuthSeed
#define AITALK_CONFIG_LENAUTHSEED   0               // AITalk_TConfig::lenAuthSeed
#define AITALK_CONFIG_LICENSE       "aitalk.lic"    // relative path of the license file

// EOF
