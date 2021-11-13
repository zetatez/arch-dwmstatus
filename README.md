![dwm](https://raw.githubusercontent.com/zetatez/arch-dwmstatus/dev/dwmstatus.png)

# dwmstatus
dwmstatus is a simple status bar.

## Requirements
In order to build dwmstatus you need the dwm installed.

## Installation
Edit config.mk to match your local setup (st is installed into
the /usr/local namespace by default).

Afterwards enter the following command to build and install dwmstatus (if
necessary as root):

    mkdir ~/.dwm; cd ~/.dwm
    git clone git@github.com:zetatez/arch-dwmstatus.git
    cd arch-dwmstatus && cp autostart.sh ~/.dwm && chmod 755 autostart.sh

    sh build.sh


