#! /bin/bash

#T=`sudo i2cget -y 1 0x48 0x00 w | ./cnv.pl`
T=`sudo ./temp.sh`
L=`sudo ./lux.sh`
K=100

if [ $L -gt $K ]
then
   ./gpio1.sh
else
   ./gpio0.sh
fi

D=`date +%Y/%m/%d%t%T`
FILE=`date +%Y%m%d`

echo $D $T $L >> ./$FILE
echo $D $T $L
