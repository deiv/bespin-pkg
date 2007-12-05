#!/bin/sh

if [ "$1" = "install" ]; then
dest=${2:-"/etc/bootsplash/themes"}
echo "Installed files will go to $dest/Bespin"
echo -e "If you're asked for a password, it's the sudo one\n"
sudo cp -rfd Bespin $dest/
exit
fi

csize=$(fbresolution)
size=${1:-${csize}}

width=${size%x*}
height=${size#*x}

if [ ! $width ] || [ ! $height ]; then
echo "usage: $0 WIDTHxHEIGHT"
exit
fi

if [ "$(which inkscape 2>/dev/null )" = "" ]; then
echo "Sorry, you need Inkscape to do this"
exit
fi

if [ "$(which convert 2>/dev/null )" = "" ]; then
echo "Sorry, you need ImageMagick to do this"
exit
fi

if [ ! -d Bespin ]; then mkdir Bespin; fi
if [ ! -d Bespin/config ]; then mkdir Bespin/config; fi
if [ ! -d Bespin/images ]; then mkdir Bespin/images; fi

inkscape -e Bespin/images/bootsplash-${width}x${height}.png -w $width -h $height bootsplash.svg
convert -quality 85 Bespin/images/bootsplash-${width}x${height}.png Bespin/images/bootsplash-${width}x${height}.jpg
rm -f Bespin/images/bootsplash-${width}x${height}.png
inkscape -e Bespin/images/silent-${width}x${height}.png -w $width -h $height silent.svg
convert -quality 85 Bespin/images/silent-${width}x${height}.png Bespin/images/silent-${width}x${height}.jpg
rm -f Bespin/images/silent-${width}x${height}.png

X1=$((472*$width/1280))
Y1=$((618*$height/1024))
Y2=$((626*$height/1024))

TX=$((110*$width/1280))
TY=$((50*$height/1024))
TW=$((1140*$width/1280))
TH=$((970*$height/1024))

echo -e "
# This is a bootsplash configuration file for
# Bespin, resolution ${width}x${height}.
#
# See www.bootsplash.org for more information.
# Written by Thomas Lübking <thomas.luebking@web.de>
#
version=3
state=1
progress_enable=0
overpaintok=1


fgcolor=7
bgcolor=0

tx=$TX
ty=$TY
tw=$TW
th=$TH

jpeg=$dest/Bespin/images/bootsplash-${width}x${height}.jpg
silentjpeg=$dest/Bespin/images/silent-${width}x${height}.jpg

trigger \"isdown\" quit
trigger \"rlreached 5\" toverbose
trigger \"rlchange 0\" tosilent
trigger \"rlchange 6\" tosilent


progress_enable=1
box silent noover $X1 $Y1 $((800*$width/1280)) $Y2 #ffffff10
box silent inter $X1 $Y1 $((480*$width/1280)) $Y2 #ffffff80
box silent $X1 $Y1 $((800*$width/1280)) $Y2 #ffffff80

" > Bespin/config/bootsplash-${width}x${height}.cfg

echo -e "
# This is a bootsplash configuration file for
# Bespin, resolution ${width}x${height}.
#
# See www.bootsplash.org for more information.
# Written by Thomas Lübking <thomas.luebking@web.de>
#
version=3
state=1

fgcolor=7
bgcolor=0

tx=$TX
ty=$TY
tw=$TW
th=$TH

jpeg=$dest/Bespin/images/bootsplash-${width}x${height}.jpg

" > Bespin/config/${width}x${height}.cfg

exit

