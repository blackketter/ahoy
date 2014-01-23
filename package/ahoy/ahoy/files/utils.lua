-- Useful Utility Functions

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
