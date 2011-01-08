#!/bin/bash

Xephyr -screen 1024x768 :2&
sleep 1
xterm -display :2 -geometry 60x20+0+200&
