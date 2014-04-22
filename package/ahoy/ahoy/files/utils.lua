require "socket"
--------------------------------------------------------------------------------
-- Useful Utility Functions
--------------------------------------------------------------------------------

function dump( var, name )
  if not name then name = "anonymous" end
  if "table" ~= type( var ) then
    print( name .. " = " .. tostring( var ) )
  else
    -- for tables, recurse through children
    for k,v in pairs( var ) do
      local child
      if 1 == string.find( k, "%a[%w_]*" ) then
        -- key can be accessed using dot syntax
        child = name .. '.' .. k
      else
        -- key contains special characters
        child = name .. '["' .. k .. '"]'
      end
      if (k ~= "__index") then
        dump( v, child )
      end
    end
  end
end

--------------------------------------------------------------------------------

function sleep(sec)
  debug("sleeping: " .. sec )
  if (sec > 0) then
    socket.select(nil, nil, sec)
  end
end

function now()
  local now = socket.gettime()
  return now
end

--------------------------------------------------------------------------------

debug_print = false
debug_file = "/tmp/debug.log"
debug_fileh = io.open(debug_file, "w")

local lastdelta = socket.gettime()

function debug(...) 
  if (debug_print or debug_fileh) then
    if (lastdelta == 0) then
    	lastdelta = socket.gettime()
    end
    
    local prefix = string.format("%10.4f", socket.gettime()-lastdelta) .. ":"
    
    if (debug_print) then
      print( prefix, ...)
    end
    
    if (debug_fileh) then
      debug_fileh:write(prefix .. "\t", ..., "\n")
      debug_fileh:flush()
    end
    
  end
end
    
function debug_on(enable)
  debug_print = enable
end
