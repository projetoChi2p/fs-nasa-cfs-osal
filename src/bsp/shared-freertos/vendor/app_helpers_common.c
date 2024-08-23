#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>


#include "app_helpers.h"





#define DEBUG_PRINT_NUMBER 0

static void vPrintNumber(
    uint8_t buffer[16],
    const int32_t n,
    const uint8_t base,
    const uint8_t unsigned_flag,
    const uint8_t do_padding,
    const uint8_t pad_character,
    const uint8_t num1
)
{
    int32_t len;
    int32_t negative;
    int32_t i;
    int32_t j;
    uint8_t outbuf[16];
    const uint8_t digits[] = "0123456789ABCDEF";
    uint32_t num;
    for (i = 0; i < sizeof(outbuf); i++) {
        outbuf[i] = '0';
    }

#if DEBUG_PRINT_NUMBER
    printf("\r\nvPrintNumber() ********************\r\nn: %d [%d]\r\n", n, __LINE__);
#endif

    /* Check if number is negative                   */
    if ((unsigned_flag == 0) && (base == 10) && (n < 0L)) {
        negative = 1;
        num = (-(n));
    }
    else {
        num = n;
        negative = 0;
    }

    /* Build number (backwards) in outbuf            */
    i = 0;
    do {
        outbuf[i] = digits[(num % base)];
        i++;
        num /= base;
    } while (num > 0);

    if (negative != 0) {
        outbuf[i] = '-';
        i++;
    }

    outbuf[i] = 0;
    i--;

    len = strlen((char*)outbuf);

#if DEBUG_PRINT_NUMBER
    printf("outbuf: '");
    for (j = 0; j < 16; j++) {
        if (outbuf[j] <= ' ') {
            printf("<%d>", outbuf[j]);
        }
        else {
            printf("%c", outbuf[j]);
        }
    }
    printf("'\r\n");
    printf("len: %d\r\n", len);
    printf("i: %d\r\n", i);
#endif

    if ((do_padding != 0) && (len < num1)) {
        i = len;
        for (; i < num1; i++) {
            outbuf[i] = pad_character;
        }

        outbuf[i] = 0;
        i--;
    }

#if DEBUG_PRINT_NUMBER
    len = strlen(outbuf);
    printf("x outbuf: '");
    for (j = 0; j < 16; j++) {
        if (outbuf[j] <= ' ') {
            printf("<%d>", outbuf[j]);
        }
        else {
            printf("%c", outbuf[j]);
        }
    }
    printf("'\r\n");
    printf("x len: %d\r\n", len);
    printf("x i: %d\r\n", i);
#endif

    for (j = 0; j < 16; j++) {
        buffer[j] = 0;
    }

    j = 0;
    while (i >= 0) {
        buffer[j] = outbuf[i];
        i--;
        j++;
    }

#if DEBUG_PRINT_NUMBER
    buffer[j] = 0;
    printf("z buffer: '");
    for (j = 0; j < 16; j++) {
        if (buffer[j] <= ' ') {
            printf("<%d>", buffer[j]);
        }
        else {
            printf("%c", buffer[j]);
        }
    }
    printf("'\r\n");
#endif
}



void HLP_vPrintI32(uint8_t outbuf[16], const int32_t n) {
    vPrintNumber(
        outbuf, n,
        10,   /* base */
        0,    /* unsigned_flag */
        0,    /* do_padding */
        ' ',  /* pad_character */
        0     /* num1 */);
}

void HLP_vPrintU32(uint8_t outbuf[16], const uint32_t n) {
    vPrintNumber(
        outbuf, n,
        10,   /* base */
        1,    /* unsigned_flag */
        0,    /* do_padding */
        ' ',  /* pad_character */
        0     /* num1 */);
}


void HLP_vPrintU16(uint8_t outbuf[16], const uint16_t n) {
    vPrintNumber(
        outbuf, n,
        10,   /* base */
        1,    /* unsigned_flag */
        0,    /* do_padding */
        ' ',  /* pad_character */
        0     /* num1 */);
}

