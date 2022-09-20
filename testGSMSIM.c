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

#include "gsm_sim.h"


//  ./testGSMSim devname
// ./testGSMSim  /dev/ttyUSB0
// export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
int main(int argc, const char* argv[]) {
    char output[256] = {0};
    if (argc < 2) {
        printf("please input: ./testGSMSim devname \n");
        printf("example: ./testGSMSim  /dev/ttyUSB0 \n");
        return -1;
    }

    //port = atoi(argv[2]);
    gsm_getcimi(argv[1], output);
    printf("gsm:%s cimi:%s \n", argv[1], output);
    gsm_getccid(argv[1], output);
    printf("gsm:%s ccid:%s \n", argv[1], output);
 
    return 0;
}