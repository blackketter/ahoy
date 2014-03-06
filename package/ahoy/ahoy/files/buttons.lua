local buttons = {}
local spi
local buttonbits = { main=0, volup=1, voldown=2, setup=3 }
local null = string.char(0)

--------------------------------------------------------------------------------
function buttons.init(spiport)
  spi = spiport;
end


function buttons.state(id)

  local buttonstate = string.byte(spi.rw(null,1)) -- get the button state byte
    
  local buttonvalue = math.floor(buttonstate / 2 ^ buttonbits[id]) % 2  -- shift down by the bit number, then mod 2 to get the single bit value
  
  return buttonvalue == 1  -- result is a boolean, not a number
end
--------------------------------------------------------------------------------
-- end of module

return buttons