void HLP_vPrintU8(uint8_t outbuf[16], const uint8_t n) {
    vPrintNumber(
        outbuf, n,
        10,   /* base */
        1,    /* unsigned_flag */
        0,    /* do_padding */
        ' ',  /* pad_character */
        0     /* num1 */);
}


void HLP_vPrintHexU32(uint8_t outbuf[16], const uint32_t n) {
    vPrintNumber(
        outbuf, n,
        16,   /* base */
        1,    /* unsigned_flag */
        1,    /* do_padding */
        '0',  /* pad_character */
        8     /* num1 */);
}


void HLP_vPrintHexU8(uint8_t outbuf[16], const uint8_t n) {
    vPrintNumber(
        outbuf, n,
        16,   /* base */
        1,    /* unsigned_flag */
        1,    /* do_padding */
        '0',  /* pad_character */
        2     /* num1 */);
}

void HLP_vPrintHexU16(uint8_t outbuf[16], const uint16_t n) {
    vPrintNumber(
        outbuf, n,
        16,   /* base */
        1,    /* unsigned_flag */
        1,    /* do_padding */
        '0',  /* pad_character */
        4     /* num1 */);
}


void HLP_vPrintFloat(uint8_t outbuf[16], float f, uint8_t integer_pad, uint8_t decimals_pad) {
    int i;
    uint8_t intbuf[16];
    uint8_t decbuf[16];
    uint8_t outbuf_sz = 15;

    int32_t dec_multiplier = 10;

    if (decimals_pad < 1) {
        decimals_pad = 1;
    }
    if (decimals_pad > 15) {
        decimals_pad = 15;
    }
    if (integer_pad > 15) {
        integer_pad = 15;
    }

    for (i = 1; i < decimals_pad; i++) {
        dec_multiplier *= 10;
    }

    int32_t intval = (int)(f);
    int32_t decval = (int)((f - (int)(f)) * dec_multiplier);
    if (decval < 0) {
        decval = (-decval);
    }


    vPrintNumber(
        intbuf, intval,
        10,     /* base */
        0,      /* unsigned_flag */
        1,      /* do_padding */
        ' ',    /* pad_character */
        integer_pad /* num1 */);

    vPrintNumber(
        decbuf, decval,
        10,       /* base */
        0,        /* unsigned_flag */
        1,        /* do_padding */
        '0',      /* pad_character */
        decimals_pad  /* num1 */);

    strncpy((char*)outbuf, (char*)intbuf, outbuf_sz);
    strncat((char*)outbuf, ".", outbuf_sz);
    strncat((char*)outbuf, (char*)decbuf, outbuf_sz);
}


/*********************************************************************
 */
char HLP_cNible2Text(uint8_t nibble) {
    char result;

    if (nibble >= 62) {
        result = '~';
    }
    else if (nibble >= 36) {
        result = nibble + 'a' - 36;
    }
    else if (nibble >= 10) {
        result = nibble + 'A' - 10;
    }
    else {
        result = nibble + '0';
    }

    return result;
}


/*********************************************************************
 * from MicroNMEA
 */
char* HLP_pcGenerateChecksum(char* s, char* checksum) {
    uint8_t c = 0;
    // Initial $ is omitted from checksum, if present ignore it.
    if (*s == '$')
        ++s;

    while (*s != '\0' && *s != '*')
        c ^= *s++;

    if (checksum) {
        checksum[0] = HLP_cNible2Text(c / 16);
        checksum[1] = HLP_cNible2Text(c % 16);
    }
    return s;
}


/*********************************************************************
 */
uint8_t HLP_u8BuildFlags(void) {
    return HLP_BUILD_FLAGS;
}


int HLP_bIsBigEndian(void) {
    union {
        uint32_t i;
        char c[4];
    } e = { 0x01000000 };

    return e.c[0];
}




uint8_t HLP_u8Maj(volatile const uint8_t v1, volatile const uint8_t v2, volatile const uint8_t v3) {
    return ((v2 & v3) | (v1 & v3) | (v1 & v2) | (v1 & v2 & v3));
}

