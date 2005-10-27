# Dependancies.
${OBJECT_PATH}/alstream${OBJEXT}: \
    ${SOURCE_PATH}/alstream.c \
    ${INCLUDE_PATH}/alstream.h \
    ${INCLUDE_PATH}/misc.h

${OBJECT_PATH}/apu${OBJEXT}: \
    ${SOURCE_PATH}/apu.c \
    ${INCLUDE_PATH}/apu.h \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/cpu.h \
    ${INCLUDE_PATH}/core.h \
    ${INCLUDE_PATH}/rom.h \
    ${INCLUDE_PATH}/mmc.h \
    ${INCLUDE_PATH}/cpu_in.h \
    ${INCLUDE_PATH}/apu_ex.h \
    ${INCLUDE_PATH}/apu/vrc6.h \
    ${INCLUDE_PATH}/apu/vrc7.h \
    ${INCLUDE_PATH}/apu/fds.h \
    ${INCLUDE_PATH}/apu/mmc5.h \
    ${INCLUDE_PATH}/apu/n106.h \
    ${INCLUDE_PATH}/apu/fme7.h

${OBJECT_PATH}/audio${OBJEXT}: \
    ${SOURCE_PATH}/audio.c \
    ${INCLUDE_PATH}/audio.h \
    ${INCLUDE_PATH}/gui.h \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/timing.h

ifndef ASM_CORE
    ${OBJECT_PATH}/core${OBJEXT}: \
        ${SOURCE_PATH}/core.c \
        ${INCLUDE_PATH}/core.h \
        ${INCLUDE_PATH}/misc.h \
        ${INCLUDE_PATH}/core/tables.h \
        ${INCLUDE_PATH}/core/memory.h \
        ${INCLUDE_PATH}/cpu.h \
        ${INCLUDE_PATH}/rom.h \
        ${INCLUDE_PATH}/mmc.h \
        ${INCLUDE_PATH}/cpu_in.h \
        ${INCLUDE_PATH}/core/addr.h \
        ${INCLUDE_PATH}/core/insns.h \
        ${INCLUDE_PATH}/core/codes.h
else
    ${OBJECT_PATH}/core${OBJEXT}: \
        ${SOURCE_PATH}/core.c \
        ${INCLUDE_PATH}/core.h \
        ${INCLUDE_PATH}/misc.h \
        ${INCLUDE_PATH}/core/tables.h \
        ${INCLUDE_PATH}/core/memory.h \
        ${INCLUDE_PATH}/cpu.h \
        ${INCLUDE_PATH}/rom.h \
        ${INCLUDE_PATH}/mmc.h \
        ${INCLUDE_PATH}/cpu_in.h \
        ${INCLUDE_PATH}/core/addr.h \
        ${INCLUDE_PATH}/core/insns.h

    ${OBJECT_PATH}/corex86${OBJEXT}: \
        ${SOURCE_PATH}/corex86.asm \
        ${INCLUDE_PATH}/core/x86/offsets.inc \
        ${INCLUDE_PATH}/core/x86/addr.inc \
        ${INCLUDE_PATH}/core/x86/codes.inc \
        ${INCLUDE_PATH}/core/x86/insns.inc
endif

${OBJECT_PATH}/cpu${OBJEXT}: \
    ${SOURCE_PATH}/cpu.c \
    ${INCLUDE_PATH}/cpu.h \
    ${INCLUDE_PATH}/core.h \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/rom.h \
    ${INCLUDE_PATH}/mmc.h \
    ${INCLUDE_PATH}/cpu_in.h \
    ${INCLUDE_PATH}/input.h \
    ${INCLUDE_PATH}/papu.h \
    ${INCLUDE_PATH}/ppu.h \
    ${INCLUDE_PATH}/crc32.h

${OBJECT_PATH}/crc32${OBJEXT}: \
    ${SOURCE_PATH}/crc32.c \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/crc32.h

${OBJECT_PATH}/data${OBJEXT}: \
    ${SOURCE_PATH}/data.c

${OBJECT_PATH}/gui${OBJEXT}: \
    ${SOURCE_PATH}/gui.c \
    ${INCLUDE_PATH}/apu.h \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/audio.h \
    ${INCLUDE_PATH}/cpu.h \
    ${INCLUDE_PATH}/core.h \
    ${INCLUDE_PATH}/rom.h \
    ${INCLUDE_PATH}/mmc.h \
    ${INCLUDE_PATH}/cpu_in.h \
    ${INCLUDE_PATH}/gui.h \
    ${INCLUDE_PATH}/input.h \
    ${INCLUDE_PATH}/papu.h \
    ${INCLUDE_PATH}/ppu.h \
    ${INCLUDE_PATH}/video.h \
    ${INCLUDE_PATH}/data.h \
    ${INCLUDE_PATH}/datafile.h \
    ${INCLUDE_PATH}/version.h \
    ${INCLUDE_PATH}/genie.h \
    ${INCLUDE_PATH}/netplay.h \
    ${INCLUDE_PATH}/timing.h \
    ${INCLUDE_PATH}/gui/objects.h \
    ${INCLUDE_PATH}/gui/menus.h \
    ${INCLUDE_PATH}/gui/dialogs.h \
    ${INCLUDE_PATH}/gui/themes.h

${OBJECT_PATH}/input${OBJEXT}: \
    ${SOURCE_PATH}/input.c \
    ${INCLUDE_PATH}/audio.h \
    ${INCLUDE_PATH}/gui.h \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/input.h \
    ${INCLUDE_PATH}/ppu.h \
    ${INCLUDE_PATH}/rom.h \
    ${INCLUDE_PATH}/mmc.h \
    ${INCLUDE_PATH}/video.h \
    ${INCLUDE_PATH}/timing.h

