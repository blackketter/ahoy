/*

Lua Opus Library

License: CC BY-SA (http://creativecommons.org/licenses/by-sa/2.5/)
This license lets others remix, tweak, and build upon a work even for 
commercial reasons, as long as they credit the original author and 
license their new creations under the identical terms.

Authors:        0.1 - Dean Blackketter 

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "opus/opus.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define OPUS_LUA_VERSION "0.1"
#define OPUS_LUA_AUTHORS "Dean Blackketter"

#define OPUS_APPLICATION_AUDIO_STR "audio"
#define OPUS_APPLICATION_RESTRICTED_LOWDELAY_STR "lowdelay"
#define OPUS_APPLICATION_VOIP_STR "voip"

#define OPUS_BANDWIDTH_FULLBAND_STR "full"
#define OPUS_BANDWIDTH_MEDIUMBAND_STR "medium"
#define OPUS_BANDWIDTH_NARROWBAND_STR "narrow"
#define OPUS_BANDWIDTH_SUPERWIDEBAND_STR "superwide"
#define OPUS_BANDWIDTH_WIDEBAND_STR "wide"

#define OPUS_BITRATE_MAX_STR "max"

#define OPUS_AUTO_STR "auto"

#define OPUS_SIGNAL_MUSIC_STR "music"
#define OPUS_SIGNAL_VOICE_STR "voice"

// TODO - These aren't used yet.  Needed for OPUS_SET_EXPERT_FRAME_DURATION
#define OPUS_FRAMESIZE_2_5_MS_NUM                (2.5)
#define OPUS_FRAMESIZE_5_MS_NUM                  (5)
#define OPUS_FRAMESIZE_10_MS_NUM                 (10)
#define OPUS_FRAMESIZE_20_MS_NUM                 (20)
#define OPUS_FRAMESIZE_40_MS_NUM                 (40)
#define OPUS_FRAMESIZE_60_MS_NUM                 (60)

// TODO - support repacketizer 

// TODO - relax these assumptions or verify that they are adequate
#define MAX_FRAME_SIZE (4000)
#define MAX_SAMPLES_PER_FRAME (5760)

// per opus 1.1 spec
#define MAX_SAMPLE_RATE (48000)

// TODO - Support channels beyond stereo
#define MAX_CHANNELS (2)

// bytes per sample for each channel
#define SAMPLE_SIZE (sizeof(opus_int16))


//------------------------------------------------------------------------------
// Prototypes
//------------------------------------------------------------------------------

LUALIB_API int opus_version(lua_State *L);
LUALIB_API int opus_help(lua_State *L);
LUALIB_API int luaopen_opus(lua_State *L);

LUALIB_API int opus_enc_new(lua_State *L);
LUALIB_API int opus_enc_encode(lua_State *L);
LUALIB_API int opus_enc_free(lua_State *L);
LUALIB_API int opus_enc_bitrate(lua_State *L);
LUALIB_API int opus_enc_complexity(lua_State *L);
LUALIB_API int opus_enc_application(lua_State *L);
LUALIB_API int opus_enc_signal(lua_State *L);
LUALIB_API int opus_enc_bandwidth(lua_State *L);
LUALIB_API int opus_enc_force_channels(lua_State *L);
LUALIB_API int opus_enc_fec(lua_State *L);
LUALIB_API int opus_enc_vbr(lua_State *L);
LUALIB_API int opus_enc_vbr_constraint(lua_State *L);
LUALIB_API int opus_enc_reset(lua_State *L);

LUALIB_API int opus_dec_new(lua_State *L);
LUALIB_API int opus_dec_decode(lua_State *L);
LUALIB_API int opus_dec_free(lua_State *L);
LUALIB_API int opus_dec_bandwidth(lua_State *L);
LUALIB_API int opus_dec_samples_per_frame(lua_State *L);
LUALIB_API int opus_dec_channels(lua_State *L);
LUALIB_API int opus_dec_frames(lua_State *L);
LUALIB_API int opus_dec_sample_count(lua_State *L);
LUALIB_API int opus_dec_gain(lua_State *L);
LUALIB_API int opus_dec_pitch(lua_State *L);
LUALIB_API int opus_dec_reset(lua_State *L);

const char* opus_error_string(int x);
int opus_check_error(lua_State *L, int x);
void register_dec(lua_State *L);
void register_enc(lua_State *L);
OpusEncoder* check_enc(lua_State* L, int index);
OpusDecoder* check_dec(lua_State* L, int index);

//------------------------------------------------------------------------------

/* functions exposed to lua */

