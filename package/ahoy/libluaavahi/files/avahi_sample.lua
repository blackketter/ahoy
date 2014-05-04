local avahi = require("avahi")

avahi.open()

avahi.browse("_ahoy._tcp")

local pollresult = avahi.poll(0)  -- pass in the sleep time, defaults to zero, returns -1 on failure, nil on success and 1 if a quit is pending

local foundservices = avahi.found() -- returns a table containing the discovered, resolved, matching services, so far.

local browserstatus = avahi.status()

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
avahi.close()
avahi = nil

-- the end
