local audio = {}

local i2c = require("i2c")


-- global
audioDevice = nil

--------------------------------------------------------------------------------
local function hexdump(str, spacer)
    return (
        string.gsub(str,"(.)",
            function (c)
                return string.format("%02X%s",string.byte(c), spacer or "")
            end
        )
    )
end

--------------------------------------------------------------------------------
local function printpage()

  for i = 0,9 do io.write("   ",i," ") end
  print()
  
  for i,v in ipairs(i2c.read(0, 0x18, 0, 100)) do
    io.write(string.format("0x%02x ",v))
    
    if (i % 10 == 0)  then print(" ", i-10) end
  end
  print("")
end

--------------------------------------------------------------------------------
local function configureCodec()

  -- set page 0
  print(i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    0,           -- starting at register 0
      0,         -- page 0
      0x01       -- software reset
    )
  )
  
  print("reset, sleeping")
  sleep(0.15)  -- sleep for 150ms
  print("configuring page 0")

  print(i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    11,          -- starting at register 11
      0x82,             -- Reg 11: dac ndac divide by 2
      0x81             -- Reg dac 12: mdac divide by 1
    )
  )

  print(i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    18,          -- starting at register 18
      0x82,             -- Reg 11: adc ndac divide by 2
      0x81             -- Reg 12: adc mdac divide by 1
    )
  )

  print(i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    28,          -- starting at register 28
      0x00              -- offset in bits
      )
  )

  print(i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    63,          -- starting at register 63
      0xd4,          -- Reg 63: Power up DAC
      0x00           -- Reg 64: unmute DACs
    )
  )

  print(i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    81,          -- starting at register 82
      0x80,        -- Reg 81: enable adc
      0x00         -- Reg 82: unmute adc
    )
  )

  printpage() -- current page is 0

  --------------------------------------------------------------------------------
  -- register page 1
  --------------------------------------------------------------------------------


  print(i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    0,           -- starting at register 0
      1          -- page 1
    )
  )
  
  print(i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    31,          -- starting at register 31
      0xd4,          -- Reg 31: Power up HP amps
      0x86             -- Reg 32: Turn on class d amp
    )
  )
  
  print(i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
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
  )

  print(i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    40,          -- starting at register 40
      0x06,      -- Reg 40: HPL driver PGA = 0 dB
                 --     HPL driver is not muted. 
      0x06,      -- Reg 41: HPR driver PGA = 0 dB
                 --         HPR driver is not muted. 
      0x04       -- Reg 42: Mono class-D driver is not muted.
    )
  )

  print(i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    46,          -- starting at register 46
      0x0a,        -- Reg 46: bias is always on and MICBIAS output is powered to 2.5v.
      0x28,        -- Reg 47:  20dB Mic PGA
      0x28,        -- Reg 48: 20k impedance on mic inputs (per caleb)
      0x80         -- Reg 49: 20k impedance on VCOM (per caleb)
      -- Reg 50: djb: WTF?
      )
  )

  printpage()  -- current page is 1


  print(i2c.write(0, 0x18, -- bus 0, device 0x18 : tlv320aic3100
    0,           -- starting at register 0
      0         -- page 0
    )
  )
end

--------------------------------------------------------------------------------
function audio.init()
  configureCodec()
  
  if (audioDevice != nil) 
    audioDevice = assert(io.open('/dev/i2s', 'r+b'), 'failure to open /dev/i2s')
  end
end

--------------------------------------------------------------------------------
function audio.pause()
--[[
djb: todo
Some examples:
    printf("Pausing.........\n");
    if (ioctl(audio, I2S_PAUSE, recorder) < 0) {
        perror("I2S_PAUSE");
    }
--]]
end

--------------------------------------------------------------------------------
function audio.resume()
--[[
djb: todo
    printf("Resuming.........\n");
    if (ioctl(audio, I2S_RESUME, recorder) < 0) {
        perror("I2S_RESUME");
    }
--]]
end


--------------------------------------------------------------------------------
function audio.isPaused()
-- djb: todo
  return false
end

