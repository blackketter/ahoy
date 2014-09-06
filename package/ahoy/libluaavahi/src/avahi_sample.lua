#!/usr/bin/lua
--require("utils")

print("requiring")
local avahi = require("avahi")
--dump(avahi)

print(avahi.help())

print(avahi.version())

print("opening")
avahi.open()

print("starting browse")
avahi.browse("_ahoy._tcp")

print("polling")
local pollresult = avahi.poll(0)  -- pass in the sleep time, defaults to zero, returns -1 on failure, nil on success and 1 if a quit is pending
dump(pollresult)

print("finding service")
local foundservices = avahi.found() -- returns a table containing the discovered, resolved, matching services, so far.
dump(foundservices)

print("getting status")
local browserstatus = avahi.status()
dump(browserstatus)

if (browserstatus == 'CacheExhausted') then
-- cached items are all loaded

elseif (browserstatus == 'AllForNow') then
-- initial scan (and cache dump) is complete

elseif (browserstatus == 'Failure') then
-- browser died, we need to restart it

else -- browserstatus is nil for success
-- the browser is running its scan

end

-- garbage collector should close down the client too
print("closing")
avahi.close()
avahi = nil

-- the end
