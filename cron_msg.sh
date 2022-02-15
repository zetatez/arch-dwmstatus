#!/bin/sh

#!/usr/bin/env sh

# crontab -e
# * * * * * sh /home/lorenzo/.suckless/arch-dwmstatus/cron_msg.sh >> /dev/null

FILEPATH=/home/lorenzo/.notification.msg
:> $FILEPATH

# Any msgs can be appended on
task|grep -ve "^$\| task\|--" > /tmp/notification.msgs
curl -s http://weicaixun.com/live/|grep text-justify:inter-ideograph|head -n 10|awk -F\> '{print $2}'|awk -F\< '{print $1}' >> /tmp/notification.msgs

ct=0
while [ $ct -lt 20 ];
do
    while read line;
    do
        echo "$line" > $FILEPATH
        let ct=ct+1
        sleep 3
    done < /tmp/notification.msgs
done

