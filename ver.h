#ifndef __VER_H__
#define DISCEMU_VER "0.2.4"
#if defined(USB_ON)
    #if defined(DEADFATTY_KEYPAD_INPUT) && defined(SCREEN_ROTATE)
        #define DEVICE_TYPE "DiscEmu(deadfatty)"
    #elif defined(SCREEN_ROTATE) && !defined(DEADFATTY_KEYPAD_INPUT)
        #if SCREEN_ROTATE==1
            #define DEVICE_TYPE "ITBoot"
        #else
            #define DEVICE_TYPE "DiscEmu"
        #endif
    #elif !defined(DEADFATTY_KEYPAD_INPUT) && !defined(SCREEN_ROTATE)
        #define DEVICE_TYPE "DiscEmu"
    #else
        #define DEVICE_TYPE "DiscEmu(U8g2 Emu)"
    #endif
#else
    #define DEVICE_TYPE "DiscEmu(U8g2 Emu)"
#endif
#ifndef BUILD_DATE
#define BUILD_DATE "2026-01-31 02:40:00"
#endif
#endif