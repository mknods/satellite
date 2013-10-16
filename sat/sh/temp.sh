#! /bin/bash
dat=$(sudo i2cget -y 1 0x48 0x00 w)
msb=$(echo ${dat:4:2})
lsb=$(echo ${dat:2:2})
dec=$(printf "%d\n" "0x$msb$lsb")
echo "scale=4;$dec*0.0625/8"|bc
