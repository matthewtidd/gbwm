#!/bin/bash

Xephyr -screen 1024x768 :2&
sleep 1
xterm -display :2 -geometry 80x20+10+200&
#DISPLAY=:2 gnome-terminal --geometry 80x20+10+200&
