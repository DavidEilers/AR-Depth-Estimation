#!/bin/bash
#Copies files/directorys if the copy(TO) doesn't exist or if the source(FROM) is newer
FROM=$1
TO=$2
echo "Trying: cp $FROM $TO"
if [ ! -f "$TO" ] && [ ! -d "$TO" ]; then #If destination doesn't exist
    a=$(cp -r $FROM $TO)
    exit $a
elif ["$FROM" -nt "$TO"]; then #If source is newer than destination
    $(rm -r $TO)
    a=$(cp -r $FROM $TO)
    exit $a
fi
exit 0