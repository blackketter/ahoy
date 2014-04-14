#!/usr/bin/lua
require "strict"
require "syslogger"
require "socket"
require "utils"

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

local default_color = {red=0.25,green=0.25,blue=0.25,fade=0.5}
local playback_color = {red=0,green=0,blue=1}

------------------------------------------------------------------------------
-- VOLUME
------------------------------------------------------------------------------

local lastVolumeUpdate = 0
local minVol = -40
local offVol = -120
local maxVol = 0
local volSteps = 11
local volStep = (maxVol - minVol)/volSteps

function checkVolume()
  local updated = false
  local newVol
  local oldVol
  local now = now()
  
  -- ignore if both pressed or both not pressed
  if (buttons.laststate('voldown') == buttons.laststate('volup')) then
    return
  end
  
  -- only update after a wait, releasing both resets this timeout
  if (now - lastVolumeUpdate > 0.2) then
    oldVol = codec.volume()

    if (buttons.laststate('voldown')) then
      newVol = oldVol - volStep 
  
      if (newVol < minVol) then
        newVol = offVol
      end
  
      updated = true
    end
  
    if (buttons.laststate('volup')) then
      newVol = oldVol + volStep 
    
      if (newVol > 0) then
        newVol = 0
      end
      
      if (newVol <= minVol) then
        newVol = minVol
      end
    
      updated = true
    end
  
    if (updated) then
      codec.volume(newVol)
      
      local brightness = (newVol - minVol) / (maxVol - minVol)    
      
      if (brightness < 0) then
        brightness = 0
      end
      
      leds.animate({red=brightness,green=brightness,blue=brightness,time=.1},default_color)
      if (not audio.isPlaying() and not audio.isRecording() ) then -- and oldVol ~= newVol) then
        audio.playWav("vol")
      end
      lastVolumeUpdate = now
    end  
  end 
end

------------------------------------------------------------------------------
-- BEGIN EXECUTION
------------------------------------------------------------------------------

codec.init()

spi.open("/dev/spidev1.0")

leds.init(spi)
buttons.init(spi)

-- fade to white while starting up
leds.set(1,1,1,1.0)

audio.init()
     
audio.playWav("whistle")

-- green button while we're waiting (and spinning and killing the cpu)
leds.set(default_color)

local audiodata = {}


-- main loop
for i=0,math.huge do

  -- check the volume
  checkVolume()
  
  -- show some colors to show the buttons are working 
  if (buttons.laststate("setup")) then
    leds.animate({red=0,blue=.5,green=.5,time=0.1},default_color)
  end

  -- if we're starting a recording, reset the data and turn on the red light
  if (buttons.state("main")) then  
    debug("starting")
    -- red light means recording    
    leds.set(1,0,0,0.5)

    debug("opening")
    audio.record()
    
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
    audio.stop()
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
      leds.animate({red=0,green=1,blue=1,time=.7},default_color)                                                              
      debug("whistling")
      audio.playWav("whistle")                                                                       
    end                                                                                          
    
    -- if we are still holding down the button (i.e. maximum recording), wait until it's released
    while (buttons.state("main")) do                                                             
      leds.set(default_color)                                                                            
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
  if (#audiodata > frames_per_second) then
    -- playback is blue
    leds.set(0,0,1)

    audio.play()
      
    local index = 1
  
    while (not buttons.state("main")) do
  
      checkVolume()
      
      audio.write(audiodata[index])
    
      index = index + 1
    
      -- we've reached the end
      -- start over and show a blink
      if (index > #audiodata) then 
        index = 1
        leds.animate({red=1,blue=1,green=1,time=0.1},playback_color)
      end
        
    end  
  
    audio.stop()
    audiodata = {}

    leds.set(default_color)
    
  end

end
