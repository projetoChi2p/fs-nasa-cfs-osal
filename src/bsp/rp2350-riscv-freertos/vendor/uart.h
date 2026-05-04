#ifndef UART_H
#define UART_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void uart_init(void);

void uart_putc(char c);

void uart_puts(char *str);

char uart_getc(void);

uint32_t uart_qc(void);

void uart_gets(char *str);

void *uint2str(uint32_t num, char *str);

void  int2str(int32_t num, char *str);

void hex1_2str(uint8_t num,   char *str);

void hex2_2str(uint8_t num,   char *str);

void hex4_2str(uint16_t num,  char *str);

void hex8_2str(uint32_t num,  char *str);

void uart_print2hex(uint8_t num);

void uart_print4hex(uint16_t num);

void uart_print8hex(uint32_t num);

void uart_printn(int32_t num);

void uart_printun(uint32_t num);

void uart_printnl(void);

void uart_printspc(void);

int32_t  str2int(char *str);

int32_t  uart_getint(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */