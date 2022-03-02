#!/bin/sh

#!/usr/bin/env sh

# crontab -e
# * * * * * sh /home/lorenzo/.suckless/arch-dwmstatus/cron_msg.sh >> /dev/null

FILEPATH=/home/lorenzo/.notification.tasks
:> $FILEPATH

# Any msgs can be appended on
task -l|grep -vi "^ID\|--\|^$\|task$" > /tmp/notification.tasks

ct=0
while [ $ct -lt 20 ];
do
    while read line;
    do
        echo "$line" > $FILEPATH
        let ct=ct+1
        sleep 3
    done < /tmp/notification.tasks
done

