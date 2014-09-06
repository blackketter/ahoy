require "debug"

-- Audio lib inherits i2s functions
audio = require("i2s")

-- default audio settings
audio.frame_size = 768                                    
audio.sample_rate = 48000
audio.sample_size = 2
audio.frame_time = (audio.frame_size/(audio.sample_rate*audio.sample_size))
audio.frames_per_second = 1/audio.frame_time

------------------------------------------------------------------------------
function audio.init()
  audio.hack()
end

function audio.hack()
  -- hack to reset audio input
  audio.open(16, audio.sample_rate, 2, "r")
  audio.read(768)
  audio.pause()
  audio.close()
end

------------------------------------------------------------------------------
function audio.playFile(name)

  debug("playing sound file " .. name)
  
  if (string.sub(name, 1, 1) ~= "/") then
    name = "/ahoy/sounds/" .. name
  end
  
  local suffix = string.match(name, ".*%.(%a+)")
  
  if (suffix == 'wav') then
    local filehandle = assert(io.open(name, "r"))
    local wav = filehandle:read("*a");
  
    local modulo = #wav % audio.frame_size
  
    -- pad to 768 bytes
    if (modulo > 0) then
      local tacked = string.rep (string.char(0), audio.frame_size - modulo)
      wav = wav .. tacked
    end
  
    -- saved files are all 44.1 wav
    audio.open(16, 44100, 2, "w")
    audio.write(wav)
    sleep(#wav / (44100*4))
    audio.pause()
    audio.close()

  elseif (suffix == 'pcm') then
    local filehandle = assert(io.open(name, "r"))
    local pcm = filehandle:read("*a");
  
    local modulo = #pcm % audio.frame_size
  
    -- pad to 768 bytes
    if (modulo > 0) then
      local tacked = string.rep (string.char(0), audio.frame_size - modulo)
      pcm = pcm .. tacked
    end
  
    -- saved files are all 48k/1ch/pcm
    audio.open(16, 48000, 1, "w")
    audio.write(pcm)
    sleep(#pcm / (48000*2))
    audio.pause()
    audio.close()
  
  elseif (suffix == 'opus') then
    local decoded = io.popen("opusdec --quiet " .. name .. " --no-dither -")
    -- assume opus files are mono, 48k
    audio.open(16, 48000, 1, "w")
    
    local starttime = 0
    local pcmcount = 0
    
    repeat
      local reqsize = math.ceil(audio.frames_per_second * 0.5) * audio.frame_size
      
      local pcm = decoded:read(reqsize)    

      debug("requested: " .. reqsize .. " got: " .. (pcm and #pcm or 0) )

      if (pcm and (#pcm ~= reqsize)) then
        local modulo = #pcm % audio.frame_size

        -- pad to 768 bytes
        if (modulo > 0) then
          local tacked = string.rep (string.char(0), audio.frame_size - modulo)
          pcm = pcm .. tacked
        end
        debug("pcm len now: " .. #pcm)
      end      
      
      if (starttime == 0) then
        starttime = now()
      end
      
      if (pcm) then
        audio.write(pcm)
        pcmcount = pcmcount + #pcm
      end
    until not pcm
  
    sleep(pcmcount / (48000 * 2) - (now() - starttime))
    
    audio.pause()
    audio.close()
      
  end


end


------------------------------------------------------------------------------
function audio.record()
    debug("begin record")
    audio.open(16, audio.sample_rate, 1, "r")
    audio.recording = true
end

function audio.play()
    audio.open(16, audio.sample_rate, 1, "w")
    audio.playing = true
end

function audio.isPlaying()
    return audio.playing
end

function audio.isRecording()
    return audio.recording
end

function audio.stop()

  -- if we're recording, pause first so the close happens quickly
  if (audio.recording) then
    audio.pause()
  end
  
  audio.playing = false
  audio.recording = false 
  audio.close()
end

return audio