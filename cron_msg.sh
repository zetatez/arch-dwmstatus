#!/usr/bin/env sh

# crontab -e
# * * * * * sh /home/lorenzo/.suckless/arch-dwmstatus/cron_msg.sh >> /dev/null

FILEPATH=/home/lorenzo/.notification.msg
:> $FILEPATH

# any msgs can be appended on
task|grep -ve "^$\| task\|--" > /tmp/notification.msgs


ct=0
while [ $ct -lt 10 ];
do
    while read line;
    do
        echo "$line" > $FILEPATH
        let ct=ct+1
        sleep 6
    done < /tmp/notification.msgs
done

