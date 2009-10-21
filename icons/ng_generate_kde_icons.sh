#!/bin/bash
setname="nmfnms"
sizes="128:64:48:32:22:16"
types="actions:animations:apps:categories:devices:emblems:emotes:filesystems:intl:mimetypes:places:status"

# create setnamed dir
IFS=':' # set delimiter
[ -d "$setname" ] || mkdir "$setname"
# create size subdirs
for sz in $sizes; do
    dir=$setname/${sz}x${sz}
    [ -d "$dir" ] || mkdir "$dir"
    for typ in $types; do
        [ -d "$dir/$typ" ] || mkdir "$dir/$typ"
    done
done

# reads alias.txt line by line
while read line; do
    echo -n "#"
    IFS=':' # set delimiter
    # split line into source and destination references
    src=${line%%:*}
    dsts=${line#*:}
    svg="$src.svg"; [ -e "$svg" ] || svg="$src.svgz"
    if [ ! -e "$svg" ]; then
        echo "ERROR: source does not exist: \"$src\""
        continue
    fi
    for sz in $sizes; do
        inkscape -w $sz -e "__temp.png" "$svg" > /dev/null
        echo -n "."
        
        IFS=',' # set delimiter
        for dst in $dsts; do
            # split destination references
            typ=${dst%/*}
            path="$setname/${sz}x${sz}/$typ"
            if [ ! -d "$path" ]; then
                echo "ERROR: invalid type: \"$typ\""
                continue
            fi
            files=${dst##*/}
            IFS=':' # set delimiter
            for f1le in $files; do
                # split multi destination references
                cp "__temp.png" "$path/$f1le.png"
            done
        done
        rm -f "__temp.png"
    done
done < alias.txt

echo 
