require "utils"

local opus = require "opus"

dump(opus)

print(opus.version())
print(opus.help())

print('Creating encoder')
local enc = opus.newencoder(48000, 2, 'voip')
dump( enc)

print('Creating decoder')
local dec = opus.newdecoder(48000, 2)
dump( dec)
