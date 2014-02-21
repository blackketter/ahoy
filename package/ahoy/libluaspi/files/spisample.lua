local spi = require("spi")

spi.open("/dev/spidev1.0")

local fullduplex = spi.rw("1",1)

local halfread = spi.read(1)

local halfwrite = spi.write("1")

spi.close