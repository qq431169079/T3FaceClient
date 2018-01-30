#!/bin/sh
echo "000000" | sudo -S service sshd start
#/etc/init.d/ssh start
sudo service sshd start
#echo "000000" | sudo -S service sshd start

sleep 0.3s
cvt 892 500 60
sleep 0.3s
xrandr --newmode "896x500_60.00"   34.75  896 920 1008 1120  500 503 513 521 -hsync +vsync
sleep 0.3s
xrandr --addmode HDMI-1 "896x500_60.00"
sleep 0.3s
xrandr --output HDMI-1 --mode "896x500_60.00"
sleep 0.3s
xrandr -o right
sleep 1s  
  
appname=`basename $0 | sed s,\.sh$,,`  
  
dirname=`dirname $0`  
tmp="${dirname#?}"  
  
if [ "${dirname%$tmp}" != "/" ]; then  
dirname=$PWD/$dirname  
fi  
LD_LIBRARY_PATH=$dirname  
export LD_LIBRARY_PATH  
$dirname/$appname "$@"  
