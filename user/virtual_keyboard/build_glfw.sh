#!/bin/sh
gcc api_glfw.c graphics2d.c virtual_keyboard.c -framework OpenGL -lGLFW -lglew -O2 -std=c99
