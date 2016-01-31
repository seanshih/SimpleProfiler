#! /bin/bash
IFS=$'\n'       # make newlines the only separator

CONTENT=`cat profiled_report.txt`
REPLACER=""

for line in $CONTENT; do 
    IFS=', ' read -a array <<< "$line"

    if [[ ${array[1]} == '%' ]] ; then
        REPLACER="$REPLACER [\"${array[2]}\", ${array[0]}],"
    fi
done

OUTFILE=piechart.html
TEMPLATE=piechart.template
REPLACED_TEXT="s/\/\/REPLACE_TOKEN_REPLACE/$REPLACER/g"
sed $REPLACED_TEXT $TEMPLATE >> $OUTFILE

start $OUTFILE


