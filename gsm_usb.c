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
#include <termios.h>

#include "gsm_usb.h"

int  gsm_open(const char* devname, int mode) {
     int fd = open(devname, mode);
     if (fd < 0) {
         printf("open %s failed\n", devname);
         return -1;
     }

     printf("open %s successfully\n", devname);
     return fd;
}

int  gsm_close(int fd) {
   int code = -1;  
   code = close(fd);  
   if( code < 0 ){  
       printf("close %d fail\n", fd);
       return -1;
   }
   
   printf("close %d successfully\n", fd);
   return 0;
}

int gsm_set_serial(int fd, int speed, int bit, char event, int stop) {//设置串口参数  
    int code = 0;
    struct termios serial;
    //int speed_in;//输入波特率
    //int speed_out;//输出波特率

    code = tcgetattr(fd, &serial);//用tcgetattr函数获取串口终端参数
    if(code == -1) {
        printf("get %d serial data error\n", fd);  
    } else if (code == 0) {  
        printf("get %d serial data successfully\n", fd);  
    }  

    memset(&serial, 0, sizeof(struct termios));  
    serial.c_cflag |= CLOCAL | CREAD;//数据接收器使能和打开本地模式  
    serial.c_cflag &= ~CSIZE;//通过位掩码的方式激活  

    switch(speed) {//设置波特率  
        case 0 : cfsetispeed(&serial, B0);  
                 cfsetospeed(&serial, B0);  
                 break;  
        case 50 : cfsetispeed(&serial, B50);  
                  cfsetospeed(&serial, B50);  
                  break;  
        case 75 : cfsetispeed(&serial, B75);  
                  cfsetospeed(&serial, B75);  
                  break;  
        case 110 : cfsetispeed(&serial, B110);  
                   cfsetospeed(&serial, B110);  
                   break;  
        case 134 : cfsetispeed(&serial, B134);  
                   cfsetospeed(&serial, B134);  
                   break;  
        case 150 : cfsetispeed(&serial, B150);  
                   cfsetospeed(&serial, B150);  
                   break;  
        case 200 : cfsetispeed(&serial, B200);  
                   cfsetospeed(&serial, B200);  
                   break;  
        case 300 : cfsetispeed(&serial, B300);  
                   cfsetospeed(&serial, B300);  
                   break;  
        case 600 : cfsetispeed(&serial, B600);  
                   cfsetospeed(&serial, B600);  
                   break;  
        case 1200 : cfsetispeed(&serial, B1200);  
                    cfsetospeed(&serial, B1200);  
                    break;  
        case 1800 : cfsetispeed(&serial, B1800);  
                    cfsetospeed(&serial, B1800);  
                    break;  
        case 2400 : cfsetispeed(&serial, B2400);  
                    cfsetospeed(&serial, B2400);  
                    break;  
        case 4800 : cfsetispeed(&serial, B4800);  
                    cfsetospeed(&serial, B4800);  
                    break;  
        case 9600 : cfsetispeed(&serial, B9600);  
                    cfsetospeed(&serial, B9600);  
                    break;  
        case 19200 : cfsetispeed(&serial, B19200);  
                     cfsetospeed(&serial, B19200);  
                     break;  
        case 38400 : cfsetispeed(&serial, B38400);  
                     cfsetospeed(&serial, B38400);  
                     break;  
        case 57600 : cfsetispeed(&serial, B57600);  
                     cfsetospeed(&serial, B57600);  
                     break;  
        case 115200 : cfsetispeed(&serial, B115200);  
                      cfsetospeed(&serial, B115200);  
                      break;  
        case 230400 : cfsetispeed(&serial, B230400);  
                      cfsetospeed(&serial, B230400);  
                      break;  
    }

    switch(bit) {//设置字符大小  
        case 8 : serial.c_cflag |= CS8;//8位  
                 break;  
        case 7 : serial.c_cflag |= CS7;//7位  
                 break;  
    }

    switch(event) {//设置奇偶校验位  
        case 'O' : //奇数  
                   serial.c_cflag |= PARENB;  
                   serial.c_cflag |= PARODD;  
                   serial.c_iflag |= (INPCK | ISTRIP);  
                   break;  
        case 'E' : //偶数  
                   serial.c_iflag |= (INPCK | ISTRIP);  
                   serial.c_cflag |= PARENB;  
                   serial.c_cflag &= ~PARODD;  
                   break;  
        case 'N' : //无奇偶校验位  
                   serial.c_cflag &= ~PARENB;  
                   break;  
    }

    if(stop == 1) {//设置停止位  
        serial.c_cflag &= ~CSTOPB;//1位  
    } else if( stop == 2 ) {  
        serial.c_cflag |= CSTOPB;//两位  
    }

    serial.c_cc[VTIME] = 1;//设置等待时间，单位百毫秒，此处必须延时至少0.1秒，否则读取不到数据  
    serial.c_cc[VMIN] = 128;//设置最少接受字符，单位字节，此处最少接受字节数必须大于接接收到的数据字节数  
    tcflush(fd,TCIFLUSH);//用tcflush函数重置缓存  

    code = tcsetattr(fd, TCSANOW, &serial);//用tcsetattr函数设置串口终端参数  
    if(code == -1) {  
        printf("set %d serial data error\n", fd);  
    }else if(code == 0) {  
        printf("set %d serial data successfully\n", fd);  
        //speed_in = cfgetispeed(&serial);//获取输入波特率  
        //speed_out = cfgetospeed(&serial);//获取输出波特率  
        //printf("speed_in=%d, apeed_out=%d, fd=%d\n\n", speed_in, speed_out, fd);  
    }

    return 0;
}

