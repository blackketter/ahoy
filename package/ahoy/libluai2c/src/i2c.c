#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "linux/i2c.h"
#include "linux/i2c-dev.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/* return values from I2Clib_rw */
enum{
    RW_ACK = 0,
    RW_NACK = 1,
    RW_ERROR_SEND = 2,
    RW_ERROR_BUS = 3,
    RW_ERROR_PARAM = 4
};

/* prototypes */
static int i2c_rw( int bus, int addr, const char *wptr, int wlen, char *rptr, int rlen);
LUALIB_API int i2c_write(lua_State *L);
LUALIB_API int i2c_read(lua_State *L);
LUALIB_API int i2c_version(lua_State *L);


LUALIB_API int i2c_version(lua_State *L){
    lua_pushstring(L, "i2c version 0.9, Mikael Sundin 2013");
    
    return 1;
}

LUALIB_API int i2c_help(lua_State *L){
    lua_pushstring(L, "usage:\n"
    
    				  "status = write(bus(int), device(int), register(int), data(int))...\n"
    				  "status, data... = read(bus(int), device(int), register(int), [count(int)])\n"
    				  "		(optional count for number of consecutive registers to read)\n"    				  
                      "   status ok: ack=0\n"
                      "   status error: nack=1, send error=2, bus error=3, parameter error=4\n"
    
                      "version - version string\n");
    return 1;
}


/* write and/or read i2c bus 
* @param bus, I2c bus to work on
* @param addr, i2c device address
* @param wptr, pointer to data to write to
* @param wlen, write length
* @param rptr, pointer to store read data to
* @param rlen, number of bytes to read
* @return: 0=ack, 1=nack, 2=bus error, 3=parameter error
*/
int i2c_rw(  int bus,
                int addr, 
                const char *wptr, int wlen, 
                char *rptr, int rlen){
    int f;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    char dev[15];

    if(wlen == 0 && rlen == 0){
        return RW_ERROR_PARAM;
    }
    
    sprintf(&dev[0], "/dev/i2c-%d", bus);
    
    messages[0].addr  = addr;
    messages[0].flags = 0;
    messages[0].len   = wlen;
    messages[0].buf   = (char *)wptr;
    
    messages[1].addr  = addr;
    messages[1].flags = I2C_M_RD;
    messages[1].len   = rlen;
    messages[1].buf   = rptr;
    
    if(wlen > 0 && rlen > 0){
        packets.msgs = &messages[0];    //write & read
        packets.nmsgs = 2;
    }else if(wlen > 0){         
        packets.msgs = &messages[0];    //only write    
        packets.nmsgs = 1;
    }else if(rlen > 0){         
        packets.msgs = &messages[1];    //only read
        packets.nmsgs = 1;
    }
    
    if ((f = open(dev, O_RDWR)) < 0) {
        return RW_ERROR_BUS;
    }

    if(ioctl(f, I2C_RDWR, &packets) < 0) {
        close(f);
        return RW_ERROR_SEND;
    }

    close(f);
    
    /* check for error from i2c transfer */
    if( (messages[0].flags & I2C_M_NO_RD_ACK) || 
        (messages[1].flags & I2C_M_NO_RD_ACK)){
        return RW_NACK;    
    }else{
        return RW_ACK;
    }
}

/* LUA parameters:
 * int i2c bus number
 * int address
 * int starting register
 * int data bytes...
 *
 * Return values:
 * status
 * Number of bytes written
 */
LUALIB_API int i2c_write(lua_State *L){
    int args;
    int bus;
    int address;
    int reg;
    
    int wlen;
    unsigned char *wptr = 0;

    if( lua_gettop(L) < 4){
        return luaL_error(L, "Wrong number of arguments");
    }
    bus = lua_tointeger (L, 1);
    address = lua_tointeger(L, 2);
	reg = lua_tointeger(L, 3);
    
    if (lua_isnumber(L, 4)) {
		/* buffer length is number of bytes pushed after bus, address and reg plus one byte for the reg address */
		wlen = lua_gettop(L) - 3 + 1; 
	
		wptr = malloc( wlen );
		if (!wptr) {
			return luaL_error(L, "Malloc failed for i2c write buffer");
		}

		/* first byte is the register address */
		wptr[0] = reg;
	
		/* subsequent bytes are pulled from the stack */
		for (int i = 1; i < wlen; i++) {
			wptr[i] = lua_tointeger(L, i + 3);
		}    
    } else if (lua_istable(L, 4)) {
    	// djb - todo: accept an array of values. (already accept a series of parameters above), until then use unpack
    }
    
    //write back i2c write status
    lua_pushnumber(L, i2c_rw(bus, address, wptr, wlen, 0, 0));
    if (wptr) free(wptr);
    return 1;
}

/* LUA 
 * Parameters:
 * int bus
 * int address
 * int reg
 * int count
 *
 * Return:
 * Status
 * data...
 */
LUALIB_API int i2c_read(lua_State *L){
    int bus;
    int address;
    int reg;
    int count=1;

    const unsigned char *wptr=0;

    if( lua_gettop(L) < 3 ){
        return luaL_error(L, "Wrong number of arguments");
    }
    
    //parse input
    bus = lua_tointeger (L, 1);
    address = lua_tointeger(L, 2);
    reg = lua_tointeger(L, 3);
    count = (lua_gettop(L) == 4) ? lua_tointeger(L, 4) : 1;
    
    /* transfer data */
    {
        unsigned char rdata[count];
		unsigned char regbyte = reg;    
		
		/* do a dummy write of a single byte to set the register position that we want to read */
		/* followed by the actual read of requested bytes */ 
		int fail = i2c_rw(bus, address, &regbyte, sizeof(regbyte), &rdata[0], count);

		if (fail != RW_ACK) 
			return luaL_error(L, "i2c_rw failed with %d", fail);
		
		lua_newtable(L);
		
		for (int i = 0; i < count; i++)  {
			lua_pushnumber(L, i+1); // lua is one based
        	lua_pushnumber(L, rdata[i]); 
        	lua_settable(L, -3);
        }
        
        return 1;
    }
    
}


/* functions exposed to lua */
static const luaL_reg i2c_functions[] = {
  {"write", i2c_write},
  {"read", i2c_read},
  {"version", i2c_version},
  {"help", i2c_help},
  {NULL, NULL}
};


/* init function, will be called when lua run require */
LUALIB_API int luaopen_i2c (lua_State *L) {
    luaL_openlib(L, "i2c", i2c_functions, 0);
    return 1; 
}