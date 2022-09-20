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

#include "location.h"
#include "gsm_usb.h"
#include "xml.h"

typedef struct svcellinfo_t {
    int mcc;     //Mobile Country Code, which is a decimal number.
    int mnc;     //Mobile Network Code, which is a decimal number.
    int lac;     //Location Area Code or Tracking Area Code, which is a hexadecimal. The value range is 0–FFFF.
    long cid;    //Cell identity in the SIB3 message, which is a hexadecimal and includes the RNC ID and cell ID. The value range is 0–FFFFFFFF.
} svcellinfo;

static int readWEBResponse(int fd, char *output, int *sz) {
    int code = 0, size = *sz, bytes = 0, rbytes = 0, rsz = *sz;
    char *rbuffer = output;
    *sz = 0;
    bytes = 0;
    code = ioctl(fd, FIONREAD, &bytes);
    printf("ioctl %d \n", bytes);
    while(rbytes < size) {
        code = read(fd, rbuffer, rsz);
        if (code <= 0) {
            break;
        }

        rbytes += code;
        rbuffer += code;
        rsz -= code;

        if (bytes <= rbytes)
            break;
    }
    //code = read(fd, output, size);
    *sz = rbytes;
    return rbytes;
}

static int getLocationbyWEB(const char *address, short port, struct svcellinfo_t *svc, char output[512]) {
    int sockfd, index = 0, code = 0, size = 510;
    struct sockaddr_in servaddr;

    char input[512] = {0};
    fd_set   rfd_set;
    struct timeval  tv;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        printf("create failed ---socket error!\n");
        return -1;
    };

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &servaddr.sin_addr) <= 0 ){
        printf("server address failed --inet_pton error!\n");
        close(sockfd);
        return -2;
    };

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        printf("connect,connect error!\n");
        close(sockfd);
        return -3;
    }
    printf("connect success\n");

    //send

    index  = sprintf(input + index, "GET /cell/?mcc=%d&mnc=%d&lac=%d&ci=%ld&output=xml HTTP/1.1\n", svc->mcc, svc->mnc, svc->lac, svc->cid);
    index += sprintf(input + index, "%s", "Host: api.cellocation.com\n");
    index += sprintf(input + index, "%s", "Content-Type: application/x-www-form-urlencoded\n");
    index += sprintf(input + index, "%s", "\n\n");

    printf("%s \n", input);

    code = write(sockfd, input, index);
    if (code < 0) {
            printf("send failed errno %d，errno string '%s'\n",errno, strerror(errno));
            close(sockfd);
            return -4;
    }else{
            printf("send success, send %d bytes！\n", code);
            usleep(100000);
    }

    while(1){
            FD_ZERO(&rfd_set);
            FD_SET(sockfd, &rfd_set);

            tv.tv_sec= 2;
            tv.tv_usec= 0;
            code = 0;
            printf("select fd\n");
            code = select(sockfd + 1, &rfd_set, NULL, NULL, &tv);
            if (code == 0) {
                printf("wait recv data \n", code);
                usleep(200000);
                continue;
            }
            printf("get read fd\n");

            if (code < 0) {
                close(sockfd);
                printf("SELECT failed！\n");
                return -5;
            };

            if (code > 0){
                memset(output, 0, 512);
                /*
                bytes = 0;
                code = ioctl(fd, FIONREAD, &bytes);
                printf("ioctl %d \n", bytes);
                index = read(sockfd, output, 512);//*/
                index = readWEBResponse(sockfd, output, &size);
                if (index == 0){
                        close(sockfd);
                        printf("read data failed！\n");
                        return -6;
                }

                printf("%s\n", output);
                break;
            }
    }
    close(sockfd);
    return 0;
}

static int getstationbyGSM(char *input, struct svcellinfo_t *svc) {
    char rat[32] = {0}, cmd[32] = {0};
    long  val[8] = {0}, i = 0, mode = 0;
    char *temp = NULL;
    input = strrchr(input, '^');
    printf("temp:%s\n", input);
    temp = strtok(input, ":");
    strcpy(cmd, temp);
    temp = strtok(NULL, ",");
    strcpy(rat, temp);
    while(1) {
        temp = strtok(NULL, ",");
        if (i == 5 || i == 6)
            mode = 16;
        else 
            mode = 0;
        val[i] = strtol(temp, NULL, mode);
        ++i;
        if (i >= 8)
            break;
    }

    printf("cmd:%s rat:%s \n", cmd, rat);
/*
    for (i = 0 ; i < 8; ++i) {
        printf("%d : %ld \n", i, val[i]);
    } //*/

    svc->mcc = val[0];
    svc->mnc = val[1];
    svc->lac = val[6];
    svc->cid = val[5];
    return 0;
}

