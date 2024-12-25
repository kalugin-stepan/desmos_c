@echo off

gcc main.c parser.c -o main.exe -I include -L lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -I . -O3