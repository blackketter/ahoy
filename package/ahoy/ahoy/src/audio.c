#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "i2sio.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

# define AUDIO_FILE_KEY "audio-file-key"

LUALIB_API int audio_write(lua_State *L);
LUALIB_API int audio_read(lua_State *L);

LUALIB_API int audio_pause(lua_State *L);
LUALIB_API int audio_resume(lua_State *L);

LUALIB_API int audio_sampleCount(lua_State *L);

LUALIB_API int audio_version(lua_State *L);

// natural size for the AR9331 i2s driver
#define READ_BUFFER_SIZE (NUM_DESC * I2S_BUF_SIZE)

////////////////////////////////////////////////////////////////////////////////

int getAudioFile(lua_State *L) {
  int* audiofPtr;
  int audiof;
  
  lua_getfield(L, LUA_REGISTRYINDEX, AUDIO_FILE_KEY);
  if (!lua_isuserdata (L, -1))
    luaL_error(L, "No open audio device");
  
  audiofPtr = lua_touserdata(L, -1);
  audiof = *audiofPtr;

//  fprintf(stderr, "current audio device file descriptor: %d\n", audiof);
  
  lua_pop(L,1);
  return audiof;
}  

////////////////////////////////////////////////////////////////////////////////

LUALIB_API int audio_open(lua_State *L){
    
    int sampleSize = lua_tointeger(L, 1);
    int sampleRate = lua_tointeger(L, 2);  
    const char* mode = luaL_optstring(L,3,"w");
    int modenum = O_WRONLY;
    
    int* audiof = (int*)lua_newuserdata(L, sizeof(int));
    
//    fprintf(stderr, "opening with mode: %s\n", mode);
    
    if (strcmp(mode,"w") == 0) 
        modenum = O_WRONLY;
    else if (strcmp(mode, "r") == 0)
        modenum = O_RDONLY;
    else 
      luaL_error(L,"File mode %s is not supported in the audio device, yet", mode);
        
      
    *audiof = open ("/dev/i2s", modenum);

    if (*audiof == -1) {
      luaL_error(L, "Failed to open /dev/i2s err: %s", strerror(errno));
    }
  
    lua_setfield(L, LUA_REGISTRYINDEX, AUDIO_FILE_KEY);

//    fprintf(stderr, "audio file opened: %d, mode: %d\n", *audiof, modenum);

    if (sampleSize == 0) 
      sampleSize = 16;
      
    if (sampleRate == 0)
      sampleRate = 44100;
      
    if (sampleSize < 0 || ioctl(*audiof, I2S_DSIZE, sampleSize) < 0) {
      luaL_error(L, "Failed to set I2S_DSIZE to %d: %s", sampleSize, strerror(errno));
    }

    if (sampleRate < 0 || ioctl(*audiof, I2S_FREQ, sampleRate) < 0) {
      luaL_error(L, "Failed to set I2S_FREQ to %d: %s", sampleRate, strerror(errno));
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

LUALIB_API int audio_close(lua_State *L) {
    int audiof = getAudioFile(L);
    close(audiof);
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, AUDIO_FILE_KEY);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

LUALIB_API int audio_write(lua_State *L){
	int		  writeResult=0;
	const char	  *audioData, *startPoint;
	size_t  remainingBytes;
  size_t audioDataLength;

  int audiof = getAudioFile(L);
  
  audioData = lua_tolstring(L, -1, &audioDataLength);
  
  if (audioData == NULL || audioDataLength == 0) {
    luaL_error(L, "No valid audio data for write");
  }

// keep trying to write out until we are out of data.
eagain:                                                         
  remainingBytes = audioDataLength;                                       
  startPoint = audioData;                                       
  writeResult = 0; 
                                                 
  do {     
      writeResult = write(audiof, startPoint, remainingBytes);  
        
//      fprintf(stderr, "Wrote %d bytes of %d remaining\n", writeResult, remainingBytes);                   
      if (writeResult < 0 && errno == EAGAIN) {                       
          fprintf(stderr, "Goto eagain");
          goto eagain;                                        
      }                                      
                       
      if (writeResult >= 0) {
        remainingBytes = remainingBytes - writeResult;                              
        startPoint += writeResult;
      } else {
//        fprintf(stderr, "write failed %s\n",strerror(errno));
        // djb - This error doesn't get displayed when killing process with control-c/sigint, rather we get a segfault
        luaL_error(L, "Failed to audio_write: %s", strerror(errno));
      }                                           
      
  } while (remainingBytes);                                      

// djb: no result returned.  todo: non-blocking writes?  yield while blocked?
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

LUALIB_API int audio_read(lua_State *L){

  ssize_t readResult;
  luaL_Buffer b;

  int audiof = getAudioFile(L);
  
  size_t requestSize = lua_tonumber(L, -1);

  if (requestSize <= 0) {
    requestSize = READ_BUFFER_SIZE;
  }
  
  // sometimes we read more than we ask for!
  char* readBuf = malloc(requestSize + READ_BUFFER_SIZE);

  if (!readBuf) {
    luaL_error(L, "Failed to allocate read buffer size %d", requestSize + READ_BUFFER_SIZE);
  }
  
  luaL_buffinit(L, &b);
  do {  
eagain:
    readResult = read(audiof, readBuf, requestSize);

    if (readResult < 0) {
      if (errno == EAGAIN) {
        fprintf(stderr, "EAGAIN on audio read\n"); 
        goto eagain;
      } else {
        luaL_error(L, "Audio read failed: %s", strerror(errno));
      }
    }

    if (readResult > 0) {
      luaL_addlstring (&b, readBuf, readResult);
      requestSize -= readResult;
    }
    
    if (readResult == 0) {
    
    }
    
  } while (requestSize > 0);
  
  luaL_pushresult(&b);

  free(readBuf);    

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
LUALIB_API int audio_pause(lua_State *L){

  int audiof = getAudioFile(L);
  
  if (ioctl(audiof, I2S_PAUSE, 1) < 0) {
    luaL_error(L, "Failed to pause recording audio: %s", strerror(errno));
  }
  if (ioctl(audiof, I2S_PAUSE, 0) < 0) {
    luaL_error(L, "Failed to pause playback audio: %s", strerror(errno));
  }
  
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
LUALIB_API int audio_resume(lua_State *L){

  int audiof = getAudioFile(L);
    
  if (ioctl(audiof, I2S_RESUME, 1) < 0) {
    luaL_error(L, "Failed to resume recording audio: %s", strerror(errno));
  }
  if (ioctl(audiof, I2S_RESUME, 0) < 0) {
    luaL_error(L, "Failed to resume playback audio: %s", strerror(errno));
  }
  
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
LUALIB_API int audio_sampleCount(lua_State *L){
    // djb: todo
    lua_pushinteger(L,0);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
LUALIB_API int audio_version(lua_State *L){
    lua_pushstring(L, "AR9331 lua audio version 0.1, Dean Blackketter 2013");
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
/* functions exposed to lua */
static const luaL_reg audio_functions[] = {
  {"open", audio_open},
  {"close",audio_close},

  {"write", audio_write},
  {"read", audio_read},

  {"pause", audio_pause},
  {"resume", audio_resume},

  {"sampleCount", audio_sampleCount},

  {"version", audio_version},

  {NULL, NULL}
};

////////////////////////////////////////////////////////////////////////////////
/* init function, will be called when lua run require */
LUALIB_API int luaopen_audio (lua_State *L) {
    luaL_openlib(L, "audio", audio_functions, 0);
    return 1; 
}