static int getstationbyWCDMA(char *input, struct svcellinfo_t *svc) {
    char rat[32] = {0}, cmd[32] = {0};
    long  val[8] = {0}, i = 0, mode = 0;
    char *temp = NULL;
    input = strrchr(input, '^');
    printf("temp:%s\n", input);
    temp = strtok(input, ":");
    strcpy(cmd, temp);
    temp = strtok(NULL, ",");
    strcpy(rat, temp);
    while(1) {
        temp = strtok(NULL, ",");
        if (i == 4 || i == 5)
            mode = 16;
        else 
            mode = 0;
        val[i] = strtol(temp, NULL, mode);
        ++i;
        if (i >= 8)
            break;
    }

    printf("cmd:%s rat:%s \n", cmd, rat);
/*
    for (i = 0 ; i < 8; ++i) {
        printf("%d : %ld \n", i, val[i]);
    } //*/

    svc->mcc = val[0];
    svc->mnc = val[1];
    svc->lac = val[5];
    svc->cid = val[4];
    return 0;
}

static int getstationbyTDSCDMA(char *input, struct svcellinfo_t *svc) {
    char rat[32] = {0}, cmd[32] = {0};
    long  val[8] = {0}, i = 0, mode = 0;
    char *temp = NULL;
    input = strrchr(input, '^');
    printf("temp:%s\n", input);
    temp = strtok(input, ":");
    strcpy(cmd, temp);
    temp = strtok(NULL, ",");
    strcpy(rat, temp);
    while(1) {
        temp = strtok(NULL, ",");
        if (i == 5 || i == 6)
            mode = 16;
        else 
            mode = 0;
        val[i] = strtol(temp, NULL, mode);
        ++i;
        if (i >= 8)
            break;
    }

    printf("cmd:%s rat:%s \n", cmd, rat);
/*
    for (i = 0 ; i < 8; ++i) {
        printf("%d : %ld \n", i, val[i]);
    } //*/

    svc->mcc = val[0];
    svc->mnc = val[1];
    svc->lac = val[6];
    svc->cid = val[5];
    return 0;
}

static int getstationbyLTE(char *input, struct svcellinfo_t *svc) {
    char rat[32] = {0}, cmd[32] = {0};
    long  val[8] = {0}, i = 0, mode = 0;
    char *temp = NULL;    //input = strrchr(input, '^');
    input = strstr(input, "^MONSC:");
    printf("temp:%s\n", input);
    temp = strtok(input, ":");
    strcpy(cmd, temp);
    temp = strtok(NULL, ",");
    strcpy(rat, temp);
    while(1) {
        temp = strtok(NULL, ",");
        if (i == 3 || i == 5)
            mode = 16;
        else 
            mode = 0;
        val[i] = strtol(temp, NULL, mode);
        ++i;
        if (i >= 8)
            break;
    }

    printf("cmd:%s rat:%s \n", cmd, rat);
/*
    for (i = 0 ; i < 8; ++i) {
        printf("%d : %ld \n", i, val[i]);
    }//*/
    svc->mcc = val[0];
    svc->mnc = val[1];
    svc->lac = val[5];
    svc->cid = val[3];
    return 0;
}

