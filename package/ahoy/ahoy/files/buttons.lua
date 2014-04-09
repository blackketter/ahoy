local buttons = {}
local spi
local buttonbits = { main=0, volup=1, voldown=2, setup=3 }
local null = string.char(0)
local laststate

--------------------------------------------------------------------------------
function buttons.init(spiport)
  spi = spiport;
end

function buttons.laststate(id)
  if (not laststate) then
    return buttons.state(id)
  end
  
  local buttonvalue = math.floor(laststate / 2 ^ buttonbits[id]) % 2  -- shift down by the bit number, then mod 2 to get the single bit value
  
  return buttonvalue == 1  -- result is a boolean, not a number
end
 
function buttons.state(id)

  laststate = string.byte(spi.rw(null,1)) -- get the button state byte
  
  return buttons.laststate(id)
end

function buttons.down(id)
  print("fix buttons.down!")
end
--------------------------------------------------------------------------------
-- end of module

return buttons