int  gsm_read(int fd, char *output, int *sz) {
    fd_set readfd;
    struct timeval timeout;
    int code = 0, size = *sz, bytes = 0, rbytes = 0, rsz = *sz;
    char *rbuffer = output;
    *sz = 0;
  
    memset(output, 0, size);
    FD_ZERO(&readfd);
    FD_SET(fd, &readfd);
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
  
    code = select(fd + 1, &readfd, NULL, NULL, &timeout);
    if (code <= 0) {
        printf("select %d return code %d\n", fd, code);
        return -1;
    }

    code = -2;
    if(!FD_ISSET(fd, &readfd)){
        return code;
    }

    bytes = 0;
    code = ioctl(fd, FIONREAD, &bytes);
    printf("ioctl %d \n", bytes);
    if (bytes == 0) {
        return bytes;
    }

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

int  gsm_write(int fd, const char *input, int sz) {
    int code = write(fd, input, sz);
    if (code == -1) {
        printf("write %d data %s\n", fd, input);
    }
    return code;
}

int  gsm_calling(int fd, const char *telcode) {
    char buffer[512] = {0};
    int index = 0, code = 0, size = 512;
    index = sprintf(buffer, "ATD%s;\r", telcode);
    code = gsm_write(fd, buffer, index);
    if (code == -1) {
        printf("calling %d %s %d cmd:%s\n", fd, telcode, code, buffer);
    }
    code = gsm_read(fd, buffer, &size);
    printf("calling return %d %s %d %d\n", fd, buffer, size, code);
    return code;
}

int  gsm_listen(int fd) {
   char buffer[512] = {0};
   int index = 0, code = 0, size = 512;
   index = sprintf(buffer, "ATA\r");
   code = gsm_write(fd, buffer, index);
   if (code == -1) {
       printf("listen %d %d cmd:%s\n", fd, code, buffer);
   }
    code = gsm_read(fd, buffer, &size);
    printf("listen return %d %s %d %d\n", fd, buffer, size, code);
   return code;
}

int  gsm_hang(int fd) {
   char buffer[512] = {0};
   int index = 0, code = 0, size = 512;
   index = sprintf(buffer, "ATH\r");
   code = gsm_write(fd, buffer, index);
   if (code == -1) {
       printf("hang up %d %d cmd:%s\n", fd, code, buffer);
   }
    code = gsm_read(fd, buffer, &size);
    printf("hang up return %d %s %d %d\n", fd, buffer, size, code);
   return code;
}

// 移动： at+cgdcont=1,"ip","cmnet"
// 联通： at+cgdcont=1,"ip","3gnet"
// 电信： at+cgdcont=1,"ip","ctnet"
int  gsm_apn_set(int fd, int cid, const char *type, const char *apn) {
   char buffer[512] = {0};
   int index = 0, code = 0, size = 512;
   index = sprintf(buffer, "AT+CGDCONT=%d,\"%s\",\"%s\"\r", cid, type, apn);
   code = gsm_write(fd, buffer, index);
   if (code == -1) {
       printf("APN SET %d %d cmd:%s\n", fd, code, buffer);
   }
    code = gsm_read(fd, buffer, &size);
    printf("APN SET return %d %s %d %d\n", fd, buffer, size, code);
   return code;
}

/*
                        gsm_apn_set();//设置APN无线接入点  
                        gsm_ppp_dialing();//进行ppp拨号，实现PDP激活和TCPIP的初始化  
                        gsm_ip_open();//打开一条TCP或UDP连接  
                        gsm_iomode();//设置数据模式  
                        gsm_data_send(data[4]);//gsm数据发送  
                        gsm_ip_close();//关闭udp连接  
*/