#! /bin/bash

sudo i2cset -y 1 0x23 0x01 c
sudo i2cset -y 1 0x23 0x07 c
sudo i2cset -y 1 0x23 0x13 c

dat=$(sudo i2cget -y 1 0x23 0x00 w)
msb=$(echo ${dat:4:2})
lsb=$(echo ${dat:2:2})
dec=$(printf "%d\n" "0x$msb$lsb")
echo $dec

