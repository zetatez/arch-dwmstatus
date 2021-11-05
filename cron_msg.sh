#!/usr/bin/env sh

# crontab -e
# */3 * * * * sh /home/lorenzo/suckless/arch-dwmstatus/cron_msg.sh >> /dev/null

FILEPATH=/home/lorenzo/.notification.msg
:> $FILEPATH

task list|grep -v "ID\|---\|task\|^$"|sed "s/[ ][ ]*/ /g"|head -n 1 > $FILEPATH

