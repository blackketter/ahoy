local leds = {}
local spi

local fps = 43.43  --  approximate update rate on led

function leds.init(spiport)
  spi = spiport
end

leds.lastcolor = nil

--------------------------------------------------------------------------------

function leds.set(color1, color2) 
    local commandstring
    
    if (color2) then
      -- color command is 0x8a, then first color RGB, slew/fade increment, duration for that color,
      -- then second color RGB, slew, and duration (slew = zero means instant, second duration zero means leave at that color indefinitely
      commandstring = string.char(0x8a,
          math.floor(color1.red*255), 
          math.floor(color1.green*255), 
          math.floor(color1.blue*255), 
          color1.fade and math.floor(255/fps)/color1.fade or 0,
          color1.time and math.floor(fps*color1.time) or 0,
        
          math.floor(color2.red*255), 
          math.floor(color2.green*255), 
          math.floor(color2.blue*255), 
          color2.fade and math.floor(255/fps)/color2.fade or 0,
          color2.time and math.floor(fps*color2.time) or 0
      )
      leds.lastcolor = color2
    else 
      if color1.fade then 
        -- if fade is specified, then fade in over up to that many seconds
        commandstring = string.char(0x84, math.floor(color1.red*255), math.floor(color1.green*255), math.floor(color1.blue*255), math.floor((255/fps)/color1.fade))
      else 
        commandstring = string.char(0x83, math.floor(color1.red*255), math.floor(color1.green*255), math.floor(color1.blue*255))
      end    
      leds.lastcolor = color1
    end
    
    spi.write(commandstring)

end               

--------------------------------------------------------------------------------
-- end of module
return leds