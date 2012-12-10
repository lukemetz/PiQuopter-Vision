sudo insmod servoblaster.ko
echo `sudo sed -n 's/servoblaster//p' /proc/devices`
echo "should be 251"

sudo mknod -m 0666 /dev/servoblaster c 251 0
