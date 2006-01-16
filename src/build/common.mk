# Common dependancies.
ifdef ASM_CORE
    ${OBJECT_PATH}/corex86${OBJEXT}: \
        ${SOURCE_PATH}/corex86.asm \
        ${INCLUDE_PATH}/core/x86/offsets.inc \
        ${INCLUDE_PATH}/core/x86/addr.inc \
        ${INCLUDE_PATH}/core/x86/codes.inc \
        ${INCLUDE_PATH}/core/x86/insns.inc
endif
