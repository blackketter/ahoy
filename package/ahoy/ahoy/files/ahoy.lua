hw_codec = require "hwcodec"

require "socket"

function sleep(sec)
    socket.select(nil, nil, sec)
end


hw_codec.init_codec()
  
for i=0,math.huge do
  print("Playing audio ", i)  
  os.execute("athplay /etc/hail.wav")
end
