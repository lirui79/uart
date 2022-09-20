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

#include "gsm_usb.h"





//  ./testGSM devname
// ./testGSM  /dev/ttyUSB1
// export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
int main(int argc, const char* argv[]) {
    char output[256] = {0};
    int rssi = 0, ber = 0, try = 0;
    if (argc < 2) {
        printf("please input: ./testGSM devname \n");
        printf("example: ./testGSM  /dev/ttyUSB1 \n");
        return -1;
    }

    int mode = 0;
    if (argc > 3)
        mode = atoi(argv[2]);

    int gsm_fd = gsm_open(argv[1], O_RDWR);
    gsm_set_serial(gsm_fd, 115200, 8, 'N', 1);//9600

    gsm_ccid(gsm_fd, output);
    printf("gsm:%s ccid:%s \n", argv[1], output);
    gsm_rssi(gsm_fd, &rssi, &ber);
    printf("gsm:%s rssi:%d ber:%d \n", argv[1], rssi, ber);

    if (mode > 0) {
        gsm_ppp(gsm_fd);
    }

    do {
        rssi = 0;
        ber = 0;
        gsm_rssi(gsm_fd, &rssi, &ber);
        printf("gsm:%s rssi:%d ber:%d \n", argv[1], rssi, ber);
        sleep(1);
    } while(try++ < 60);

    gsm_close(gsm_fd);
    return 0;
}