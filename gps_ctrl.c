#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/types.h>  
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>  
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>  
#include <netdb.h>
#include <sys/ioctl.h>



#include "uart.h"



int main(int argc, char **argv)
{
    int fd = -1, code = 0, wsz = 0,type = 1;
    char buffer[1024] = {0};
    fd = uart_open(argv[1], 115200, 0, 8, 'N', 1);

    if (argc > 2) 
        type = strtol(argv[2], NULL, 0);

    if (type == 1) {
        
        wsz = sprintf(buffer, "AT+CVAUXS=1\r");
        code = uart_write(fd, buffer, wsz);
        printf("AT+CVAUXS=1\n");
        sleep(1);

        wsz = sprintf(buffer, "AT+CGPSNMEA=511,3\r");
        code = uart_write(fd, buffer, wsz);
        printf("AT+CGPSNMEA=511,3\n");
        sleep(1);

        wsz = sprintf(buffer, "AT+CBDS=1\r");
        code = uart_write(fd, buffer, wsz);
        printf("AT+CBDS=1\n");
        sleep(1);

        wsz = sprintf(buffer, "AT+CGPS=1\r");
        code = uart_write(fd, buffer, wsz);
        printf("AT+CGPS=1\n");
        sleep(1);

        wsz = sprintf(buffer, "AT+CGPSINFO=5\r");
        code = uart_write(fd, buffer, wsz);
        printf("AT+CGPSINFO=5\n");
        sleep(1);
    } else {

        wsz = sprintf(buffer, "AT+CGPS=0\r");
        code = uart_write(fd, buffer, wsz);
        printf("AT+CGPS=0\n");
        sleep(1);

        wsz = sprintf(buffer, "AT+CVAUXS=0\r");
        code = uart_write(fd, buffer, wsz);
        printf("AT+CVAUXS=0\n");
        sleep(1);
    }

    uart_close(fd);
    return 0;
}
