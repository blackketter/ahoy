#!/usr/bin/lua
require "strict"
require "syslogger"
require "socket"
require "utils"
require "debug"

local audio = require "audio"
local codec = require "codec"
local leds = require "leds"
local buttons = require "buttons"
local opus = require "opus"
local spi = require "spi"

debug_on(true)

-- opus frames are 20ms, which is 960 samples at mono 48k sampling rate, which is 1920 bytes. or:
-- 48000*0.02*2=1920 bytes

-- luckily,the audio hardware frame size is 768 bytes of stereo 16-bit audio, which, at 48000 is:
-- 1/(48000*2*2/768) = 0.004sec = 4ms

-- so each opus frame is conveniently 5 hardware frames (which are converted from stereo to mono)

local opus_sample_rate = 48000
local opus_frames_per_second = 50  -- 20mS
local opus_samples_per_frame = opus_sample_rate / opus_frames_per_second
local opus_frame_size = opus_samples_per_frame * 2 -- two bytes per sample

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
      
      leds.set({red=brightness,green=brightness,blue=brightness,time=.1},leds.lastcolor)
      if (not audio.isPlaying() and not audio.isRecording() ) then -- and oldVol ~= newVol) then
        audio.playFile("vol.wav")
      end
      lastVolumeUpdate = now
    end  
  end 
end

------------------------------------------------------------------------------
-- WHISTLE
------------------------------------------------------------------------------
function whistle()
  debug("whistling")
  leds.set({red=0,green=1,blue=1,time=.7},default_color)                                                              
  audio.playFile("whistle.opus")                                                                       
end

------------------------------------------------------------------------------
-- SETUP
------------------------------------------------------------------------------
function checkSetup()
  if (buttons.laststate("setup")) then
    debug("Setup on")
    
    leds.set({red=1,blue=0,green=0,time=0.5,},{red=0,blue=0,green=0,time=1,fade=1})
    audio.playFile("setup-on.opus")
       
    while (buttons.state("setup")) do
    end
    
    while (not buttons.state("setup")) do
      checkVolume()
    end
    
    debug("Setup off")
    leds.set(default_color)
    audio.playFile("setup-off.opus")
    
  end
end


------------------------------------------------------------------------------
-- RECORDING
------------------------------------------------------------------------------
function checkRecording()
  local opusdata = {}
  
  if (buttons.state("main")) then  
    debug("starting recording")
    -- red light means recording    
    leds.set({red=1,green=0,blue=0,fade=0.5})

    local enc = opus.newencoder(48000, 1, 'voip')

    audio.record()
    
    local recfile = assert(io.open("/tmp/recording", "w"))
  
    debug("reading audio frames of size:" .. opus_frame_size)
    while (buttons.state("main")) do
      local pcmframe  = audio.read(opus_frame_size)
      local opusframe = enc:encode(pcmframe)
      debug('opusframe length ' .. #opusframe)

      if (pcmframe) then
        recfile:write(pcmframe)
      end
      
      opusdata[#opusdata+1] = opusframe
      
    end  
    debug("opus frames: " .. #opusdata)
    audio.stop()

    assert(recfile:close())
    
    -- remove the last 0.1 second to hide the click from the button
    local i
    for i = 1, opus_frames_per_second/10 do
      if (#opusdata > 0) then
        opusdata[#opusdata] = nil
      end
    end

    -- whistle if the recording is less than one second long
    if (#opusdata < opus_frames_per_second * 1) then      
      whistle()
      -- throw out the short data
      opusdata = {}
    end                                                                                          
    
    -- if we are still holding down the button (i.e. maximum recording), wait until it's released
    while (buttons.state("main")) do                                                             
      leds.set(default_color)                                                                            
    end  
  end
  return opusdata
end

------------------------------------------------------------------------------
-- TEST TONE
------------------------------------------------------------------------------
function checkTestTone() 

  local audiodata = {}  
  -- test tone mode
  -- the test tone file is exactly .64 seconds of 44.1k audio, which ends up to be exactly 147 frames of 768 bytes.
  -- added on to a 44 byte wav header
  -- djb: need to make the audio playback accept any number of bytes
  if (buttons.state("setup") and buttons.laststate("volup")) then
    debug("Test tone loading...")
    audiodata = {}
    local onekfile = assert(io.open("/ahoy/sounds/1k.wav","r"))
    onekfile:seek("set", 44) -- skip default wav header
    local lastread
    repeat
   	  lastread = onekfile:read(audio.frame_size)
      audiodata[#audiodata+1] = lastread
    until (not lastread)
    onekfile:close()
  end
  
  return audiodata
  
end

------------------------------------------------------------------------------
-- BEGIN EXECUTION
------------------------------------------------------------------------------

codec.init()

spi.open("/dev/spidev1.0")

leds.init(spi)
buttons.init(spi)

-- fade to white while starting up
leds.set({red=1,green=1,blue=1,fade=1.0})

audio.init()

-- give a little whistle at startup     
whistle()

-- main loop
for i=0,math.huge do

  -- check the volume
  checkVolume()
  
  -- check to see if we are entering setup mode
  checkSetup() 

  -- if we're starting a recording, reset the data and turn on the red light
  local opusdata = checkRecording()
  
  -- if we have audio data, then play it.
  if (#opusdata > 0) then
    -- playback is blue
    leds.set(playback_color)

    audio.play()
      
    local index = 1
--    local dec = opus.newdecoder(48000, 1)

    while (not buttons.state("main")) do
  
      checkVolume()
      local decodedframe = dec:decode(opusdata[index])
      debug("decoded frame of length " .. #decodedframe)
      audio.write(decodedframe)
--      audio.write(opusdata[index])
      index = index + 1
    
      -- we've reached the end
      -- start over and show a blink
      if (index > #opusdata) then 
        index = 1
        leds.set({red=1,blue=1,green=1,time=0.1},playback_color)
        debug("repeat sound")
      end
        
    end  
  
    audio.stop()
    debug("stop playing")
    opusdata = {}
    
    leds.set(default_color)
    
  end

end
