

apu.o : apu.c include/apu.h include/misc.h include/cpu.h include/rom.h \
    include/mmc.h include/core.h include/apu_ex.h include/apu/vrc6.h \
    include/apu/vrc7.h include/apu/fds.h include/apu/mmc5.h \
    include/apu/n106.h include/apu/fme7.h

audio.o : audio.c include/audio.h include/gui.h include/misc.h \
    include/timing.h

core.o : core.c include/core.h include/misc.h include/core/tables.h \
    include/core/memory.h include/cpu.h include/rom.h include/mmc.h \
    include/core/addr.h include/core/insns.h include/core/codes.h

cpu.o : cpu.c include/cpu.h include/misc.h include/rom.h include/mmc.h \
    include/core.h include/input.h include/papu.h include/ppu.h \
    include/crc32.h

crc32.o : crc32.c include/misc.h include/crc32.h

data.o : data.c

gui.o : gui.c include/apu.h include/misc.h include/audio.h include/cpu.h \
    include/rom.h include/mmc.h include/core.h include/gui.h \
    include/input.h include/papu.h include/ppu.h include/video.h \
    include/data.h include/datafile.h include/version.h include/genie.h \
    include/netplay.h include/timing.h include/gui/objects.h \
    include/gui/menus.h include/gui/dialogs.h

input.o : input.c include/gui.h include/misc.h include/input.h \
    include/ppu.h include/rom.h include/mmc.h include/video.h \
    include/timing.h

main.o : main.c include/build.h include/audio.h include/cpu.h \
    include/misc.h include/rom.h include/mmc.h include/core.h \
    include/gui.h include/input.h include/papu.h include/ppu.h \
    include/video.h include/data.h include/datafile.h include/version.h \
    include/netplay.h include/timing.h

mmc.o : mmc.c include/cpu.h include/misc.h include/rom.h include/mmc.h \
    include/core.h include/gui.h include/papu.h include/ppu.h \
    include/timing.h include/mmc/mmc1.h include/mmc/mmc3.h \
    include/mmc/mmc2and4.h include/mmc/mmc5.h include/mmc/unrom.h \
    include/mmc/shared.h include/mmc/cnrom.h include/mmc/aorom.h \
    include/mmc/gnrom.h include/mmc/bandai.h include/mmc/dreams.h \
    include/mmc/nina.h include/mmc/sunsoft4.h include/mmc/vrc6.h \
    include/mmc/ffe_f3.h

netplay.o : netplay.c include/misc.h include/netplay.h

papu.o : papu.c include/audio.h include/apu.h include/misc.h \
    include/papu.h include/timing.h

ppu.o : ppu.c include/cpu.h include/misc.h include/rom.h include/mmc.h \
    include/core.h include/input.h include/ppu.h include/video.h \
    include/timing.h include/crc32.h include/ppu/tiles.h \
    include/ppu/backgrnd.h include/ppu/sprites.h

rom.o : rom.c include/ppu.h include/rom.h include/misc.h include/mmc.h \
    include/cpu.h include/core.h

video.o : video.c include/audio.h include/cpu.h include/misc.h \
    include/rom.h include/mmc.h include/core.h include/gui.h \
    include/input.h include/ppu.h include/video.h include/data.h \
    include/datafile.h include/timing.h
