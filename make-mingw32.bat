@echo off
cd src
mingw32-make -f build/makefile.mingw32 %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..
