#!/usr/bin/lua
require "strict"
require "syslogger"

local audio = require "audio"
local codec = require "codec"
local leds = require "leds"

require "socket"

function sleep(sec)
    socket.select(nil, nil, sec)
end

-- print("Init codec")
codec.init()
leds.init()

--audio.pause()
--audio.resume()
--audio.isPaused()
 
local hailfile = assert(io.open("/ahoy/sounds/hail.wav", "r"))
local audiodata = hailfile:read("*a");

-- white while booting
leds.set(1,1,1)
audio.open(16, 44100, "w")
audio.write(audiodata)
audio.close()

for i=0,math.huge do
  local audiodata = ''
  local remainingBytes = 768*200*2 -- 44100*2*2
  
  -- print("Recording audio ", i)
  -- red while recording
  leds.set(1,0,0)
  audio.open(16, 44100, "r")
  
  while (remainingBytes > 0) do
    local justread = audio.read(remainingBytes)
    local justreadlen = string.len(justread)
    audiodata = audiodata .. justread
    remainingBytes = remainingBytes - justreadlen
  end

  audio.close()

  -- print("Playing audio ", i)
  -- blue while playing
  leds.set(0,0,1)
  audio.open(16, 44100, "w")
  audio.write(audiodata)
  audio.close()
  
--[[  
  local audiodata = hailfile:read(remainingBytes);

  if (audiodata and #audiodata > 0) then
    print("Playing audio ", i)
    audio.write(audiodata)
  else
    hailfile:seek("set")
  end
--]]

end