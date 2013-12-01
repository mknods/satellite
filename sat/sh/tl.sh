#!/bin/bash
# $1 /dev/video0
# $2 frame offset
# $3 1920x1080
# $4 Brightness 65%
# $5 image num
# $6 sleep(sec)
# $7 Contrast=15%

rm -f /sat/img/*
for (( i=0; i<$5; i++ )); do
	filename=$(printf "/sat/img/IMG%03d.jpg" $i)
	echo $filename 
	fswebcam -d $1 -S $2 --no-banner -r $3 -s Brightness=$4% $filename
	sleep $6
done
tar -zcvf /sat/out/img.tgz /sat/img 

