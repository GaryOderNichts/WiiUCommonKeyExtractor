@echo off
windres extractor.rc extractorres.o
gcc -o extractor.exe extractor.c extractorres.o -lgdi32 -lcomdlg32 -lshlwapi -mwindows