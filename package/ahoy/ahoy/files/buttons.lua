local buttons = {}

--------------------------------------------------------------------------------
function buttons.init(...)
  buttons.fds = {}
  for i,v in ipairs{...} do
    local tempfd = assert(io.open("/sys/class/gpio/export", "w"))
    tempfd:write(v)
    tempfd:close()
    
    tempfd = assert(io.open("/sys/class/gpio/gpio" .. v .. "/direction", "w"))
    tempfd:write("in")
    tempfd:close()
   	
    tempfd = assert(io.open("/sys/class/gpio/gpio" .. v .. "/active_low", "w"))
    tempfd:write("1")
    tempfd:close()
         
    buttons.fds[v] = assert(io.open("/sys/class/gpio/gpio" .. v .. "/value", "r"))
  end
end

function buttons.state(id)
  local state
  buttons.fds[id]:seek("set")
  state = assert(buttons.fds[id]:read(1))
--  print("Button " .. id .. " is " .. state)
  return tonumber(state) == 1
end
--------------------------------------------------------------------------------
-- end of module

return buttons