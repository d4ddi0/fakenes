@echo off
if "%1" == "" goto error1
if "%CMDEXTVERSION%" == "" goto old
if /i "%1" == "djgpp" goto djgpp
if /i "%1" == "mingw32" goto mingw32
goto error2

:old
if "%1" == "DJGPP" goto djgpp
if "%1" == "MINGW32" goto mingw32
goto error2

:djgpp
echo MAKE = make > config.mk
echo RM = rm -f >> config.mk
echo. >> config.mk
echo MAKEFILE = build/Makefile.djgpp >> config.mk
echo Run 'make' to build FakeNES.
goto end

:mingw32
echo MAKE = mingw32-make > config.mk
echo RM = rm -f >> config.mk
echo. >> config.mk
echo MAKEFILE = build/Makefile.mingw32 >> config.mk
echo Run 'mingw32-make' to build FakeNES.
goto end

:error1
echo No platform specified.
echo.
echo Valid platforms:
echo    djgpp       (DJGPP)
echo    mingw32     (Mingw32)
goto end

:error2
echo Unknown platform (%1).

:end
