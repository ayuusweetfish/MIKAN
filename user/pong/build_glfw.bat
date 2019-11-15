:: MSYS2: pacman -S mingw-w64-x86_64-glfw
:: MSYS2: pacman -S mingw-w64-x86_64-glew

gcc api_glfw.c ovo.c -lopengl32 -lGLFW3 -lglew32 -O2 -std=c99
