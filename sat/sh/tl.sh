#!/bin/bash
# $1 フレームオフセット
# $2 解像度
# $3 sleep
# $4 枚数
# $5 デバイス /dev/video0
# $6 Brightness 65% 
# &7 Contrast=15%

rm -f /home/pi/img/IMG*
for (( i=0; i<$4; i++ )); do
	filename=$(printf "/sat/img/IMG%03d.jpg" $i)
	echo $filename 
	fswebcam -d $5 -S $1 --no-banner -r $2 -s Brightness=$6% $filename
	sleep 5
done
tar -zcvf /sat/out/img.tgz /sat/img 
#ffmpeg -r 1 -i IMG%03d.jpg out.mp4
#rm -f IMG*

