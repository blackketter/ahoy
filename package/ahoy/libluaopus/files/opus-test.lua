#!/usr/bin/lua

require "utils"

local opus = require "opus"

dump(opus, 'opus')

print(opus.version())
print(opus.help())
print()
print('Creating encoder')
local enc = opus.newencoder(48000, 2, 'voip')
dump( enc,"enc")
dump( getmetatable(enc), "enc(metatable)");
print()

local sampledata = string.rep('1234',480)

print('sampledata length', #sampledata)

local sampleframe = enc:encode(sampledata)

print('sampleframe = length', #sampleframe)

dump( enc:bitrate(), 'bitrate' )

dump( enc:complexity(), 'complexity' )

dump( enc:application(), 'application' )

dump( enc:signal(),'signal' )

dump( enc:bandwidth(), 'bandwidth' )

dump( enc:force_channels(), 'force_channels' )

dump( enc:fec(), 'fec' )

dump( enc:signal(), 'signal' )

dump( enc:vbr(), 'vbr' )

dump( enc:vbr_constraint(), 'vbr_constraint' )

dump( enc:reset(), 'reset' )

print()
print('Creating decoder')

local dec = opus.newdecoder(48000, 2)
dump( dec,"dec")
dump( getmetatable(dec), 'dec(metatable)')

print()

local decodeddata = dec:decode(sampleframe)

print('decode returned data of length', #decodeddata )

dump( dec:bandwidth(), 'bandwidth' )

dump( dec:samples_per_frame(sampleframe), 'samples_per_frame' )

dump( dec:channels(), 'channels' )

dump( dec:channels(sampleframe), 'channels in sample frame')

dump( dec:frames(sampleframe), 'frames' )

dump( dec:samples(sampleframe), 'samples')

dump( dec:gain(), 'gain' )

dump( dec:pitch(), 'pitch' )

dump( dec:reset(), 'reset' )

print('opus-test done')