--------------------------------------------------------------------------------
function audio.sampleSize(newSize)
--[[
  djb: todo
	if (ioctl(audio, I2S_DSIZE, hdr.bit_p_spl) < 0) {
		perror("I2S_DSIZE");
	}
--]]
  return newSize
end

--------------------------------------------------------------------------------
function audio.sampleRate(newRate)
--[[
  djb: todo
	if (ioctl(audio, I2S_FREQ, hdr.sample_fq) < 0) {
		perror("I2S_FREQ");
	}
--]]
  return newRate
end

--------------------------------------------------------------------------------
function audio.read(bytesToRead)
  -- djb: todo - make non-blocking

  return assert(audioDevice:read(bytesToRead)
end

--[[
    audiodata = (char *) malloc (bufsz * sizeof (char));
    if (audiodata == NULL) {
        return ENOMEM;
    }

    do {
        /*
         * Bug#:    26972
         * The byte stream after the `.wav' header could have
         * additional data (like author, album etc...) apart
         * from the actual `audio data'.  Hence, ensure that
         * extra stuff is not written to the device.  Stop at
         * wherever the audio data ends.
         *  +--------+----------------------+--------+
         *  | header | audio data . . . . . | extras |
         *  +--------+----------------------+--------+
         */
        count = bufsz;

eagain:
        ret = read (audio, audiodata, count);
        if (ret < 0 && errno == EAGAIN) {
            printf("record %d, error %d \n", __LINE__, ret);
            goto eagain;
        }

        if ((write(fd, audiodata, ret)) < 0)  {
            printf("record %d, error %d \n", __LINE__, ret);
            perror("Read audio data");
            break;
        }


        i += ret;

    } while (i <= sc.data_length);

    free (audiodata);
--]]
end

--------------------------------------------------------------------------------
function audio.write(soundData)

  -- djb: to do - make non-blocking
  audioDevice:write(soundData)
  
  return string.len(soundData)

--[[
	if (ioctl(audio, I2S_DSIZE, hdr.bit_p_spl) < 0) {
		perror("I2S_DSIZE");
	}

	if (ioctl(audio, I2S_FREQ, hdr.sample_fq) < 0) {
		perror("I2S_FREQ");
	}

    if (mclk_sel) {
	    if (ioctl(audio, I2S_MCLK, mclk_sel) < 0) {
	    	perror("I2S_MCLK");
	    }
	}

	audiodata = (char *) malloc (bufsz * sizeof (char));
	if (audiodata == NULL) {
		return ENOMEM;
	}

	for (i = 0; i <= sc.data_length; i += bufsz) {
		/*
		 * Bug#:	26972
		 * The byte stream after the `.wav' header could have
		 * additional data (like author, album etc...) apart
		 * from the actual `audio data'.  Hence, ensure that
		 * extra stuff is not written to the device.  Stop at
		 * wherever the audio data ends.
		 *	+--------+----------------------+--------+
		 *	| header | audio data . . . . . | extras |
		 *	+--------+----------------------+--------+
		 */
		count = bufsz;
		if ((i + count) > sc.data_length) {
			count = sc.data_length - i;
		}

		if ((count = read (fd, audiodata, count)) <= 0) {
			perror("Read audio data");
			break;
		}

#ifdef WASP
        tmpcount = count;
        data = audiodata;
        ret = 0;
        if (valfix != -1) {
            memset(data, valfix, tmpcount);
        }
erestart:
        ret = write(audio, data, tmpcount);
        if (ret == -ERESTART) {
            goto erestart;
        }
#else
eagain:                                                         
        tmpcount = count;                                       
        data = audiodata;                                       
        ret = 0;                                                
        do {                                                    
            ret = write(audio, data, tmpcount);                     
            if (ret < 0 && errno == EAGAIN) {                       
                dp("%s:%d %d %d\n", __func__, __LINE__, ret, errno);
                goto eagain;                                        
            }                                                       
            tmpcount = tmpcount - ret;                              
            data += ret;                                            
        } while(tmpcount);                                      
#endif
		dp("i = %d\n", i);
	}

	free (audiodata);
--]]

--------------------------------------------------------------------------------
-- end of module
return audio