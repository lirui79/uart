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


static int findCmdResponse(char *input, const char *cmd, const char *res) {
    char *response = strstr(input, cmd);
    if (response == NULL) {
        return 1;// read again
    }

    response = strstr(response, res);
    if (response == NULL) {
        return 2;
    }
    return 0;
}

static int  gsm_cmd(int gsm_fd, const char *cmd, const char *response, const char *res) {
	char input[64] = {0}, output[512] = {0};
    int index = 0, code = -1, size = 510, cnt = 0;
    index = sprintf(input, "%s",cmd);
    while(1) {
        code = gsm_write(gsm_fd, input, index);
        if (code == -1) {
            printf("send %d %d cmd:%s\n", gsm_fd, code, input);
        }
        usleep(300000);
        size = 510;
        memset(output, 0, size);
        code = gsm_read(gsm_fd, output, &size);        
        printf("gsm_recv return %d %d %s %d \n", gsm_fd, size, output, code);
        code = findCmdResponse(output, response, res);
        printf("findCmdResponse: %d %d\n", cnt, code);
        if (code != 0) {
            if (cnt >= 3) {
                code = -3;
                break;
            }
            ++cnt;
            continue;
        }
        break;
    }
    return code;
}

int  gsm_ppp(int gsm_fd) {
	int cmdN = 9;
	const char* cmd[] = {
		"AT+CPIN?\r", 
		"AT+CREG?\r",
		"AT+CEREG?\r",
		"AT+QIACT=1\r",
		"AT+QIACT?\r",
		"AT+QIDEACT=1\r",
		"AT+CSQ\r",
		"AT+CGREG?\r",
		"AT+CGDCONT=1,\"IP\",\"CMNET\"\r",
//		"ATD*99#\r",
//		"+++\r",
	};
	const char *response[] = {
		"+CPIN:", 
		"+CREG:",
		"+CEREG:",
		"+QIACT",
		"+QIACT:",
		"+QIDEACT",
		"+CSQ:",
		"+CGREG:",
		"+CGDCONT",
//		"CONNECT",
//		"OK",
	};

	const char *res[] = {
		"OK", 
		"OK",
		"OK",
		"OK",
		"OK",
		"OK",
		"OK:",
		"OK",
		"OK",
//		"CONNECT",
//		"OK",
	};

	for (int i = 0 ; i < cmdN; ++i) {
        gsm_cmd(gsm_fd, cmd[i], response[i], res[i]);
	}
	char input[64] = {0}, output[512] = {0};
	int sz = sprintf(input, "ATD*99#\r");
	int cnt = 0, osz = 510;
	printf("input:%s %d\n", input, sz);
	gsm_write(gsm_fd, input, sz);
    do {
		sleep(1);
        osz = gsm_read(gsm_fd, output, &osz);
        printf("gsm_recv return %d %d %s\n", gsm_fd, osz, output);
        if (osz <= 0)
        	continue;
        break;
	}while(cnt++ < 5);
	return 0;
}