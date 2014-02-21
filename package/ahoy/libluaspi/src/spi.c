/*

Lua Linux SPI Library

Authors:   0.01 - Dean Blackketter, Ahoy! 

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int fd;  // global file descriptor for SPI port TODO: Make this not global, i.e. allow multiple open spi ports

/* prototypes */
LUALIB_API int luaopen_spi (lua_State *L);

LUALIB_API int spi_version(lua_State *L);
LUALIB_API int spi_help(lua_State *L);
LUALIB_API int spi_open(lua_State *L);
LUALIB_API int spi_close(lua_State *L);
LUALIB_API int spi_write(lua_State *L);
LUALIB_API int spi_read(lua_State *L);
LUALIB_API int spi_rw(lua_State *L);

static int spi_do( int fd, const char* wdata, size_t wlen,
                            char* rdata, size_t rlen );

/* functions exposed to lua */
static const luaL_reg spi_functions[] = {
  {"version", spi_version},
  {"help", spi_help},
  {"open", spi_open},
  {"close", spi_close},
  {"write", spi_write},
  {"read", spi_read},
  {"rw", spi_rw},
  {NULL, NULL}
};


/* init function, will be called when lua run require */
LUALIB_API int luaopen_spi (lua_State *L) {
    luaL_openlib(L, "spi", spi_functions, 0);
    return 1;
}



LUALIB_API int spi_version(lua_State *L){
    lua_pushstring(L, "spi version 0.01, Dean Blackketter");

    return 1;
}

LUALIB_API int spi_help(lua_State *L){
  lua_pushstring(L, "usage:\n"
              "open(device)\n"
              "close()\n"
              "write(string)\n"
              "rdata = read([count(int), defaults to 1])\n"
              "rdata = rw(wdata, [readcount(int), defaults to length of string])\n"
              "version - version string\n");
  return 1;
}

LUALIB_API int spi_open(lua_State *L) {

  if (lua_gettop(L) != 1) {
      return luaL_error(L, "Missing SPI port path");
  }
  
  const char* path = luaL_checkstring(L,1);
  fd = open(path, O_RDWR);
  if (fd < 0) {
    return luaL_error(L, "SPI Open failed: %s", strerror(errno));
  }
  return 0;
}

LUALIB_API int spi_close(lua_State *L) {

  if (fd) 
    close(fd);
    
  fd = 0;  
  return 0;
}

LUALIB_API int spi_write(lua_State *L){

    size_t wlen = 0;  // strict compiler thinks wlen may be uninitialized

    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Wrong number of arguments");
    }
    
    const char* wdata = luaL_checklstring(L, 1, &wlen);

    int status = spi_do(fd, wdata, wlen, NULL, 0);
    
    if (status < 0)
      luaL_error(L,"ioctl failed, return %d", strerror(status));
    
    return 0;
}

LUALIB_API int spi_read(lua_State *L){

  size_t rlen = 1;
  
  if (lua_gettop(L) == 1) {
      rlen = luaL_checkint(L, 1);    
  } else if (lua_gettop(L) == 0) {
      rlen = 1;
  } else {
      return luaL_error(L, "Wrong number of arguments");
  }
  
  char* rdata = malloc(rlen);
  
  if (rdata == NULL)
    return luaL_error(L, "Malloc failed with data read of length %d\n", rlen);
    
  int status = spi_do(fd, NULL, 0, rdata, rlen);
  if (status < 0)
    luaL_error(L,"ioctl failed, return %d", strerror(status));

  lua_pushlstring(L,rdata, rlen);

  return 1;
   
}

LUALIB_API int spi_rw(lua_State *L){
  size_t rlen = 1;
  size_t wlen;
  char* rdata;
  const char* wdata;
  
  int params = lua_gettop(L);  

  if (params > 2 || params < 1)  {
      return luaL_error(L, "Wrong number of arguments");
  }
  
  wdata = luaL_checklstring(L, 1, &wlen);
    
  if (lua_gettop(L) == 2) {
      rlen = luaL_checkint(L, 2);    
  } else if (lua_gettop(L) == 1) {
      rlen = wlen;
  }
 
  rdata = malloc(rlen);
  
  if (rdata == NULL)
    return luaL_error(L, "Malloc failed with data read of length %d\n", rlen);
    
  int status = spi_do(fd, wdata, wlen, rdata, rlen);
  
  if (status < 0)    
    luaL_error(L,"ioctl failed, return %d", strerror(status));

  lua_pushlstring(L, rdata, rlen);

  return 1;
}

int spi_do( int fd, const char* wdata, size_t wlen,
                           char* rdata, size_t rlen ) {
      
      
	struct spi_ioc_transfer	xfer[2];
	int			status;

	memset(xfer, 0, sizeof xfer);

	xfer[0].tx_buf = (unsigned long)wdata;
	xfer[0].len = wlen;

	xfer[1].rx_buf = (unsigned long)rdata;
	xfer[1].len = rlen;

	status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
	return status;
}
