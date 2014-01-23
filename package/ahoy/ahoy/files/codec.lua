local codec = {}

local i2c = require("i2c")

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
      0x04       -- Reg 42: Mono class-D driver is not muted.
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