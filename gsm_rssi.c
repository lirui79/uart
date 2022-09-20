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

#include "gsm_usb.h"



static int parselparam(char *input, int *rssi, int *ber) {
    char cmd[32] = {0};
    long  val[4] = {0}, i = 0, mode = 0;
    char *temp = NULL;
    input = strrchr(input, '+');
    printf("temp:%s\n", input);
    temp = strtok(input, ":");
    strcpy(cmd, temp);
    while(1) {
        if (i < 1)
          temp = strtok(NULL, ",");
        else
          temp = strtok(NULL, " ");
        val[i] = strtol(temp, NULL, mode);
        ++i;
        if (i >= 2)
            break;
    }

    printf("cmd:%s \n", cmd);

    rssi[0] = val[0];
    ber[0]  = val[1];
    return 0;
}


static int findCmdResponse(char *input, char *cmd) {
    char *response = strstr(input, cmd);
    if (response == NULL) {
        return 1;// read again
    }

    response = strstr(response, "OK");
    if (response == NULL) {
        return 2;
    }
    return 0;
}

int gsm_getrssi(const char *devname, int *rssi, int *ber) {
    char input[64] = {0}, output[512] = {0};
    int index = 0, code = -1, size = 510, cnt = 0;
    int gsm_fd = gsm_open(devname, O_RDWR);
    gsm_set_serial(gsm_fd, 115200, 8, 'N', 1);//9600
    
    index = sprintf(input, "%s","AT+CSQ\r");
    while(1) {
        code = gsm_write(gsm_fd, input, index);
        if (code == -1) {
            printf("send %d %d cmd:%s\n", gsm_fd, code, input);
        }
        sleep(1);

        memset(output, 0, size);
        code = gsm_read(gsm_fd, output, &size);        
        printf("gsm_recv return %d %d %s %d \n", gsm_fd, size, output, code);
        code = findCmdResponse(output, "+CSQ:");
        printf("findCmdResponse: %d %d\n", cnt, code);
        if (code != 0) {
            if (cnt >= 3) {
                code = -3;
                break;
            }
            ++cnt;
            continue;
        }
        code = parselparam(output, rssi, ber);
        break;
    }

    gsm_close(gsm_fd);
    return code;
}


int gsm_rssi(int gsm_fd, int *rssi, int *ber) {
    char input[64] = {0}, output[512] = {0};
    int index = 0, code = -1, size = 510, cnt = 0;
    
    index = sprintf(input, "%s","AT+CSQ\r");
    while(1) {
        code = gsm_write(gsm_fd, input, index);
        if (code == -1) {
            printf("send %d %d cmd:%s\n", gsm_fd, code, input);
        }
        sleep(1);
        size = 510;
        memset(output, 0, size);
        code = gsm_read(gsm_fd, output, &size);        
        printf("gsm_recv return %d %d %s %d \n", gsm_fd, size, output, code);
        code = findCmdResponse(output, "+CSQ:");
        printf("findCmdResponse: %d %d\n", cnt, code);
        if (code != 0) {
            if (cnt >= 3) {
                code = -3;
                break;
            }
            ++cnt;
            continue;
        }
        code = parselparam(output, rssi, ber);
        break;
    }

    return code;
}