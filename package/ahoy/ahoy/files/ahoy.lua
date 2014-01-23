#!/usr/bin/lua
require "strict"
require "syslogger"

local audio = require "audio"
local codec = require "codec"
local leds = require "leds"
local buttons = require "buttons"
local opus = require "opus"

local two_seconds = 2*44100*4

require "socket"

function sleep(sec)
    socket.select(nil, nil, sec)
end

local main_button = 12  -- on gpio 12
local setup_button = 11 -- on gpio 11

-- print("Init codec")
codec.init()
leds.init()
buttons.init(main_button,setup_button)  -- get ready to read buttons on specified GPIOs

  -- hack to reset audio input
  audio.open(16, 44100, "r")
  audio.read(768)
  audio.close()
  
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

-- flicker green button while we're waiting (and spinning and killing the cpu)
while (not buttons.state(main_button)) do
  leds.set(0,1,0)
  leds.set(0,0,0)
end

local audiodata = ''

-- main loop
for i=0,math.huge do

  -- if we're starting a recording, reset the data and turn on the red light
  if (buttons.state(main_button)) then
  
    leds.set(1,0,0)
    audiodata = ''

    audio.open(16, 44100, "r")
  
    while (buttons.state(main_button) and #audiodata < two_seconds) do
       audiodata = audiodata .. audio.read(768)
    end  

    audio.close()
  end
  
  -- if we are still holding down the button (i.e. maximum recording), wait until it's released
  while (buttons.state(main_button)) do
    leds.set(0,1,0)
  end

  -- playback is blue
  leds.set(0,0,1)

  audio.open(16, 44100, "w")
  
  while (not buttons.state(main_button)) do
    print("starting playback " .. i )
    audio.write(audiodata)
  end  
  
  audio.close()
    
end