static const luaL_reg enc_functions[] = {
  {"encode", opus_enc_encode},
  {"bitrate", opus_enc_bitrate},
  {"complexity", opus_enc_complexity},
  {"application", opus_enc_application},
  {"signal", opus_enc_signal},
  {"bandwidth", opus_enc_bandwidth},
  {"force_channels", opus_enc_force_channels},
  {"fec", opus_enc_fec},
  {"signal", opus_enc_signal},
  {"vbr", opus_enc_vbr},
  {"vbr_constraint", opus_enc_vbr_constraint},
  {"reset", opus_enc_reset},
  {NULL, NULL}
};

static const luaL_Reg enc_gc_functions[] = {
  {"__gc", opus_enc_free}, 
  {NULL, NULL} 
};

static const luaL_reg dec_functions[] = {
  {"decode", opus_dec_decode},
  {"bandwidth", opus_dec_bandwidth},
  {"samples_per_frame", opus_dec_samples_per_frame},
  {"channels", opus_dec_channels},
  {"frames", opus_dec_frames},
  {"samples", opus_dec_sample_count},
  {"gain", opus_dec_gain},
  {"pitch", opus_dec_pitch},
  {"reset", opus_dec_reset},
  {NULL, NULL}
};

static const luaL_Reg dec_gc_functions[] = {
  {"__gc", opus_dec_free}, 
  {NULL, NULL} 
};

static const luaL_reg opus_functions[] = {
  {"version", opus_version},
  {"help", opus_help},
  {"newencoder", opus_enc_new},
  {"newdecoder", opus_dec_new},
  {NULL, NULL}
};


//------------------------------------------------------------------------------
// Utility
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  checks for error code, throws error if there is one, otherwise, returns value
int opus_check_error(lua_State *L, int x) {
  // all opus errors are negative
  if (x<0)
    return luaL_error(L, "%s (%d)", opus_strerror(x), x);
  else
    return x;
}

//------------------------------------------------------------------------------
// Encoder
//------------------------------------------------------------------------------

OpusEncoder* check_enc(lua_State* L, int index)
{
  void* ud = 0;
  luaL_checktype(L, index, LUA_TTABLE);
  lua_getfield(L, index, "__self");
  ud = luaL_checkudata(L, -1, "opus.encoder");
  lua_pop(L,1);
  luaL_argcheck(L, ud != 0, 0,"`opus.encoder' expected");  
  
  return *((OpusEncoder**)ud);      
}


//------------------------------------------------------------------------------

