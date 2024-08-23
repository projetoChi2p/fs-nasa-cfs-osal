
#include <stdint.h>

#define PCLINUX

#ifndef __APP_HELPERS_H
#define __APP_HELPERS_H

#ifdef __cplusplus
 extern "C" {
#endif 



#define CPU_TICKS_PER_SECOND    1000000
#define TIMER_TICKS_PER_SECOND  1000000

//#define HLP_vSmallDelay  isb
//#define TICKS_PER_MILLI (TIMER_TICKS_PER_SECOND/1000)


//#define HLP_SYSTICK_RELOAD (TIMER_TICKS_PER_SECOND / 1000)
//#define XSCUTIMER_DEVICE_ID		XPAR_XSCUTIMER_0_DEVICE_ID

#ifdef PCLINUX
#define __FPU_PRESENT 1
#define __MPU_PRESENT 1
#define __ICACHE_PRESENT 1
#define __DCACHE_PRESENT 1
#define APP_L1_ICACHE 1
#define APP_L1_DCACHE 1
#endif

#ifndef __FPU_PRESENT
#define __FPU_PRESENT 0
#endif

#ifndef __MPU_PRESENT
#define __MPU_PRESENT 0
#endif

#ifndef __ICACHE_PRESENT
#define __ICACHE_PRESENT 0
#endif

#ifndef __DCACHE_PRESENT
#define __DCACHE_PRESENT 0
#endif

#ifndef __DTCM_PRESENT
#define __DTCM_PRESENT 0
#endif


#ifdef __NO_INLINE__
#define HLP_BUILD_MAY_INLINE 0
#else
#define HLP_BUILD_MAY_INLINE 1
#endif

//__OPTIMIZE__ is defined in all optimizing compilations. 
//__OPTIMIZE_SIZE__ is defined if the compiler is optimizing for size, not speed.
#ifdef __OPTIMIZE__

#ifdef __OPTIMIZE_SIZE__
#define HLP_BUILD_OPTIMIZE (8+1)
#else
#ifdef __OPTIMIZATION_LEVEL__
#define HLP_BUILD_OPTIMIZE ((__OPTIMIZATION_LEVEL__<<1)+1)
#else
#define HLP_BUILD_OPTIMIZE (1)
#endif
#endif

#else
#define HLP_BUILD_OPTIMIZE 0x0
#endif

#define HLP_BUILD_FLAGS ((HLP_BUILD_MAY_INLINE << 4)+HLP_BUILD_OPTIMIZE)





void HLP_vSystemConfig(void);
void HLP_vRtosBringUp(void);
void HLP_vBaremetalBringUp(void);

int HLP_bIsBigEndian(void);

uint32_t HLP_u32GetFlashStatus(void);
uint32_t HLP_u32GetMachineStatus(void);

uint32_t HLP_u32GetLoResTick(void);
uint32_t HLP_u32GetHiResTick(void);

uint32_t HLP_u32HiResTickSubtract(uint32_t last, uint32_t now);

void HLP_vLEDToggle(int led);

void HLP_vIncTick(void);


char* HLP_pcGenerateChecksum(char* s, char* checksum);

void HLP_vPrintHexU8(uint8_t outbuf[16], const uint8_t n);
void HLP_vPrintU8(uint8_t outbuf[16], const uint8_t n);


void HLP_vPrintHexU16(uint8_t outbuf[16], const uint16_t n);
void HLP_vPrintU16(uint8_t outbuf[16], const uint16_t n);

void HLP_vPrintU32(uint8_t outbuf[16], const uint32_t n);
void HLP_vPrintHexU32(uint8_t outbuf[16], const uint32_t n);

void HLP_vPrintFloat(uint8_t outbuf[16], float f, uint8_t integer_pad, uint8_t decimals_pad);


uint8_t HLP_u8Maj(volatile const uint8_t v1, volatile const uint8_t v2, volatile const uint8_t v3);

uint16_t HLP_u16Maj(volatile const uint16_t v1, volatile const uint16_t v2, volatile const uint16_t v3);

uint16_t HLP_u16PowerMilliVolts(void);
uint16_t HLP_u16TemperatureCelsius(void);
uint8_t HLP_u8GetCacheSettings(void);

void HLP_vSmallDelay(void);

void HLP_vConsoleInit( void );

#define HLP_vConsolePrintFormattedBspUnlocked    HLP_vConsolePrintFormatted
#define HLP_vConsolePrintFormattedBspLockUnknown HLP_vConsolePrintFormatted

#define HLP_vConsolePrintBytesBspLocked          HLP_vConsolePrintBytesStdioUnlocked
#define HLP_vConsolePrintBytesBspUnlocked        HLP_vConsolePrintBytesStdioUnlocked


void HLP_vConsolePrintFormatted( const char * fmt, ... );
void HLP_vConsolePrintBytesStdioUnlocked( const uint8_t *data, int size );
void HLP_vConsolePrintBytesBaremetal( const uint8_t *data, int size );

#ifdef __cplusplus
}
#endif

#endif /* __APP_HELPERS_H */
