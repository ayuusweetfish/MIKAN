#!/bin/sh
gcc api_glfw.c main.c -framework OpenGL -lGLFW -lglew -O2 -std=c99
