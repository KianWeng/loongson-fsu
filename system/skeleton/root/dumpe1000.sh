#!/bin/sh
vbus2phys()
{
if [ $(($1)) -gt $((0x90000000)) ]; then
echo $1
else
printf "0x%x" $(($1&0x0fffffff))
fi
}

devmem()
{
busybox devmem $1 $2
}

dumpdesc()
{
local rb=$1
w=${2:-4}
bw=$(($w*8))
cnt=${3:-256}
i=0
while [ $i -lt $cnt ];
do
printf "%d %x:" $i $(($rb+$i*16)) 
j=0
while [ $j -lt 16 ]; do
echo -n " " $(devmem $(($rb+$i*16+$j)) $bw) 
j=$((j+$w))
done
echo

i=$(($i+1))
done
}

case "$1" in
00*)
bar0=$(od -An -tx4 -j16 -N4 /sys/bus/pci/devices/$1/config|sed 's/ /0x/')
tb=$(devmem $(($bar0+0x3800)) 64)
rb=$(devmem $(($bar0+0x2800)) 64)
tdh=$(devmem $(($bar0+0x3810)) 64)
tdt=$(devmem $(($bar0+0x3818)) 64)

rdh=$(devmem $(($bar0+0x2810)) 64)
rdt=$(devmem $(($bar0+0x2818)) 64)

echo tb:$tb rb:$rb tdh:$tdh tdt:$tdt rdh:$rdh rdt:$rdt

echo $0 $(vbus2phys $tb)
echo $0 $(vbus2phys $rb)
;;
''|eth*)
eth0=${1:-eth0}
bar0=$(od -An -tx4 -j16 -N4 /sys/class/net/$eth0/device/config|sed 's/ /0x/')
tb=$(devmem $(($bar0+0x3800)) 64)
rb=$(devmem $(($bar0+0x2800)) 64)
tdh=$(devmem $(($bar0+0x3810)) 64)
tdt=$(devmem $(($bar0+0x3818)) 64)

rdh=$(devmem $(($bar0+0x2810)) 64)
rdt=$(devmem $(($bar0+0x2818)) 64)

echo tb:$tb rb:$rb tdh:$tdh tdt:$tdt rdh:$rdh rdt:$rdt

echo $0 $(vbus2phys $tb)
echo $0 $(vbus2phys $rb)
;;
*)
dumpdesc $1 $2 $3
;;
esac

