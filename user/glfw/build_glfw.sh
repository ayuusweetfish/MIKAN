#!/bin/sh
gcc api_glfw.c ovo.c -framework OpenGL -lGLFW -lglew -O2 -std=c99