LUALIB_API int opus_enc_new(lua_State *L){

  int app_int;
  int error;
  
  int n = lua_gettop(L);  // Number of arguments
  if (n != 3) 
      return luaL_error(L, "Got %d arguments expected 3 (samplerate, channels, application)", n); 

  opus_int32 Fs = luaL_checknumber (L, 1);      
  int channels = luaL_checknumber (L, 2); 
  const char* application = luaL_checkstring (L, 3);      

  if (!strcmp(application, OPUS_APPLICATION_AUDIO_STR)) {
    app_int = OPUS_APPLICATION_AUDIO;
  } else if (!strcmp(application, OPUS_APPLICATION_RESTRICTED_LOWDELAY_STR)) {
    app_int = OPUS_APPLICATION_RESTRICTED_LOWDELAY;
  } else if (!strcmp(application, OPUS_APPLICATION_VOIP_STR)) {
    app_int = OPUS_APPLICATION_VOIP;
  } else {
    return luaL_error(L, "Application must be one of: audio, lowdelay or voip", n);
  }
  
  // new table for the instance of the encoder
  lua_newtable(L);
  
  // Get metatable 'opus.encoder' store in the registry
  luaL_getmetatable(L, "opus.encoder");
  
  lua_pushvalue(L, -1); // duplicates the metatable
  lua_setmetatable(L, -3);
  
// set the metatable as __index
  lua_setfield(L, -2, "__index");

 // save away the number of channels
  lua_pushinteger(L, channels);
  lua_setfield(L, -2, "channels");

  // save away the sample rate
  lua_pushinteger(L, Fs);
  lua_setfield(L, -2, "samplerate");

  // Allocate memory for a pointer to to object
  OpusEncoder **enc = (OpusEncoder **)lua_newuserdata(L, sizeof(OpusEncoder *));  
  luaL_getmetatable(L, "opus.encoder");
  lua_setmetatable(L, -2);

  
  // Set field '__self' of instance table to the encoder user data
  lua_setfield(L, -2, "__self");  

  *enc = opus_encoder_create ( Fs,  channels,  app_int,  &error);

  opus_check_error(L, error);

  return 1; 
}

//------------------------------------------------------------------------------

