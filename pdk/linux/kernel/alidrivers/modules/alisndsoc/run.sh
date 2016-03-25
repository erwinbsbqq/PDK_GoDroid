dmesg -c;
udhcpc -i eth0;
mkdir -p /home/tom/3921;
mount -o nolock 10.41.65.72:/home/tom/3921 /home/tom/3921;
cd /home/tom/3921/3921/PDK_S3921/snake/alidrivers/modules/alisndsoc/
insmod ali-snd-codec.ko; insmod ali-snd-cpu-iis.ko; insmod ali-snd-platform.ko;
insmod ali-snd-machine.ko;
/bin/mkdir /dev/snd
/bin/ln -s /dev/pcmC0D0c /dev/snd/pcmC0D0c 
/bin/ln -s /dev/pcmC0D0p /dev/snd/pcmC0D0p 
/bin/ln -s /dev/controlC0 /dev/snd/controlC0 
/bin/ln -s /dev/pcmC1D0c /dev/snd/pcmC1D0c 
/bin/ln -s /dev/pcmC1D0p /dev/snd/pcmC1D0p 
/bin/ln -s /dev/controlC1 /dev/snd/controlC1 
/bin/ln -s /dev/seq /dev/snd/seq 
/bin/ln -s /dev/timer /dev/snd/timer
/home/tom/3921/alsalib/build/bin/amixer cset iface='MIXER',name='Master Playback Volume' 255
/home/tom/3921/alsalib/build/bin/speaker-test -t sine  -f 1000

