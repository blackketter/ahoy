-- Audio lib inherits i2s functions
audio = require("i2s")

local frame_size = 768                                    

------------------------------------------------------------------------------
function audio.init()
  -- hack to reset audio input
  audio.open(16, 44100, "r")
  audio.read(768)
  audio.close()
end

------------------------------------------------------------------------------
function audio.playWav(name)

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
  sleep(#wav / (44100*4))
  audio.pause()
  audio.close()

end

------------------------------------------------------------------------------
function audio.record()
    audio.open(16, 44100, "r")
    audio.recording = true
end

function audio.play()
    audio.open(16, 44100, "w")
    audio.playing = true
end

function audio.isPlaying()
    return audio.playing
end

function audio.isRecording()
    return audio.recording
end

function audio.stop()
  if (audio.recording) then
    audio.pause()
  end
  
  audio.playing = false
  audio.recording = false 
  audio.close()
end

return audio