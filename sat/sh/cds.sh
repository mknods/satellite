#! /bin/bash
sudo i2cset -y 1 0x23 0x01 c
sudo i2cset -y 1 0x23 0x07 c
sudo i2cset -y 1 0x23 0x10 c
#usleep 120000
sleep 1
dat=$(sudo i2cget -y 1 0x23 0x00 w)
msb=$(echo ${dat:4:2})
lsb=$(echo ${dat:2:2})
dec=$(printf "%d\n" "0x$msb$lsb")
echo "scale=1;$dec*1.0"|bc