${OBJECT_PATH}/main${OBJEXT}: \
    ${SOURCE_PATH}/main.c \
    ${INCLUDE_PATH}/build.h \
    ${INCLUDE_PATH}/audio.h \
    ${INCLUDE_PATH}/cpu.h \
    ${INCLUDE_PATH}/core.h \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/rom.h \
    ${INCLUDE_PATH}/mmc.h \
    ${INCLUDE_PATH}/cpu_in.h \
    ${INCLUDE_PATH}/gui.h \
    ${INCLUDE_PATH}/input.h \
    ${INCLUDE_PATH}/papu.h \
    ${INCLUDE_PATH}/ppu.h \
    ${INCLUDE_PATH}/video.h \
    ${INCLUDE_PATH}/data.h \
    ${INCLUDE_PATH}/datafile.h \
    ${INCLUDE_PATH}/version.h \
    ${INCLUDE_PATH}/netplay.h \
    ${INCLUDE_PATH}/timing.h

${OBJECT_PATH}/mmc${OBJEXT}: \
    ${SOURCE_PATH}/mmc.c \
    ${INCLUDE_PATH}/cpu.h \
    ${INCLUDE_PATH}/core.h \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/rom.h \
    ${INCLUDE_PATH}/mmc.h \
    ${INCLUDE_PATH}/cpu_in.h \
    ${INCLUDE_PATH}/gui.h \
    ${INCLUDE_PATH}/papu.h \
    ${INCLUDE_PATH}/ppu.h \
    ${INCLUDE_PATH}/timing.h \
    ${INCLUDE_PATH}/mmc/mmc1.h \
    ${INCLUDE_PATH}/mmc/mmc3.h \
    ${INCLUDE_PATH}/mmc/mmc2and4.h \
    ${INCLUDE_PATH}/mmc/mmc5.h \
    ${INCLUDE_PATH}/mmc/unrom.h \
    ${INCLUDE_PATH}/mmc/shared.h \
    ${INCLUDE_PATH}/mmc/cnrom.h \
    ${INCLUDE_PATH}/mmc/aorom.h \
    ${INCLUDE_PATH}/mmc/gnrom.h \
    ${INCLUDE_PATH}/mmc/bandai.h \
    ${INCLUDE_PATH}/mmc/dreams.h \
    ${INCLUDE_PATH}/mmc/nina.h \
    ${INCLUDE_PATH}/mmc/sunsoft4.h \
    ${INCLUDE_PATH}/mmc/vrc6.h \
    ${INCLUDE_PATH}/mmc/ffe_f3.h

${OBJECT_PATH}/netplay${OBJEXT}: \
    ${SOURCE_PATH}/netplay.c \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/netplay.h

${OBJECT_PATH}/papu${OBJEXT}: \
    ${SOURCE_PATH}/papu.c \
    ${INCLUDE_PATH}/audio.h \
    ${INCLUDE_PATH}/apu.h \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/input.h \
    ${INCLUDE_PATH}/papu.h \
    ${INCLUDE_PATH}/timing.h

${OBJECT_PATH}/ppu${OBJEXT}: \
    ${SOURCE_PATH}/ppu.c \
    ${INCLUDE_PATH}/cpu.h \
    ${INCLUDE_PATH}/core.h \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/rom.h \
    ${INCLUDE_PATH}/mmc.h \
    ${INCLUDE_PATH}/cpu_in.h \
    ${INCLUDE_PATH}/input.h \
    ${INCLUDE_PATH}/ppu.h \
    ${INCLUDE_PATH}/video.h \
    ${INCLUDE_PATH}/timing.h \
    ${INCLUDE_PATH}/crc32.h \
    ${INCLUDE_PATH}/ppu/tiles.h \
    ${INCLUDE_PATH}/ppu/backgrnd.h \
    ${INCLUDE_PATH}/ppu/sprites.h

${OBJECT_PATH}/rom${OBJEXT}: \
    ${SOURCE_PATH}/rom.c \
    ${INCLUDE_PATH}/ppu.h \
    ${INCLUDE_PATH}/rom.h \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/mmc.h \
    ${INCLUDE_PATH}/cpu.h \
    ${INCLUDE_PATH}/core.h \
    ${INCLUDE_PATH}/cpu_in.h

${OBJECT_PATH}/video${OBJEXT}: \
    ${SOURCE_PATH}/video.c \
    ${INCLUDE_PATH}/audio.h \
    ${INCLUDE_PATH}/cpu.h \
    ${INCLUDE_PATH}/core.h \
    ${INCLUDE_PATH}/misc.h \
    ${INCLUDE_PATH}/rom.h \
    ${INCLUDE_PATH}/mmc.h \
    ${INCLUDE_PATH}/cpu_in.h \
    ${INCLUDE_PATH}/gui.h \
    ${INCLUDE_PATH}/input.h \
    ${INCLUDE_PATH}/ppu.h \
    ${INCLUDE_PATH}/video.h \
    ${INCLUDE_PATH}/data.h \
    ${INCLUDE_PATH}/datafile.h \
    ${INCLUDE_PATH}/timing.h \
    ${INCLUDE_PATH}/blit/2xsoe.h \
    ${INCLUDE_PATH}/blit/shared.h \
    ${INCLUDE_PATH}/blit/2xscl.h \
    ${INCLUDE_PATH}/blit/interp2x.h \
    ${INCLUDE_PATH}/blit/interp3x.h \
    ${INCLUDE_PATH}/blit/s2xsoe.h \
    ${INCLUDE_PATH}/blit/s2xscl.h \
    ${INCLUDE_PATH}/blit/u2xscl.h

${OBJECT_PATH}/unzip${OBJEXT}: \
    ${SOURCE_PATH}/unzip.c \
    ${INCLUDE_PATH}/unzip.h
