#!/bin/sh
setname="nmfnms"
sizes="128 64 48 32 22 16"

[ -d "$setname" ] || mkdir "$setname"
for sz in $sizes; do
    dir=$setname/${sz}x${sz}
    [ -d "$dir" ] || mkdir "$dir"
    for svg in *.svg; do
#         links="$(grep ${svg%.svg} $1)"
        png="$dir/${svg%.svg}.png"
        if [ ! -e $png ] || [ $svg -nt $png ]; then
            inkscape -w $sz -e "$png" "$svg"
        fi
    done
done

    
    