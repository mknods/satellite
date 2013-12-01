# $1 movie
# $2 output_file
# $3 time
# $4 width
# $5 height

cd /sat/out/ && rm -rf /sat/out/*
omxplayer $1 &
raspivid -n -o out.h264 -t $3 -w $4 -h $5 && MP4Box -fps 30 -add out.h264 $2
