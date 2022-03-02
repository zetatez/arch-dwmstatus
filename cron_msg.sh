#!/bin/sh

#!/usr/bin/env sh

# crontab -e
# * * * * * sh /home/lorenzo/.suckless/arch-dwmstatus/cron_msg.sh >> /dev/null

FILEPATH=/home/lorenzo/.notification.msgs
:> $FILEPATH

while true; do
    # Any msgs can be appended on
    curl -s http://weicaixun.com/live/|grep text-justify:inter-ideograph|head -n 10|grep -o -P "(?<=ph\">).*(?=\<)"|sed 's/<.*>//g' > /tmp/notification.msgs

    ct=0
    while [ $ct -lt 10 ];
    do
        while read line;
        do
            dunstify "$line"
            let ct=ct+1
            sleep 6
        done < /tmp/notification.msgs
    done
done &


