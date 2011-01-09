#!/bin/bash

Xephyr -screen 1024x768 :2&
sleep 1
xterm -display :2 -geometry 80x20+10+200&
