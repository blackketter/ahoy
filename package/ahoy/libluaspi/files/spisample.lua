#!/usr/bin/lua

require "socket"

function sleep(sec)
    socket.select(nil, nil, sec)
end
    
local spi = require "spi"

spi.open("/dev/spidev1.0")

--local fullduplex = spi.rw("1",1)

--local halfread = spi.read(1)

--local halfwrite = spi.write(string.char(value))

local value
spi.write(string.char(128+10, 
	2,2,2, 0, 1,  
	4,4,4, 0, 1))
	
while 1 do
	print(string.byte(spi.read(1)))
	sleep(1)
end
--end

spi.close()