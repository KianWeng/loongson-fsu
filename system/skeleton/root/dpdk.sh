#!/bin/sh
cd /
insmod uio.ko
insmod igb_uio.ko
mkdir /mnt/huge && mount n /mnt/huge -t hugetlbfs
echo 128 > /sys/kernel/mm/hugepages/hugepages-32768kB/nr_hugepages

igbs=$(cd /sys/bus/pci/drivers/igb/; echo 0*;)
ems=$(cd /sys/bus/pci/drivers/e1000e/; echo 0*;)
for i in $ems ;do
echo $i > /sys/bus/pci/drivers/e1000e/unbind
done;

for i in $igbs ;do
echo $i > /sys/bus/pci/drivers/igb/unbind
done;

for i in $(lspci |grep 8086|cut -d' ' -f4);do
echo $i|sed 's/:/ /' > /sys/bus/pci/drivers/igb_uio/new_id
done

./l2fwd  -- -p3 --no-mac-updating