uint16_t HLP_u16Maj(volatile const uint16_t v1, volatile const uint16_t v2, volatile const uint16_t v3) {
    return ((v2 & v3) | (v1 & v3) | (v1 & v2) | (v1 & v2 & v3));
}


#define FLASH_ICACHE_PREFETCH_ENABLED   (1<<0)
#define FLASH_ICACHE_ENABLED            (1<<1)
#define FLASH_DCACHE_ENABLED            (1<<2)

#define CORTEX_ICACHE_ENABLED           (1<<4)
#define CORTEX_DCACHE_ENABLED           (1<<5)
#define CORTEX_CACHE_WRITE_THROUGH      (1<<6)

uint8_t HLP_u8GetCacheSettings(void) {
    uint8_t cache_settings = 0;
   
    //
    // Vendor specific SoC Flash cache
    //
    // STM ART settings, stm32l4xx_hal_conf.h or stm32f7xx_hal_conf.h
    #ifdef PREFETCH_ENABLE
    #if (PREFETCH_ENABLE)
        cache_settings |= FLASH_ICACHE_PREFETCH_ENABLED;
    #endif
    #endif
    
    // STM ART settings, stm32l4xx_hal_conf.h
    #ifdef INSTRUCTION_CACHE_ENABLE
    #if (INSTRUCTION_CACHE_ENABLE)
        cache_settings |= FLASH_ICACHE_ENABLED;
    #endif
    #endif

    // STM ART settings, stm32f7xx_hal_conf.h
    //For stm32f7 the ART cache covers only accesses through ITCM interface
    // ART_ACCLERATOR_ENABLE (!sic), not ART_ACCELERATOR_ENABLE
    #ifdef ART_ACCLERATOR_ENABLE
    #if (ART_ACCLERATOR_ENABLE)
        cache_settings |= FLASH_ICACHE_ENABLED;
    #endif
    #endif
    
    // Cypress PSoC 5 LP
    #ifdef CYDEV_INSTRUCT_CACHE_ENABLED
    #if (CYDEV_INSTRUCT_CACHE_ENABLED)
        cache_settings |= FLASH_ICACHE_ENABLED;
    #endif
    #endif
        
    // STM ART settings, stm32l4xx_hal_conf.h
    #ifdef DATA_CACHE_ENABLE
    #if (DATA_CACHE_ENABLE)
        cache_settings |= FLASH_DCACHE_ENABLED;
    #endif
    #endif
    
    
    //
    // Arm Cortex CPU cache
    //
    
    //stm32f7xx
    //app_helpers.h see also HLP_vSystemConfig() for MPU_Config() and/or CPU_CACHE_Enable()
    #ifdef HLP_INSTRUCTION_CACHE_ENABLE
    #if (HLP_INSTRUCTION_CACHE_ENABLE)
        cache_settings |= CORTEX_ICACHE_ENABLED;
    #endif
    #endif
            
    #ifdef HLP_DATA_CACHE_ENABLE
    #if (HLP_DATA_CACHE_ENABLE)
        cache_settings |= CORTEX_DCACHE_ENABLED;
    #endif
    #endif
    
    #ifdef HLP_MPU_SET_WRITE_THROUGH
    #if (HLP_MPU_SET_WRITE_THROUGH)
        cache_settings |= CORTEX_CACHE_WRITE_THROUGH;
    #endif
    #endif
    
	//zynq-7000
	#ifdef APP_L1_ICACHE
	#if (APP_L1_ICACHE == 1)
        cache_settings |= CORTEX_ICACHE_ENABLED;
	#endif
    #endif

	#ifdef APP_L1_DCACHE
	#if (APP_L1_DCACHE == 1)
        cache_settings |= CORTEX_DCACHE_ENABLED;
	#endif
    #endif

	#ifdef APP_L1_CACHE_WRITE_THROUGH
	#if (APP_L1_CACHE_WRITE_THROUGH == 1)
        cache_settings |= CORTEX_CACHE_WRITE_THROUGH;
	#endif
    #endif
	
    return cache_settings;
}

