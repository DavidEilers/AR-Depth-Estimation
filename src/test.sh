#!/bin/bash
FROM=$1
TO=$2
echo "Trying: cp $FROM $TO"
if [ ! -f "$TO" ]; then
    a=$(cp $FROM $TO)
    exit $a
fi
exit 0