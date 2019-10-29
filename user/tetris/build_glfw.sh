#!/bin/sh
gcc api_glfw.c tetris.c main.c -framework OpenGL -lGLFW -lglew -O2 -std=c99
