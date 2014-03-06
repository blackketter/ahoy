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
    
    if fadetime then -- if fadetime is specified, then fade in over that many seconds
      commandstring = string.char(0x83, math.floor(red*255), math.floor(green*255), math.floor(blue*255), math.floor((255/fps)/fadetime))
    else 
      commandstring = string.char(0x84, math.floor(red*255), math.floor(green*255), math.floor(blue*255))
    end
    
    spi.write(commandstring)

end

--------------------------------------------------------------------------------
-- end of module
return leds