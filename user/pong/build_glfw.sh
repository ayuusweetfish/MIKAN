#!/bin/sh
gcc api_glfw.c pong.c -framework OpenGL -lGLFW -lglew -O2 -std=c99
