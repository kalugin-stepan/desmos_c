@echo off

gcc test.c parser.c -o test.exe -I include -L lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -g -I .