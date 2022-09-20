#ifndef __UART_H___
#define __UART_H___


#ifdef __cplusplus
extern "C"{
#endif

int uart_open(const char *devname, int speed, int flow_ctrl, int databits, char parity, int stopbits);


int uart_write(int fd, unsigned char *data, unsigned long wsz);

int uart_wflush(int fd);

int uart_rsize(int fd);

//int uart_select(int fd, unsigned long ts);

int uart_read(int fd, unsigned char *data, unsigned long rsz);

int uart_rflush(int fd);

int uart_close(int fd);


#ifdef __cplusplus
}
#endif

#endif
