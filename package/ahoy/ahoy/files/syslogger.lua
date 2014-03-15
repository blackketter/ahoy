require "nixio"
-- set the ident string to the filename that first included this module
nixio.openlog(debug.getinfo(3, "S").short_src, "odelay")

local _error = error
error =  function(txt, level)
	if level then level = level + 1 
	else level = 2
	end
	local info = debug.getinfo(level, "Sl")
	local msg
	if info.what == "C" then
		msg = "[C function] "
	else
		msg = string.format("[%s]:%d ", info.short_src, info.currentline)
	end
	nixio.syslog("err", msg .. (txt or ""))
	_error(txt, level)
end

_G.assert = function (...)
	local cond, txt = ...
	if not cond then
		error(txt or "assertion failed!", 2)
	end
	return ...
end
