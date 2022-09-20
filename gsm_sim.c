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


static int parselparam_cimi(char *input, char cimi[256]) {
    char cmd[32] = {0};
    const char* temp = NULL;
    temp = strtok(input, "\r\n");
    strcpy(cmd, temp);
    printf("cmd:%s\n", cmd);
    temp = strtok(NULL, "\r\n");
    printf("temp:%s\n", temp);
    sprintf(cimi, "%s", temp);
    return 0;
}

static int parselparam(char *input, char sim[256]) {
    char cmd[32] = {0};
    char *temp = NULL;
    input = strrchr(input, '+');
    printf("temp:%s\n", input);
    temp = strtok(input, ":");
    strcpy(cmd, temp);
    printf("cmd:%s\n", cmd);
    temp = strtok(NULL, "\r\n");
    printf("temp:%s\n", temp);
    sprintf(sim, "%s", temp);
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


int gsm_getcimi(const char *devname, char cimi[256]) {
    char input[64] = {0}, output[512] = {0};
    int index = 0, code = -1, size = 510, cnt = 0;
    int gsm_fd = gsm_open(devname, O_RDWR);
    gsm_set_serial(gsm_fd, 115200, 8, 'N', 1);//9600
    
    index = sprintf(input, "%s","AT+CIMI\r");
    while(1) {
        code = gsm_write(gsm_fd, input, index);
        if (code == -1) {
            printf("send %d %d cmd:%s\n", gsm_fd, code, input);
        }
        sleep(1);

        memset(output, 0, size);
        code = gsm_read(gsm_fd, output, &size);        
        printf("gsm_recv return %d %d %s %d \n", gsm_fd, size, output, code);
        code = findCmdResponse(output, "AT+CIMI");
        printf("findCmdResponse: %d %d\n", cnt, code);
        if (code != 0) {
            if (cnt >= 3) {
                code = -3;
                break;
            }
            ++cnt;
            continue;
        }
        code = parselparam_cimi(output, cimi);
        break;
    }

    gsm_close(gsm_fd);
    return code;

}

int gsm_getccid(const char *devname, char sim[256])  {
    char input[64] = {0}, output[512] = {0};
    int index = 0, code = -1, size = 510, cnt = 0;
    int gsm_fd = gsm_open(devname, O_RDWR);
    gsm_set_serial(gsm_fd, 115200, 8, 'N', 1);//9600
    
    index = sprintf(input, "%s","AT+QCCID\r");
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
        code = findCmdResponse(output, "+QCCID:");
        printf("findCmdResponse: %d %d\n", cnt, code);
        if (code != 0) {
            if (cnt >= 3) {
                code = -3;
                break;
            }
            ++cnt;
            continue;
        }
        code = parselparam(output, sim);
        break;
    }

    gsm_close(gsm_fd);
    return code;

}

int  gsm_ccid(int gsm_fd, char ccid[256]) {
    char input[64] = {0}, output[512] = {0};
    int index = 0, code = -1, size = 510, cnt = 0;
    
    index = sprintf(input, "%s","AT+QCCID\r");
    while(1) {
        code = gsm_write(gsm_fd, input, index);
        if (code == -1) {
            printf("send %d %d cmd:%s\n", gsm_fd, code, input);
        }
        sleep(1);

        memset(output, 0, size);
        code = gsm_read(gsm_fd, output, &size);        
        printf("gsm_recv return %d %d %s %d \n", gsm_fd, size, output, code);
        code = findCmdResponse(output, "+QCCID:");
        printf("findCmdResponse: %d %d\n", cnt, code);
        if (code != 0) {
            if (cnt >= 3) {
                code = -3;
                break;
            }
            ++cnt;
            continue;
        }
        code = parselparam(output, ccid);
        break;
    }
    return code;
}