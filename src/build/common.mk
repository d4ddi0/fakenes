

apu.o: apu.c include/apu.h include/apu_ex.h include/cpu.h include/misc.h

audio.o: audio.c include/audio.h include/gui.h include/misc.h \
         include/timing.h

core.o : core.c include/core.h include/core/tables.h include/core/codes.h

cpu.o : cpu.c include/cpu.h include/core.h include/input.h include/misc.h \
        include/mmc.h include/papu.h include/ppu.h include/rom.h

gui.o : gui.c include/apu.h include/audio.h include/cpu.h include/data.h \
        include/gui.h include/misc.h include/papu.h include/ppu.h \
        include/rom.h include/timing.h include/video.h \
        include/gui/dialogs.h include/gui/menus.h include/gui/objects.h

input.o : input.c include/gui.h include/input.h include/misc.h \
          include/ppu.h include/rom.h include/timing.h include/video.h

main.o : main.c include/audio.h include/build.h include/cpu.h \
         include/data.h include/gui.h include/input.h include/misc.h \
         include/mmc.h include/papu.h include/ppu.h include/rom.h \
         include/timing.h include/video.h

mmc.o :  mmc.c include/cpu.h include/gui.h include/misc.h include/mmc.h \
         include/papu.h include/ppu.h include/rom.h include/timing.h \
         include/mmc/shared.h include/mmc/aorom.h include/mmc/bandai.h \
         include/mmc/cnrom.h include/mmc/dreams.h include/mmc/ffe_f3.h \
         include/mmc/gnrom.h include/mmc/mmc1.h include/mmc/mmc2and4.h \
         include/mmc/mmc3.h include/mmc/mmc5.h include/mmc/nina.h \
         include/mmc/sunsoft4.h include/mmc/unrom.h include/mmc/vrc6.h

papu.o : papu.c include/audio.h include/apu.h include/misc.h \
         include/papu.h include/timing.h

ppu.o : ppu.c include/cpu.h include/input.h include/misc.h include/mmc.h \
        include/ppu.h include/rom.h include/timing.h include/video.h \
        include/ppu/tiles.h include/ppu/backgrnd.h include/ppu/sprites.h

rom.o : rom.c include/cpu.h include/rom.h include/misc.h include/mmc.h \
        include/ppu.h support/unzip.h

video.o : video.c include/audio.h include/cpu.h include/data.h \
          include/gui.h include/input.h include/misc.h include/ppu.h \
          include/rom.h include/timing.h include/video.h include/mmc.h
