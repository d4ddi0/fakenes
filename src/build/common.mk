

apu${OBJEXT} : apu.c include/apu.h include/misc.h include/cpu.h \
    include/rom.h include/mmc.h include/core.h include/apu_ex.h \
    include/apu/vrc6.h include/apu/vrc7.h include/apu/fds.h \
    include/apu/mmc5.h include/apu/n106.h include/apu/fme7.h

audio${OBJEXT} : audio.c include/audio.h include/gui.h include/misc.h \
    include/timing.h

core${OBJEXT} : core.c include/core.h include/misc.h include/cpu.h \
    include/rom.h include/mmc.h include/core/tables.h \
    include/core/addr.h include/core/insns.h \
    include/core/codes.h

corex86${OBJEXT}: corex86.asm support/coreoff.inc \
    include/core/x86/addr.inc include/core/x86/codes.inc \
    include/core/x86/insns.inc

cpu${OBJEXT} : cpu.c include/cpu.h include/misc.h include/rom.h \
    include/mmc.h include/core.h include/input.h include/papu.h \
    include/ppu.h include/crc32.h

data${OBJEXT} : data.c include/datafile.h

gui${OBJEXT} : gui.c include/apu.h include/misc.h include/audio.h \
    include/cpu.h include/rom.h include/mmc.h include/core.h \
    include/gui.h include/input.h include/papu.h include/ppu.h \
    include/video.h include/data.h include/datafile.h include/netplay.h \
    include/timing.h include/gui/objects.h include/gui/menus.h \
    include/gui/dialogs.h include/version.h

input${OBJEXT} : input.c include/gui.h include/misc.h include/input.h \
    include/ppu.h include/rom.h include/mmc.h include/video.h \
    include/timing.h

main${OBJEXT} : main.c include/build.h include/audio.h include/cpu.h \
    include/misc.h include/rom.h include/mmc.h include/core.h \
    include/gui.h include/input.h include/papu.h include/ppu.h \
    include/video.h include/data.h include/datafile.h include/netplay.h \
    include/timing.h include/version.h

mmc${OBJEXT} : mmc.c include/cpu.h include/misc.h include/rom.h \
    include/mmc.h include/core.h include/gui.h include/papu.h \
    include/ppu.h include/timing.h include/mmc/mmc1.h include/mmc/mmc3.h \
    include/mmc/mmc2and4.h include/mmc/mmc5.h include/mmc/unrom.h \
    include/mmc/shared.h include/mmc/cnrom.h include/mmc/aorom.h \
    include/mmc/gnrom.h include/mmc/bandai.h include/mmc/dreams.h \
    include/mmc/nina.h include/mmc/sunsoft4.h include/mmc/vrc6.h \
    include/mmc/ffe_f3.h

netplay${OBJEXT} : netplay.c include/misc.h include/netplay.h

papu${OBJEXT} : papu.c include/audio.h include/apu.h include/misc.h \
    include/papu.h include/timing.h

ppu${OBJEXT} : ppu.c include/cpu.h include/misc.h include/rom.h \
    include/mmc.h include/core.h include/input.h include/ppu.h \
    include/video.h include/timing.h include/crc32.h include/ppu/tiles.h \
    include/ppu/backgrnd.h include/ppu/sprites.h

rom${OBJEXT} : rom.c include/ppu.h include/rom.h include/misc.h \
    include/mmc.h include/cpu.h include/core.h support/unzip.h

crc32${OBJEXT} : crc32.c include/misc.h include/crc32.h

video${OBJEXT} : video.c include/audio.h include/cpu.h include/misc.h \
    include/rom.h include/mmc.h include/core.h include/gui.h \
    include/input.h include/ppu.h include/video.h include/data.h \
    include/datafile.h include/timing.h

unzip${OBJEXT} : support/unzip.c support/unzip.h

