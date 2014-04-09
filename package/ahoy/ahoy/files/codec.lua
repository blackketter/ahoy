require "utils"

local codec = {}

local i2c = require("i2c")


--------------------------------------------------------------------------------
-- Volume mapping table and function for Register 38
--------------------------------------------------------------------------------
local vol_map = {  
   [0]= -0.0, [1]=-0.5,    [2]=-1.0,   [3]=-1.5,   [4]=-2.0,   [5]=-2.5,   [6]=-3.0,   [7]=-3.5,   [8]=-4.0,   [9]=-4.5, 
  [10]=-5.0,  [11]=-5.5,  [12]=-6.0,  [13]=-6.5,  [14]=-7.0,  [15]=-7.5,  [16]=-8.0,  [17]=-8.5,  [18]=-9.0,  [19]=-9.5, 
  [20]=-10.0, [21]=-10.5, [22]=-11.0, [23]=-11.5, [24]=-12.0, [25]=-12.5, [26]=-13.0, [27]=-13.5, [28]=-14.0, [29]=-14.5, 
  [30]=-15.0, [31]=-15.5, [32]=-16.0, [33]=-16.5, [34]=-17.0, [35]=-17.5, [36]=-18.1, [37]=-18.6, [38]=-19.1, [39]=-19.6, 
  [40]=-20.1, [41]=-20.6, [42]=-21.1, [43]=-21.6, [44]=-22.1, [45]=-22.6, [46]=-23.1, [47]=-23.6, [48]=-24.1, [49]=-24.6, 
  [50]=-25.1, [51]=-25.6, [52]=-26.1, [53]=-26.6, [54]=-27.1, [55]=-27.6, [56]=-28.1, [57]=-28.6, [58]=-29.1, [59]=-29.6, 
  [60]=-30.1, [61]=-30.6, [62]=-31.1, [63]=-31.6, [64]=-32.1, [65]=-32.6, [66]=-33.1, [67]=-33.6, [68]=-34.1, [69]=-34.6, 
  [70]=-35.2, [71]=-35.7, [72]=-36.2, [73]=-36.7, [74]=-37.2, [75]=-37.7, [76]=-38.2, [77]=-38.7, [78]=-39.2, [79]=-39.7, 
  [80]=-40.2, [81]=-40.7, [82]=-41.2, [83]=-41.7, [84]=-42.1, [85]=-42.7, [86]=-43.2, [87]=-43.8, [88]=-44.3, [89]=-44.8, 
  [90]=-45.2, [91]=-45.8, [92]=-46.2, [93]=-46.7, [94]=-47.4, [95]=-47.9, [96]=-48.2, [97]=-48.7, [98]=-49.3, [99]=-50.0, 
  [100]=-50.3,[101]=-51.0,[102]=-51.4,[103]=-51.8,[104]=-52.2,[105]=-52.7,[106]=-53.7,[107]=-54.2,[108]=-55.3,[109]=-56.7,
  [110]=-58.3,[111]=-60.2,[112]=-62.7,[113]=-64.3,[114]=-66.2,[115]=-68.7,[116]=-72.2,[117]=-78.3 }

function lookup_volume(db)

  -- 117 in the AIC3100 map
  local vol_last = #vol_map - 1
   
  if (db >= 0) then 
    return 0
  end
  
  if (db <= vol_map[vol_last]) then
    return vol_last
  end
  
  local i = 0

  -- would be slightly faster with a binary search.  slightly.  
  while (db < vol_map[i]) do
    i = i+1
  end
 
  
 return i
end

--------------------------------------------------------------------------------
local newdb = 0

function codec.volume(db)
  if (db) then
    newdb = db
    local newsetting = lookup_volume(db)
    
    i2c.write(0, 0x18, 0, 1)  -- set page 1
    i2c.write(0, 0x18, 38, newsetting)
  end
  
  return newdb

end

--------------------------------------------------------------------------------
function codec.init()

  -- set page 0
  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    0,           -- starting at register 0
      0,         -- page 0
      0x01       -- software reset
    )
  
--  print("reset, sleeping")
  sleep(0.15)  -- sleep for 150ms
--  print("configuring page 0")

  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    11,          -- starting at register 11
      0x82,             -- Reg 11: dac ndac divide by 2
      0x81             -- Reg dac 12: mdac divide by 1
    )

  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    18,          -- starting at register 18
      0x82,             -- Reg 11: adc ndac divide by 2
      0x81             -- Reg 12: adc mdac divide by 1
    )

  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    28,          -- starting at register 28
      0x00              -- offset in bits
    )

  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    63,          -- starting at register 63
      0xd4,          -- Reg 63: Power up DAC
      0x00           -- Reg 64: unmute DACs
    )

  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    81,          -- starting at register 82
      0x80,        -- Reg 81: enable adc
      0x00         -- Reg 82: unmute adc
    )

  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    86,          -- starting at register 86
      0xA0,      -- Reg 86: enable AGC, -10dB
      0xFE,      -- Reg 87: AGC hysteresis=DISABLE, noise threshold = -90dB
      0x50,      -- Reg 88:  AGC maximum gain= 40 dB
      0x68,      -- Reg 89: Attack time=864/Fs
      0xA8,      -- Reg 90: Decay time=22016/Fs
      0x00,      -- Reg 91: Noise debounce 0 ms
      0x00       -- Reg 92: Signal debounce 0 ms
    )

--  printpage() -- current page is 0

  --------------------------------------------------------------------------------
  -- register page 1
  --------------------------------------------------------------------------------


  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    0,           -- starting at register 0
      1          -- page 1
    )
  
  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    31,          -- starting at register 31
      0xd4,          -- Reg 31: Power up HP amps
      0x86             -- Reg 32: Turn on class d amp
    )
  
  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    35,          -- starting at register 35
      0x44,          -- Reg 35: DAC_L is routed directly to the HPL driver, 
               --     MIC1LP input is not routed to the left-channel mixer amplifier, 
               --     MIC1RP input is not routed to the rightpa-channel mixer amplifier 
               --     DAC_R is routed to the right-channel mixer amplifier.
               --     MIC1RP input is not routed to the right-channel mixer amplifier.
               --     HPL driver output is not routed to the HPR driver.
      0x80,           -- Reg 36: Unity gain on left channel HP out
      0x80,           -- Reg 37: Unity gain on right channel HP out
      0x80            -- Reg 38: Unity gain on left channel to SPKR out
    )

  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    40,          -- starting at register 40
      0x06,      -- Reg 40: HPL driver PGA = 0 dB
                 --     HPL driver is not muted. 
      0x06,      -- Reg 41: HPR driver PGA = 0 dB
                 --         HPR driver is not muted. 
      0x04       -- Reg 42: Mono class-D driver is not muted, gain 6dB
    )

  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    46,          -- starting at register 46
      0x0a,        -- Reg 46: bias is always on and MICBIAS output is powered to 2.5v.
      0x28,        -- Reg 47:  20dB Mic PGA
      0x28,        -- Reg 48: 20k impedance on mic inputs (per caleb)
      0x80         -- Reg 49: 20k impedance on VCOM (per caleb)
      -- Reg 50: djb: WTF?
      )

--  printpage()  -- current page is 1


  i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    0,           -- starting at register 0
      0         -- page 0
    )
end

--------------------------------------------------------------------------------
-- end of module
return codec