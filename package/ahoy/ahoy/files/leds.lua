local leds = {}
 
function leds.init()
end

--------------------------------------------------------------------------------
function leds.set(red, green, blue)
    local r = 0
    local g = 0
    local b = 0
    local w = 0
    leds.whitef = assert(io.open("/sys/devices/platform/leds-gpio/leds/ahoy:white:white/brightness", "a+"))
    leds.redf = assert(io.open("/sys/devices/platform/leds-gpio/leds/ahoy:red:red/brightness", "a+"))
    leds.greenf = assert(io.open("/sys/devices/platform/leds-gpio/leds/ahoy:green:green/brightness", "a+"))
    leds.bluef = assert(io.open("/sys/devices/platform/leds-gpio/leds/ahoy:blue:blue/brightness", "a+"))
    
    if (red > 0) and (green > 0) and (blue > 0) then
      -- if all three colors have energy turn on white
      w = 1          
    else 
      
      -- if there are components of the individual colors, turn them each on
      if (red > 0) then
        r = 1
      end
      
      if (green > 0) then
        g = 1
      end
      
      if (blue > 0) then 
        b = 1
      end
      
    end    
    
    assert(leds.redf:write(tostring(r)))
    assert(leds.greenf:write(tostring(g)))
    assert(leds.bluef:write(tostring(b)))
    assert(leds.whitef:write(tostring(w)))

    leds.redf:close()
    leds.greenf:close()
    leds.bluef:close()
    leds.whitef:close()

end

--------------------------------------------------------------------------------
-- end of module
return leds