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
      dump( v, child )
    end
  end
end

--------------------------------------------------------------------------------

function sleep(sec)
  socket.select(nil, nil, sec)
end

function now()
  local now = socket.gettime()
  return now
end

--------------------------------------------------------------------------------

local debug_print = false

function debug(...) 
  if (debug_print) then
    if (lastdelta == 0) then
    	lastdelta = socket.gettime()
    end
     
    print( socket.gettime()-lastdelta .. ":", ...)
  end
end
    
function debug_on(enable)
  debug_print = enable
end