int opus_enc_encode(lua_State *L) 
{
  // probably too big to put on the stack.
  unsigned char temp_frame[MAX_FRAME_SIZE];

  int n = lua_gettop(L);  // Number of arguments
  
  
  if (n == 2) {
    OpusEncoder* enc = check_enc(L, 1);

    size_t pcm_len;

    const opus_int16 *pcm= (const opus_int16 *)lua_tolstring (L, 2, &pcm_len); 
    
    // need to use the saved channel count 
    //  which begs the questions: 
    //      why is there no accessor to the opus lib for this?  or
    //      why can't the opus lib do this calculation?
    lua_getfield(L, 1, "channels");
    int channels = luaL_checkint(L, -1);
    
    int frame_size = pcm_len / sizeof(opus_int16) / channels;
    
    opus_int32 encoded_bytes =	opus_encode (enc, pcm, frame_size, temp_frame, MAX_FRAME_SIZE);
    
    opus_check_error(L, encoded_bytes);
    
    lua_pushlstring(L, (char*)temp_frame, (size_t)encoded_bytes);
  } else {
    luaL_error(L, "Got %d arguments expected 2 (self, pcm data)", n); 
  }
  
  return 0;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_enc_free(lua_State *L){
  OpusEncoder* enc = check_enc(L, 1);

  opus_encoder_destroy(enc);
  
  return 0;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_enc_bitrate(lua_State *L){
  OpusEncoder* enc = check_enc(L, 1);
  int params = lua_gettop(L);
  opus_int32 bitrate;
  
  if (params == 2) {
    if (lua_isnumber(L,2)) {
      bitrate = luaL_checkint(L, 2);
      opus_check_error(L,opus_encoder_ctl(enc, OPUS_SET_BITRATE(bitrate)));
    } else if (lua_isstring(L,2)) {
      const char* bitrate_str = luaL_checkstring(L,2);
      if (!strcmp(bitrate_str, OPUS_AUTO_STR)) {
        bitrate = OPUS_AUTO;
      } else if (!strcmp(bitrate_str, OPUS_BITRATE_MAX_STR)) {
        bitrate = OPUS_BITRATE_MAX;
      } else {
        luaL_error(L, "Invalid bitrate (must be bits per second or 'auto' or 'max')");
      }
      lua_pushvalue(L, 2); // return a copy of what was passed in
    }
  } else if (params == 1) {
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_GET_BITRATE(&bitrate)));
    if (bitrate == OPUS_AUTO) {
      lua_pushstring(L, OPUS_AUTO_STR);
    } else if (bitrate == OPUS_BITRATE_MAX) {
      lua_pushstring(L, OPUS_BITRATE_MAX_STR);
    } else {
      lua_pushinteger(L, bitrate);
    }
  } else {
    luaL_error(L, "Got %d arguments expected only one or two (self, bitrate)", params);
  }
    
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_enc_complexity(lua_State *L){
  OpusEncoder* enc = check_enc(L, 1);
  int params = lua_gettop(L);
  opus_int32 complexity;
   
  if (params == 2) {
    complexity = luaL_checkint(L, 2);
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(complexity)));
  } else if (params == 1) {
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_GET_COMPLEXITY(&complexity)));
  } else {
    luaL_error(L, "Got %d arguments expected only one or two (self, complexity)", params);
  }
  
  lua_pushinteger(L, complexity);
  
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_enc_bandwidth(lua_State *L){
  OpusEncoder* enc = check_enc(L, 1);
  int params = lua_gettop(L);
  opus_int32 bandwidth;
  
  if (params == 2) {
    const char* bandwidth_str = luaL_checkstring(L,2);
    if (!strcmp(bandwidth_str, OPUS_AUTO_STR)) {
      bandwidth = OPUS_AUTO;
    } else if (!strcmp(bandwidth_str, OPUS_BANDWIDTH_NARROWBAND_STR)) {
      bandwidth = OPUS_BANDWIDTH_NARROWBAND;
    } else if (!strcmp(bandwidth_str, OPUS_BANDWIDTH_MEDIUMBAND_STR)) {
      bandwidth = OPUS_BANDWIDTH_MEDIUMBAND;
    } else if (!strcmp(bandwidth_str, OPUS_BANDWIDTH_WIDEBAND_STR)) {
      bandwidth = OPUS_BANDWIDTH_WIDEBAND;
    } else if (!strcmp(bandwidth_str, OPUS_BANDWIDTH_SUPERWIDEBAND_STR)) {
      bandwidth = OPUS_BANDWIDTH_SUPERWIDEBAND;
    } else if (!strcmp(bandwidth_str, OPUS_BANDWIDTH_FULLBAND_STR)) {
      bandwidth = OPUS_BANDWIDTH_FULLBAND;
    } else {
      luaL_error(L, "Invalid bandwidth (must be one of 'narrow','medium','wide','superwide','full', or 'auto')");
    }
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(bandwidth)));
    
    lua_pushvalue(L, 2); // return a copy of what was passed in
  } else if (params == 1) {
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_GET_BANDWIDTH(&bandwidth)));
    if (bandwidth == OPUS_AUTO) {
      lua_pushstring(L, OPUS_AUTO_STR);
    } else if (bandwidth == OPUS_BANDWIDTH_NARROWBAND) {
      lua_pushstring(L, OPUS_BANDWIDTH_NARROWBAND_STR);
    } else if (bandwidth == OPUS_BANDWIDTH_MEDIUMBAND) {
      lua_pushstring(L, OPUS_BANDWIDTH_MEDIUMBAND_STR);
    } else if (bandwidth == OPUS_BANDWIDTH_WIDEBAND) {
      lua_pushstring(L, OPUS_BANDWIDTH_WIDEBAND_STR);
    } else if (bandwidth == OPUS_BANDWIDTH_SUPERWIDEBAND) {
      lua_pushstring(L, OPUS_BANDWIDTH_SUPERWIDEBAND_STR);
    } else if (bandwidth == OPUS_BANDWIDTH_FULLBAND) {
      lua_pushstring(L, OPUS_BANDWIDTH_FULLBAND_STR);
    } else {
      luaL_error(L, "invalid bandwidth from opus library");
    }
  } else {
    luaL_error(L, "Got %d arguments expected only one or two (self, bandwidth)", params);
  }
    
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_enc_application(lua_State *L){
  OpusEncoder* enc = check_enc(L, 1);
  int params = lua_gettop(L);
  opus_int32 application;

  if (params == 2) {
    const char* application_str = luaL_checkstring(L,2);
    if (!strcmp(application_str, OPUS_APPLICATION_AUDIO_STR)) {
      application = OPUS_APPLICATION_AUDIO;
    } else if (!strcmp(application_str, OPUS_APPLICATION_RESTRICTED_LOWDELAY_STR)) {
      application = OPUS_APPLICATION_RESTRICTED_LOWDELAY;
    } else if (!strcmp(application_str, OPUS_APPLICATION_VOIP_STR)) {
      application = OPUS_APPLICATION_VOIP;
    } else {
      luaL_error(L, "Invalid application (must be one of 'audio','lowdelay' or 'voip')");
    }
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_SET_APPLICATION(application)));
    lua_pushvalue(L, 2); // return a copy of what was passed in
    
  } else if (params == 1) {
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_GET_APPLICATION(&application)));
    if (application == OPUS_APPLICATION_AUDIO) {
      lua_pushstring(L, OPUS_APPLICATION_AUDIO_STR);
    } else if (application == OPUS_APPLICATION_RESTRICTED_LOWDELAY) {
      lua_pushstring(L, OPUS_APPLICATION_RESTRICTED_LOWDELAY_STR);
    } else if (application == OPUS_APPLICATION_VOIP) {
      lua_pushstring(L, OPUS_APPLICATION_VOIP_STR);
    } else {
      luaL_error(L, "invalid application from opus library: %d", application);
    }
  } else {
    luaL_error(L, "Got %d arguments expected only one or two (self, application)", params);
  }
    
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_enc_signal(lua_State *L){
  OpusEncoder* enc = check_enc(L, 1);
  int params = lua_gettop(L);
  opus_int32 signal;

  if (params == 2) {
    const char* signal_str = luaL_checkstring(L,2);
    if (!strcmp(signal_str, OPUS_AUTO_STR)) {
      signal = OPUS_AUTO;
    } else if (!strcmp(signal_str, OPUS_SIGNAL_VOICE_STR)) {
      signal = OPUS_SIGNAL_VOICE;
    } else if (!strcmp(signal_str, OPUS_SIGNAL_MUSIC_STR)) {
      signal = OPUS_SIGNAL_MUSIC;
    } else {
      luaL_error(L, "Invalid signal (must be one of 'auto','voice' or 'music')");
    }
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_SET_SIGNAL(signal)));
    lua_pushvalue(L, 2); // return a copy of what was passed in
    
  } else if (params == 1) {
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_GET_SIGNAL(&signal)));
    if (signal == OPUS_AUTO) {
      lua_pushstring(L, OPUS_AUTO_STR);
    } else if (signal == OPUS_SIGNAL_VOICE) {
      lua_pushstring(L, OPUS_SIGNAL_VOICE_STR);
    } else if (signal == OPUS_SIGNAL_MUSIC) {
      lua_pushstring(L, OPUS_SIGNAL_MUSIC_STR);
    } else {
      luaL_error(L, "invalid signal from opus library: %d", signal);
    }
  } else {
    luaL_error(L, "Got %d arguments expected only one or two (self, signal)", params);
  }
    
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_enc_force_channels(lua_State *L){
  OpusEncoder* enc = check_enc(L, 1);
  int params = lua_gettop(L);
  opus_int32 force_channels;
  
  if (params == 2) {
    if (lua_isnumber(L,2)) {
      force_channels = luaL_checkint(L, 2);
      opus_check_error(L,opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS(force_channels)));
    } else if (lua_isstring(L,2)) {
      const char* force_channels_str = luaL_checkstring(L,2);
      if (!strcmp(force_channels_str, OPUS_AUTO_STR)) {
        force_channels = OPUS_AUTO;
      } else {
        luaL_error(L, "Invalid force channels (must be 1, 2 or 'auto')");
      }
      lua_pushvalue(L, 2); // return a copy of what was passed in
    }
  } else if (params == 1) {
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_GET_FORCE_CHANNELS(&force_channels)));
    if (force_channels == OPUS_AUTO) {
      lua_pushstring(L, OPUS_AUTO_STR);
    } else {
      lua_pushinteger(L, force_channels);
    }
  } else {
    luaL_error(L, "Got %d arguments expected only one or two (self, force channels (0,1 or 'auto'))", params);
  }
    
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_enc_fec(lua_State *L){
  OpusEncoder* enc = check_enc(L, 1);
  int params = lua_gettop(L);
  opus_int32 fec;
  
  if (params == 1) {
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_GET_INBAND_FEC(&fec)));
  } else if (params == 2) {
    if (!lua_isboolean(L,2)) {
      luaL_error(L,"Needs boolean flag for FEC");
    }
    
    fec = (opus_int32)lua_toboolean(L,2);
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(fec)));
  }
  
  lua_pushboolean(L, (int)fec);
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_enc_vbr(lua_State *L){
  OpusEncoder* enc = check_enc(L, 1);
  int params = lua_gettop(L);
  opus_int32 vbr;
  
  if (params == 1) {
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_GET_VBR(&vbr)));
  } else if (params == 2) {
    if (!lua_isboolean(L,2)) {
      luaL_error(L,"Needs boolean flag for VBR");
    }
    
    vbr = (opus_int32)lua_toboolean(L,2);
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_SET_VBR(vbr)));
  }
  
  lua_pushboolean(L, (int)vbr);
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_enc_vbr_constraint(lua_State *L){
  OpusEncoder* enc = check_enc(L, 1);
  int params = lua_gettop(L);
  opus_int32 vbr_constraint;
  
  if (params == 1) {
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_GET_VBR_CONSTRAINT(&vbr_constraint)));
  } else if (params == 2) {
    if (!lua_isboolean(L,2)) {
      luaL_error(L,"Needs boolean flag for VBR constraint");
    }
    
    vbr_constraint = (opus_int32)lua_toboolean(L,2);
    opus_check_error(L,opus_encoder_ctl(enc, OPUS_SET_VBR_CONSTRAINT(vbr_constraint)));
  }
  
  lua_pushboolean(L, (int)vbr_constraint);
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_enc_reset(lua_State *L){
  OpusEncoder* enc = check_enc(L, 1);
  int result = opus_encoder_ctl(enc, OPUS_RESET_STATE);

  opus_check_error(L, result);
    
  return 0;
}

