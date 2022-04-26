## README for sample/SEN-SCD30

### Prerequisite

- (For Raspberry Pi) I2C interface must be enabled (`raspi-config`)
- The i2c address `0x61` of the sensor must be available on `/dev/i2c-1`: You can check it with `i2cdetect` as follows:
```bash
$ sudo i2cdetect -y 1
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- 61 -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --
```

### Build and run the sample

1. Clone this repository with `--recursive` to also clone required submodules
2. Go to `lib/device/SEN-SCD30/embedded-scd` and run `make prepare` to generate a required file (`scd_git_version.c`)
3. Return to this sample directory then run `make`
4. Run `./sample`
