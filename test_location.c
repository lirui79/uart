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

#include "location.h"
#include "gsm_usb.h"

// http://www.cellocation.com/interfac/#cell 
//   http://api.cellocation.com:82/cell/?mcc=460&mnc=1&lac=4301&ci=20986&output=xml
//  ./test_location hostname port devname
// ./test_location api.cellocation.com 82 /dev/ttyUSB1
// export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
int main(int argc, const char* argv[]) {
    short port = 0;
    struct gpslocation_t gps_loc = {0};
    int circle = atoi(argv[4]);
    if (argc < 3) {
        printf("please input: ./test_location hostname port devname \n");
        printf("example: ./test_location api.cellocation.com 82 /dev/ttyUSB1 \n");
        return -1;
    }
    for (int i = 0; i < circle; ++i) {
        printf("%d run get location\n", i);
        port = atoi(argv[2]);
        memset(&gps_loc, 0, sizeof(struct gpslocation_t));
        getgpsbybasestation(argv[1], port, argv[3], &gps_loc);
        printf("lat: %f, lon %f radius %f  address %s\n", gps_loc.lat, gps_loc.lon, gps_loc.radius, gps_loc.address);
    }
    return 0;
}