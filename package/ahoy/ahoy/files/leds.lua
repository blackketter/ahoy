local leds = {}
local spi

local fps = 43.43  --  approximate update rate on led

function leds.init(spiport)
  spi = spiport
end

--------------------------------------------------------------------------------
function leds.set(red, green, blue, fadetime)

    -- construct a byte sequence to set the color
    local commandstring
    
    if fadetime then 
      -- if fadetime is specified, then fade in over up to that many seconds
      commandstring = string.char(0x84, math.floor(red*255), math.floor(green*255), math.floor(blue*255), math.floor((255/fps)/fadetime))
    else 
      commandstring = string.char(0x83, math.floor(red*255), math.floor(green*255), math.floor(blue*255))
    end
    
    spi.write(commandstring)

end

function leds.animate(color1, color2) 
    local commandstring
    
    -- color command is 0x8a, then first color RGB, slew/fade increment, duration for that color,
    -- then second color RGB, slew, and duration (slew = zero means instant, second duration zero means leave at that color indefinitely
    commandstring = string.char(0x8a,
        math.floor(color1.red*255), 
        math.floor(color1.green*255), 
        math.floor(color1.blue*255), 
        color1.fade and math.floor(255/fps)/color1.fade or 0,
        math.floor(fps*color1.time),
        
        math.floor(color2.red*255), 
        math.floor(color2.green*255), 
        math.floor(color2.blue*255), 
        color2.fade and math.floor(255/fps)/color2.fade or 0,
        color2.time and math.floor(fps*color2.time) or 0
    )
    spi.write(commandstring)

end               

--------------------------------------------------------------------------------
-- end of module
return leds