void register_enc(lua_State *L)
{  
  // Register metatable for user data in registry
  luaL_newmetatable(L, "opus.encoder");
  luaL_register(L, 0, enc_gc_functions);      
  luaL_register(L, 0, enc_functions);   
  lua_pop(L,1);  
}

//------------------------------------------------------------------------------
// Decoder
//------------------------------------------------------------------------------
 
void register_dec(lua_State *L)
{  
  // Register metatable for user data in registry
  luaL_newmetatable(L, "opus.decoder");
  luaL_register(L, 0, dec_gc_functions);      
  luaL_register(L, 0, dec_functions);      
  lua_pop(L,1);
}

//-----------------------------------------------------------------------------

LUALIB_API int opus_dec_new(lua_State *L){

  int error;

  int n = lua_gettop(L);  // Number of arguments
  if (n != 2) 
      return luaL_error(L, "Got %d arguments expected 2 (samplerate, channels)", n); 

  opus_int32 Fs = luaL_checknumber (L, 1);      
  int channels = luaL_checknumber (L, 2); 

  lua_newtable(L);      // Create table to represent instance

  // Get metatable 'opus.decoder' store in the registry
  luaL_getmetatable(L, "opus.decoder");

  lua_pushvalue(L, -1); // duplicates the metatable
  lua_setmetatable(L, -3);
  
// set the metatable as __index
  lua_setfield(L, -2, "__index");

 // save away the number of channels
  lua_pushinteger(L, channels);
  lua_setfield(L, -2, "channels");

  // save away the sample rate
  lua_pushinteger(L, Fs);
  lua_setfield(L, -2, "samplerate");
   
  // Allocate memory for a pointer to to object
  OpusDecoder **s = (OpusDecoder **)lua_newuserdata(L, sizeof(OpusDecoder *));  
  luaL_getmetatable(L, "opus.decoder");
  lua_setmetatable(L, -2);


  // Set field '__self' of instance table to the encoder user data
  lua_setfield(L, -2, "__self");  
  
  *s = opus_decoder_create( Fs,  channels, &error);
  
  opus_check_error(L, error);
    
  return 1; 
}