//^MONSC: LTE,460,00,41332,789F6BB,403,11D6,-114,-6,-88
static int parselparam(char *input, struct svcellinfo_t *svc) {
    const char *RAT[] = {"GSM", "WCDMA", "TD-SCDMA", "LTE"};
    char *response = strstr(input, "^MONSC:");
    const char *temp = strchr(input, ':');
    printf("temp: %s\n", temp);
    int i = 0 , code = -1;
    for (i = 0; i < 4; ++i) {
         if (strstr(temp, RAT[i]) == NULL)
             continue;
         code = i;
         break;
    }
    printf("cmd index : %d\n", code);
    if (code < 0)
        return -1;
    switch (code)
    {
    case 0:
        return getstationbyGSM(input, svc);
    case 1:
        return getstationbyWCDMA(input, svc);
    case 2:
        return getstationbyTDSCDMA(input, svc);
    case 3:
        return getstationbyLTE(input, svc);
    default:
        break;
    }

    return -2;
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

static int getparamby4G(const char *devname, struct svcellinfo_t *svc) {
    char input[64] = {0}, output[516] = {0};
    int index = 0, code = -1, size = 512, cnt = 0;
    //int gsm_fd = gsm_open("/dev/ttyUSB1", O_RDWR);
    int gsm_fd = gsm_open(devname, O_RDWR);
    gsm_set_serial(gsm_fd, 9600, 8, 'N', 1);//9600
    
    index = sprintf(input, "%s","AT^MONSC\r");
    while(1) {
        code = gsm_write(gsm_fd, input, index);
        if (code == -1) {
            printf("send %d %d cmd:%s\n", gsm_fd, code, input);
        }
        sleep(1);

        memset(output, 0, size);
        code = gsm_read(gsm_fd, output, &size);        
        printf("gsm_recv return %d %d %s %d \n", gsm_fd, size, output, code);
        code = findCmdResponse(output, "^MONSC:");
        printf("findCmdResponse: %d %d\n", cnt, code);
        if (code != 0) {
            if (cnt >= 3) {
                code = -3;
                break;
            }
            ++cnt;
            continue;
        }
        code = parselparam(output, svc);
        break;
    }

    gsm_close(gsm_fd);
    return code;
}

static int getcontentbyname(struct xml_node *root, int index, char *output) {
    struct xml_node* node= NULL;
    struct xml_string *xname = NULL, *xcontent = NULL;
    const uint8_t *name, *content ;
    size_t length = 0;
    const char *nodename[] = {"errcode", "lat", "lon", "radius", "address"};
    node  =  xml_node_child(root, index);
    xname = xml_node_name(node);
    name = xml_string_buffer(xname);
    length = xml_string_length(xname);
    if (strncmp(name, nodename[index], length) != 0) {
        return -1;
    }
    xcontent = xml_node_content(node);
    content = xml_string_buffer(xcontent);
    length = xml_string_length(xcontent);
    strncpy(output, content, length);
    return 0;
}

static int getpostion(char *input, struct gpslocation_t *gloc) {
    char *temp = NULL, output[256] = {0};
    struct xml_document* document=NULL;
    struct xml_node* root = NULL;
    uint8_t *name, *content ;
    int code = 0;
    temp = strchr(input, '<');
    if (temp == NULL) {
        return -1;
    }
    document = xml_parse_document(temp, strlen(temp));
    if (document == NULL) {
        return -1;
    }
    root = xml_document_root(document);
    getcontentbyname(root, 0, output);
    code = strtol(output, NULL, 0);
    if (code != 0) {
       xml_document_free(document, false);
       return -2;
    }
    getcontentbyname(root, 1, output);
    gloc->lat = atof(output);
    getcontentbyname(root, 2, output);
    gloc->lon = atof(output);
    getcontentbyname(root, 3, output);
    gloc->radius = atof(output);
    getcontentbyname(root, 4, gloc->address);
    xml_document_free(document, false);

    return 0;
}

int getgpsbybasestation(const char *hostname, short port, const char *devname, struct gpslocation_t *gloc) {
    char output[512] = {0};
    {
        char address[64] = {0};
        struct svcellinfo_t svcell = {0};
        struct hostent *host = NULL;
        int  tryn = 0, code = 0;
        memset(gloc, 0, sizeof(struct gpslocation_t));
        code = getparamby4G(devname, &svcell);
        if (code < 0) {
            printf("get station param failed:%d \n", code);
            return code;
        }
        memset(output, 0, 512);
        //host = gethostbyname(hostname);
        //strncpy(address, inet_ntoa(*(struct in_addr*)host->h_addr), 16);
        while(1) {
            host = gethostbyname(hostname);
            if (host == NULL) {
                if (tryn >= 3) {
                   printf("try get ip address three times fail\n");
                   return -1;
                }
                printf("host == NULL, can not get ip address in %d time\n", tryn);
                sleep(1);
                ++tryn;
                continue;
            }

            strncpy(address, inet_ntoa(*(struct in_addr*)host->h_addr), 60); 
            printf("can get ip address %s in %d time\n", address, tryn);
            break;
        }
        printf("%s:%s %d %d %d %d %ld\n", hostname, address, port, svcell.mcc, svcell.mnc, svcell.lac, svcell.cid);
        code = getLocationbyWEB(address, port, &svcell, output);
        if (code < 0) {
            printf("get location failed code:%d from %s:%d \n", code, hostname, port);
            return code;
        }
    }
    return getpostion(output, gloc);
}