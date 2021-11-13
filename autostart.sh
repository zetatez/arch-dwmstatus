#!/bin/bash

## troj
pkill -9 trojan
cd ~/.trojan
./trojan &

## dwmstatus
dwmstatus 2>&1 >/dev/null &

## accelerate key speed !!
xset r rate 168 96

