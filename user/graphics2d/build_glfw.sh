#!/bin/sh
gcc api_glfw.c graphics2d.c test.c -framework OpenGL -lGLFW -lglew -O2 -std=c99
