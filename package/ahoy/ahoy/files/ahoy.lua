#!/usr/bin/lua
local audio = require "audio"
local codec = require "codec"

require "socket"

function sleep(sec)
    socket.select(nil, nil, sec)
end

print("Init codec")
codec.init()

print("Init audio")
audio.init()

print("set sample rate")
audio.sampleRate(44100)

print("set sample size")
audio.sampleSize(16)

--audio.pause()
--audio.resume()
--audio.isPaused()
 
local hailfile = assert(io.open("/ahoy/sounds/hail.wav", "r"))

for i=0,math.huge do
  local remainingBytes = 768*200 -- 44100*2*2
  
  local onesecond = hailfile:read(remainingBytes);
  
--  print("Recording audio ", i)
  
  while (false) do -- while (remainingBytes >= 0) do
    print("remainingBytes: ", remainingBytes)
    local justread = audio.read(remainingBytes)
    local justreadlen = string.len(justread)
    print("justreadlen: ", justreadlen)
    onesecond = onesecond .. justread
    print("now have ", string.len(onesecond), " bytes")
    remainingBytes = remainingBytes - justreadlen
    print()
  end
  
  if (onesecond and #onesecond > 0) then
    print("Playing audio ", i)
    audio.write(onesecond)
  else
    hailfile:seek("set")
  end

end


