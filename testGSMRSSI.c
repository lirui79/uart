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

#include "gsm_rssi.h"


//  ./testGSMRssi devname
// ./testGSMRssi  /dev/ttyUSB0
// export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
int main(int argc, const char* argv[]) {
    int rssi = 0, ber = 0;
    //int circle = atoi(argv[4]);
    if (argc < 2) {
        printf("please input: ./testGSMRssi devname \n");
        printf("example: ./testGSMRssi  /dev/ttyUSB0 \n");
        return -1;
    }

    //port = atoi(argv[2]);
    gsm_getrssi(argv[1], &rssi, &ber);
    printf("gsm:%s rssi:%d ber:%d \n", argv[1], rssi, ber);
 
    return 0;
}