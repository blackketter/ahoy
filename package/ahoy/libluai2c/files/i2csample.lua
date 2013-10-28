local i2c = require("i2c")

function hexdump(str, spacer)
    return (
        string.gsub(str,"(.)",
            function (c)
                return string.format("%02X%s",string.byte(c), spacer or "")
            end
        )
    )
end

--listing members in i2c, debug purpose
for key, value in pairs(i2c) do
    print("found member " .. key);
end

--print(i2c.write(0, 0x18, '\00\00\42')) --write decimal 42 to address 0 on 25AA512(2bytes address)


--repeat read until chip is ready, todo add some type of timeout
reads=0
repeat
        reads = reads + 1
        status, data = i2c.read(0, 0x18, 0, 10)
until status == 0

print( reads )
print( hexdump(data, " ") )