/*

Lua Opus Library

License: CC BY-SA (http://creativecommons.org/licenses/by-sa/2.5/)
This license lets others remix, tweak, and build upon a work even for commercial reasons, as long as they credit the original author and license their new creations under the identical terms.

Authors:        0.1 - Dean Blackketter 

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "opus/opus.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

LUALIB_API int opus_version(lua_State *L);
LUALIB_API int opus_help(lua_State *L);
LUALIB_API int luaopen_opus (lua_State *L);


LUALIB_API int opus_version(lua_State *L){
    lua_pushstring(L, "opus version 0.1, Dean Blackketter");

    return 1;
}

LUALIB_API int opus_help(lua_State *L){
    lua_pushstring(L, "usage:\n"


      "version - version string\n");
    return 1;
}

/* functions exposed to lua */
static const luaL_reg opus_functions[] = {
  {"version", opus_version},
  {"help", opus_help},
  {NULL, NULL}
};



/* init function, will be called when lua run require */
LUALIB_API int luaopen_opus (lua_State *L) {
    luaL_openlib(L, "opus", opus_functions, 0);
    return 1;
}

--------------------------------------------------------------------------------
-- Encoder
--------------------------------------------------------------------------------

OpusEncoder * 	opus_encoder_create (opus_int32 Fs, int channels, int application, int *error)
 	Allocates and initializes an encoder state. More...
 
int 	opus_encoder_init (OpusEncoder *st, opus_int32 Fs, int channels, int application)
 	Initializes a previously allocated encoder state The memory pointed to by st must be at least the size returned by opus_encoder_get_size(). More...
 
opus_int32 	opus_encode (OpusEncoder *st, const opus_int16 *pcm, int frame_size, unsigned char *data, opus_int32 max_data_bytes)
 	Encodes an Opus frame. More...
 
void 	opus_encoder_destroy (OpusEncoder *st)
 	Frees an OpusEncoder allocated by opus_encoder_create(). More...
 
int 	opus_encoder_ctl (OpusEncoder *st, int request,...)
 	opus_encoder_ctl(enc, OPUS_SET_BITRATE(bitrate));
  opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(complexity));

--------------------------------------------------------------------------------
-- Encoder
--------------------------------------------------------------------------------
 	
OpusDecoder * 	opus_decoder_create (opus_int32 Fs, int channels, int *error)
 	Allocates and initializes a decoder state. More...
 
int 	opus_decoder_init (OpusDecoder *st, opus_int32 Fs, int channels)
 	Initializes a previously allocated decoder state. More...
 
int 	opus_decode (OpusDecoder *st, const unsigned char *data, opus_int32 len, opus_int16 *pcm, int frame_size, int decode_fec)
 	Decode an Opus packet. More...
 
int 	opus_decoder_ctl (OpusDecoder *st, int request,...)
 	Perform a CTL function on an Opus decoder. More...
 
void 	opus_decoder_destroy (OpusDecoder *st)
 	Frees an OpusDecoder allocated by opus_decoder_create(). More...

int 	opus_packet_get_bandwidth (const unsigned char *data)
 	Gets the bandwidth of an Opus packet. More...
 
int 	opus_packet_get_samples_per_frame (const unsigned char *data, opus_int32 Fs)
 	Gets the number of samples per frame from an Opus packet. More...
 
int 	opus_packet_get_nb_channels (const unsigned char *data)
 	Gets the number of channels from an Opus packet. More...
 
int 	opus_packet_get_nb_frames (const unsigned char packet[], opus_int32 len)
 	Gets the number of frames in an Opus packet. More...
 
int 	opus_packet_get_nb_samples (const unsigned char packet[], opus_int32 len, opus_int32 Fs)
 	Gets the number of samples of an Opus packet. More...
 
int 	opus_decoder_get_nb_samples (const OpusDecoder *dec, const unsigned char packet[], opus_int32 len)
 	Gets the number of samples of an Opus packet. More...
 