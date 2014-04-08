#!/usr/bin/lua
require "strict"
require "syslogger"

local audio = require "audio"
local codec = require "codec"
local leds = require "leds"
local buttons = require "buttons"
local opus = require "opus"
local spi = require "spi"

local frame_size = 768
local frame_time = (768/(44100*4))
local frames_per_second = 1/frame_time
local max_time = 60 / frame_time
local min_time = 1 / frame_time 
local lastdelta = 0
local debug_print = false

require "socket"
------------------------------------------------------------------------------
-- FUNCTIONS
------------------------------------------------------------------------------
function sleep(sec)
    socket.select(nil, nil, sec)
end

function debug(...) 
  if (debug_print) then
    if (lastdelta == 0) then
    	lastdelta = socket.gettime()
    end
     
    print( socket.gettime()-lastdelta .. ":", ...)
  end
end
    

------------------------------------------------------------------------------
function playWav(name)

  if (string.sub(name, 1, 1) ~= "/") then
    name = "/ahoy/sounds/" .. name .. ".wav"
  end
  
  local wavfile = assert(io.open(name, "r"))
  local wav = wavfile:read("*a");
  
  local modulo = #wav % frame_size
  
  -- pad to 768 bytes
  if (modulo > 0) then
    wav = wav .. string.rep (string.char(0), frame_size - modulo)
  end
  
  audio.open(16, 44100, "w")
  audio.write(wav)
  audio.close()

end

------------------------------------------------------------------------------
-- BEGIN EXECUTION
------------------------------------------------------------------------------

-- print("Init codec")
codec.init()

spi.open("/dev/spidev1.0")

leds.init(spi)
buttons.init(spi)

-- fade to white while starting up
leds.set(1,1,1,1.0)

-- hack to reset audio input
audio.open(16, 44100, "r")
audio.read(768)
audio.close()
     
playWav("whistle")

-- green button while we're waiting (and spinning and killing the cpu)
leds.set(0,1,0,0.5)

local audiodata = {}


-- main loop
for i=0,math.huge do

  -- if we're starting a recording, reset the data and turn on the red light
  if (buttons.state("main")) then  
    debug("starting")
    -- red light means recording    
    leds.set(1,0,0,0.5)

    debug("opening")   
    audio.open(16, 44100, "r")
    
    debug("open tmp")
    local recfile = assert(io.open("/tmp/recording", "w"))
  
    debug("starting while")
    while (buttons.state("main")) do
      debug("reading audio")
      local audioframe  = audio.read(frame_size)
      audiodata[#audiodata+1] = audioframe
      if (audioframe) then
        recfile:write(audioframe)
      end
    end  
    debug("endwhile")
    audio.pause() 
    audio.close()
    debug("closing tmp")
    assert(recfile:close())
    
    -- remove the last 23 frames (.1 seconds)
    local i
    for i = 1, 23 do
      if (#audiodata > 0) then
        audiodata[#audiodata] = nil
      end
    end

    if (#audiodata < frames_per_second) then      
      debug("animating")
      leds.animate({red=0,green=1,blue=1,time=.7},{red=0,green=1,blue=0})                                                              
      debug("whistling")
      playWav("whistle")                                                                       
    end                                                                                          
    
    -- if we are still holding down the button (i.e. maximum recording), wait until it's released
    while (buttons.state("main")) do                                                             
      leds.set(0,1,0)                                                                            
    end  
  end

  -- test tone mode
  -- the test tone file is exactly .64 seconds of 44.1k audio, which ends up to be exactly 147 frames of 768 bytes.
  -- added on to a 44 byte wav header
  -- djb: need to make the audio playback accept any number of bytes
  if (buttons.laststate("setup") and buttons.laststate("volup")) then
    print("Test tone loading...")
    audiodata = {}
    local onekfile = assert(io.open("/ahoy/sounds/1k.wav","r"))
    onekfile:seek("set", 44) -- skip default wav header
    local lastread
    repeat
   	  lastread = onekfile:read(frame_size)
      audiodata[#audiodata+1] = lastread
    until (not lastread)
    onekfile:close()
  end
  
  -- if we have audio data, then play it.
  if (#audiodata > 0) then
    -- playback is blue
    leds.set(0,0,1)

    audio.open(16, 44100, "w")
  
    local index = 1
  
    while (not buttons.state("main")) do
  
      audio.write(audiodata[index])
    
      index = index + 1
    
      -- we've reached the end
      -- start over and show a blink
      if (index > #audiodata) then 
        index = 1
        leds.animate({red=1,blue=1,green=1,time=0.1},{red=0,green=0,blue=1})
      end
      
      -- show some colors to show the buttons are working 
      if (buttons.laststate("volup")) then
        leds.animate({red=.5,blue=.5,green=0,time=0.1},{red=0,green=0,blue=1})
      end
  
      if (buttons.laststate("voldown")) then
        leds.animate({red=.5,blue=0,green=.5,time=0.1},{red=0,green=0,blue=1})
      end
  
      if (buttons.laststate("setup")) then
        leds.animate({red=0,blue=.5,green=.5,time=0.1},{red=0,green=0,blue=1})
      end
  
    end  
  
    audio.close()
    audiodata = {}

    -- fade to green again
    leds.set(0,1,0,0.5)
    
  end

end