//-----------------------------------------------------------------------------

LUALIB_API int opus_dec_decode(lua_State *L){
  opus_int16 temp_pcm[MAX_SAMPLES_PER_FRAME*MAX_CHANNELS];
  int sample_count;
  int byte_count;
  int channels;
  const unsigned char *data;
  size_t data_len;
  
  // TODO - this doesn't support FEC (framesize needs to be tailored for FEC...)
  int frame_size = MAX_SAMPLES_PER_FRAME;
  int decode_fec = 0;
  
  OpusDecoder* dec = check_dec(L, 1);
  int params = lua_gettop(L);

  if (params == 2) {

    data = (const unsigned char*)lua_tolstring (L, 2, &data_len); 
    channels = opus_packet_get_nb_channels(data);  

    sample_count = opus_check_error(L,opus_decode (dec, data, (opus_int32)data_len, temp_pcm, frame_size, decode_fec));

    byte_count = sample_count * channels * SAMPLE_SIZE; 
  } else {
    return luaL_error(L, "Got %d arguments expected 2 (self, encoded frame)", params);
  } 
  
  lua_pushlstring (L, (const char *)temp_pcm, byte_count);
  
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_dec_free(lua_State *L){
  OpusDecoder* dec = check_dec(L, 1);

  opus_decoder_destroy (dec);

  return 0;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_dec_bandwidth(lua_State *L){
  OpusDecoder* dec = check_dec(L, 1);
  int params = lua_gettop(L);
  opus_int32 bandwidth;
  const unsigned char *data;
  size_t data_len;
 
  const char* bandwidth_string;
  
  if (params == 1) {
      opus_decoder_ctl (dec,OPUS_GET_BANDWIDTH	(&bandwidth));
  } else if (params == 2) {
      data = (const unsigned char*)lua_tolstring (L, 2, &data_len); 
      bandwidth = opus_packet_get_bandwidth (data);  
  } else {
      return luaL_error(L, "Got %d arguments expected self (most recently decoded frame) or optional frame", params);
  }
  
  switch (bandwidth) {
    case OPUS_BANDWIDTH_FULLBAND:
      bandwidth_string = OPUS_BANDWIDTH_FULLBAND_STR;
      break;
    case OPUS_BANDWIDTH_MEDIUMBAND:
      bandwidth_string = OPUS_BANDWIDTH_MEDIUMBAND_STR;
      break;
    case OPUS_BANDWIDTH_NARROWBAND:
      bandwidth_string = OPUS_BANDWIDTH_NARROWBAND_STR;
      break;
    case OPUS_BANDWIDTH_SUPERWIDEBAND:
      bandwidth_string = OPUS_BANDWIDTH_SUPERWIDEBAND_STR;
      break;
    case OPUS_BANDWIDTH_WIDEBAND:
      bandwidth_string = OPUS_BANDWIDTH_WIDEBAND_STR;
      break;
    default:
      return opus_check_error(L,bandwidth);
      break;
  }
        
  lua_pushstring(L, bandwidth_string);
     
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_dec_samples_per_frame(lua_State *L){
  int params = lua_gettop(L);
  const unsigned char *data;
  size_t data_len;
  int samples_per_frame;
  
  // we had saved away the sample rate, we need it now
  lua_getfield(L, 1, "samplerate");
  opus_int32 Fs = luaL_checkint(L, -1);

  if (params == 2) {
    data = (const unsigned char*)lua_tolstring (L, 2, &data_len); 
    samples_per_frame = opus_check_error(L,opus_packet_get_samples_per_frame (data, Fs));
  } else {
    return luaL_error(L, "Got %d arguments expected 2 (self, frame data)", params);
  }
  
  lua_pushinteger(L, samples_per_frame);

  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_dec_channels(lua_State *L){
  int params = lua_gettop(L);
  const unsigned char *data;
  size_t data_len;
  int channels;
  
  if (params == 2) {
    data = (const unsigned char*)lua_tolstring (L, 2, &data_len); 
    channels = opus_check_error(L,opus_packet_get_nb_channels (data));
  } else {
    return luaL_error(L, "Got %d arguments expected 2 (self, frame data)", params);
  }
  
  lua_pushinteger(L, channels);
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_dec_frames(lua_State *L){
  int params = lua_gettop(L);
  const unsigned char *data;
  opus_int32 data_len;
  int frames;
  
  if (params == 2) {
    data = (const unsigned char *)lua_tolstring (L, 2, (size_t*)&data_len); 
    frames = opus_check_error(L,opus_packet_get_nb_frames (data, data_len));
  } else {
    return luaL_error(L, "Got %d arguments expected 2 (self, frame data)", params);
  }
  
  lua_pushinteger(L, frames);
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_dec_sample_count(lua_State *L){
  OpusDecoder* dec = check_dec(L, 1);
  int params = lua_gettop(L);
  const unsigned char *data;
  opus_int32 data_len;
  int sample_count;
  
  if (params == 2) {
    data = ( const unsigned char *)lua_tolstring (L, 2, (size_t*)&data_len); 
    sample_count = opus_check_error(L,opus_decoder_get_nb_samples (dec, data, data_len));
  } else {
    return luaL_error(L, "Got %d arguments expected 2 (self, frame data)", params);
  }
  
  lua_pushinteger(L, sample_count);

  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_dec_gain(lua_State *L){
  OpusDecoder* dec = check_dec(L, 1);
  int params = lua_gettop(L);
  opus_int32 gain;
   
  if (params == 2) {
    gain = luaL_checkint(L, 2);
    opus_check_error(L,opus_decoder_ctl(dec, OPUS_SET_GAIN(gain)));
  } else if (params == 1) {
    opus_check_error(L,opus_decoder_ctl(dec, OPUS_GET_GAIN(&gain)));
  } else {
    luaL_error(L, "Got %d arguments expected only one or two (self, gain)", params);
  }
  
  lua_pushinteger(L, gain);
  
  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_dec_pitch(lua_State *L){
  OpusDecoder* dec = check_dec(L, 1);
  int params = lua_gettop(L);
  opus_int32 pitch;

  if (params == 1) {
    opus_check_error(L,opus_decoder_ctl(dec, OPUS_GET_PITCH(&pitch)));
  } else {
    luaL_error(L, "Got %d arguments expected only one (self)", params);
  }
  
  lua_pushinteger(L, pitch);

  return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_dec_reset(lua_State *L){
  OpusDecoder* dec = check_dec(L, 1);

  int result = opus_decoder_ctl (dec, OPUS_RESET_STATE);

  opus_check_error(L, result);

  return 0;
}

//------------------------------------------------------------------------------

OpusDecoder* check_dec(lua_State* L, int index)
{
  void* ud = 0;
  luaL_checktype(L, index, LUA_TTABLE); 
  lua_getfield(L, index, "__self");
  ud = luaL_checkudata(L, index, "opus.decoder");
  luaL_argcheck(L, ud != 0, 0,"`opus.decoder' expected");  
  
  return *((OpusDecoder**)ud);      
}


//------------------------------------------------------------------------------
// Module Management
//------------------------------------------------------------------------------

LUALIB_API int opus_version(lua_State *L){

    lua_pushstring(L,  "lib opus version:" OPUS_LUA_VERSION ", authors: " OPUS_LUA_AUTHORS ", opus version: ");
    lua_pushstring(L, opus_get_version_string());
    lua_concat(L, 2);

    return 1;
}

//------------------------------------------------------------------------------

LUALIB_API int opus_help(lua_State *L){
    lua_pushstring(L, "usage:\n"

      // TODO
      "help - this message\n"
      "version - version string\n");
    return 1;
}

//------------------------------------------------------------------------------

/* init function, will be called when lua run require */
LUALIB_API int luaopen_opus (lua_State *L) {
    luaL_openlib(L, "opus", opus_functions, 0);
    
    register_dec(L);
    register_enc(L);

    return 1;
}

