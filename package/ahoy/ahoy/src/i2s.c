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

# define I2S_FILE_KEY "i2s-file-key"

LUALIB_API int i2s_write(lua_State *L);
LUALIB_API int i2s_read(lua_State *L);

LUALIB_API int i2s_pause(lua_State *L);
LUALIB_API int i2s_resume(lua_State *L);

LUALIB_API int i2s_sampleCount(lua_State *L);

LUALIB_API int i2s_version(lua_State *L);

// natural size for the AR9331 i2s driver
#define READ_BUFFER_SIZE (NUM_DESC * I2S_BUF_SIZE)

////////////////////////////////////////////////////////////////////////////////

int getI2SFile(lua_State *L) {
  int* i2sfPtr;
  int i2sf;
  
  lua_getfield(L, LUA_REGISTRYINDEX, I2S_FILE_KEY);
  if (!lua_isuserdata (L, -1))
    luaL_error(L, "No open i2s device");
  
  i2sfPtr = lua_touserdata(L, -1);
  i2sf = *i2sfPtr;

//  fprintf(stderr, "current i2s device file descriptor: %d\n", i2sf);
  
  lua_pop(L,1);
  return i2sf;
}  

////////////////////////////////////////////////////////////////////////////////

LUALIB_API int i2s_open(lua_State *L){
    
    int sampleSize = lua_tointeger(L, 1);
    int sampleRate = lua_tointeger(L, 2);  
    const char* mode = luaL_optstring(L,3,"w");
    int modenum = O_WRONLY;
    
    int* i2sf = (int*)lua_newuserdata(L, sizeof(int));
    
//    fprintf(stderr, "opening with mode: %s\n", mode);
    
    if (strcmp(mode,"w") == 0) 
        modenum = O_WRONLY;
    else if (strcmp(mode, "r") == 0)
        modenum = O_RDONLY;
    else 
      luaL_error(L,"File mode %s is not supported in the i2s device, yet", mode);
        
      
    *i2sf = open ("/dev/i2s", modenum);

    if (*i2sf == -1) {
      luaL_error(L, "Failed to open /dev/i2s err: %s", strerror(errno));
    }
  
    lua_setfield(L, LUA_REGISTRYINDEX, I2S_FILE_KEY);

//    fprintf(stderr, "i2s file opened: %d, mode: %d\n", *i2sf, modenum);

    if (sampleSize == 0) 
      sampleSize = 16;
      
    if (sampleRate == 0)
      sampleRate = 44100;
      
    if (sampleSize < 0 || ioctl(*i2sf, I2S_DSIZE, sampleSize) < 0) {
      luaL_error(L, "Failed to set I2S_DSIZE to %d: %s", sampleSize, strerror(errno));
    }

    if (sampleRate < 0 || ioctl(*i2sf, I2S_FREQ, sampleRate) < 0) {
      luaL_error(L, "Failed to set I2S_FREQ to %d: %s", sampleRate, strerror(errno));
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

LUALIB_API int i2s_close(lua_State *L) {
    int i2sf = getI2SFile(L);
    close(i2sf);
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, I2S_FILE_KEY);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

LUALIB_API int i2s_write(lua_State *L){
	int		  writeResult=0;
	const char	  *i2sData, *startPoint;
	size_t  remainingBytes;
  size_t i2sDataLength;

  int i2sf = getI2SFile(L);
  
  i2sData = lua_tolstring(L, -1, &i2sDataLength);
  
  if (i2sData == NULL || i2sDataLength == 0) {
    luaL_error(L, "No valid i2s data for write");
  }

// keep trying to write out until we are out of data.
eagain:                                                         
  remainingBytes = i2sDataLength;                                       
  startPoint = i2sData;                                       
  writeResult = 0; 
                                                 
  do {     
      writeResult = write(i2sf, startPoint, remainingBytes);  
        
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
        luaL_error(L, "Failed to i2s_write: %s", strerror(errno));
      }                                           
      
  } while (remainingBytes);                                      

// djb: no result returned.  todo: non-blocking writes?  yield while blocked?
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

LUALIB_API int i2s_read(lua_State *L){

  ssize_t readResult;
  luaL_Buffer b;

  int i2sf = getI2SFile(L);
  
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
    readResult = read(i2sf, readBuf, requestSize);

    if (readResult < 0) {
      if (errno == EAGAIN) {
        fprintf(stderr, "EAGAIN on i2s read\n"); 
        goto eagain;
      } else {
        luaL_error(L, "I2S read failed: %s", strerror(errno));
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
LUALIB_API int i2s_pause(lua_State *L){

  int i2sf = getI2SFile(L);
  
  if (ioctl(i2sf, I2S_PAUSE, 1) < 0) {
    luaL_error(L, "Failed to pause recording i2s: %s", strerror(errno));
  }
  if (ioctl(i2sf, I2S_PAUSE, 0) < 0) {
    luaL_error(L, "Failed to pause playback i2s: %s", strerror(errno));
  }
  
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
LUALIB_API int i2s_resume(lua_State *L){

  int i2sf = getI2SFile(L);
    
  if (ioctl(i2sf, I2S_RESUME, 1) < 0) {
    luaL_error(L, "Failed to resume recording i2s: %s", strerror(errno));
  }
  if (ioctl(i2sf, I2S_RESUME, 0) < 0) {
    luaL_error(L, "Failed to resume playback i2s: %s", strerror(errno));
  }
  
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
LUALIB_API int i2s_sampleCount(lua_State *L){
    // djb: todo
    lua_pushinteger(L,0);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
LUALIB_API int i2s_version(lua_State *L){
    lua_pushstring(L, "AR9331 lua i2s version 0.1, Dean Blackketter 2013");
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
/* functions exposed to lua */
static const luaL_reg i2s_functions[] = {
  {"open", i2s_open},
  {"close",i2s_close},

  {"write", i2s_write},
  {"read", i2s_read},

  {"pause", i2s_pause},
  {"resume", i2s_resume},

  {"sampleCount", i2s_sampleCount},

  {"version", i2s_version},

  {NULL, NULL}
};

////////////////////////////////////////////////////////////////////////////////
/* init function, will be called when lua run require */
LUALIB_API int luaopen_i2s (lua_State *L) {
    luaL_openlib(L, "i2s", i2s_functions, 0);
    return 1; 
}