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


//卫星信息
typedef struct {
  unsigned char   idx;    //卫星编号
  unsigned char   pitch;  //卫星仰角
  unsigned short  yaw;  //卫星方位角
  unsigned char   snr;    //信噪比       
} gpssat;

//UTC时间信息
typedef struct {
  unsigned short   year;  //年份
  unsigned char    month;  //月份
  unsigned char    day;   //日期
  unsigned char    hour;   //小时
  unsigned char    min;   //分钟
  unsigned char    sec;   //秒钟
} gpstime;

//GPS协议解析后数据存放结构体
 typedef struct {
  unsigned char     svnum;          //可见GPS卫星数
  gpssat            svinf[12];    //最多12颗GPS卫星
  gpstime           utctime;      //UTC时间
  unsigned int      latitude;        //纬度 分扩大100000倍,实际要除以100000
  unsigned char     nshemi;          //北纬/南纬,N:北纬;S:南纬          
  unsigned int      longitude;          //经度 分扩大100000倍,实际要除以100000
  unsigned char     ewhemi;          //东经/西经,E:东经;W:西经
  unsigned char     status;          //GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.          
  unsigned char     posslnum;        //用于定位的GPS卫星数,0~12.
  unsigned char     possl[12];        //用于定位的卫星编号
  unsigned char     fixmode;          //定位类型:1,没有定位;2,2D定位;3,3D定位
  unsigned short    pdop;          //位置精度因子 0~500,对应实际值0~50.0
  unsigned short    hdop;          //水平精度因子 0~500,对应实际值0~50.0
  unsigned short    vdop;          //垂直精度因子 0~500,对应实际值0~50.0 
  int               altitude;         //海拔高度,放大了10倍,实际除以10.单位:0.1m   
  unsigned short    speed;          //地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时   
} GPS_Msg;



//$GPGGA,022044.00,3958.766165,N,11619.359286,E,1,08,0.9,29.6,M,-7.0,M,,*45
int GPS_GPGGA_InfoGet(GPS_Msg *gmsg, char *buf) {
    char *buffer[16] = {0};
    char *temp = NULL, *ptr = NULL;
    temp = strtok_r(buf, ",", &ptr);
    int i = 0;
    if (strstr((const char*)temp, "$GPGGA") == NULL) {
        printf("strstr == NULL\n");
        return -1;
    }

    for (i = 0; i < 12; ++i) {
        buffer[i] = strtok_r(NULL, ",", &ptr);
        printf("%s \n", buffer[i]);
    }

    {//utc time
        double d1 = strtod(buffer[0], NULL);
        int t1 = ((int )d1) % 10000;
        gmsg->utctime.hour = d1 / 10000 + 8;
        gmsg->utctime.min  = t1 / 100;
        gmsg->utctime.sec  = t1 % 100;
    }

    {// 
        gmsg->latitude = strtod(buffer[1], NULL) * 1000;
        gmsg->nshemi   = buffer[2][0];
    }

    {// 
        gmsg->longitude = strtod(buffer[3], NULL) * 1000;
        gmsg->ewhemi   = buffer[4][0];
    }

    {// 
        gmsg->status = strtol(buffer[5], NULL, 0);
    }

    {// 
        gmsg->posslnum = strtol(buffer[6], NULL, 0);
    }


    return 0;
}

//+CGPSINFO: 3958.789405,N,11619.360178,E,150922,071046.0,73.0,0.0,44.6
int GPS_CGPSINFO_Get(GPS_Msg *gmsg, char *buf) {
    char *buffer[9] = {0};
    char *temp = NULL, *ptr = NULL;
    temp = strtok_r(buf, ": ", &ptr);
    int i = 0;
    if (strstr((const char*)temp, "+CGPSINFO") == NULL) {
        printf("strstr == NULL\n");
        return -1;
    }

    for (i = 0; i < 9; ++i) {
        buffer[i] = strtok_r(NULL, ",", &ptr);
        printf("%s \n", buffer[i]);
    }

    {// 
        gmsg->latitude = strtod(buffer[0], NULL) * 1000;
        gmsg->nshemi   = buffer[1][0];
    }

    {// 
        gmsg->longitude = strtod(buffer[2], NULL) * 1000;
        gmsg->ewhemi   = buffer[3][0];
    }

    {//utc time
        long d1 = strtol(buffer[4], NULL, 0);
        long t1 = d1 / 100;
        gmsg->utctime.year  = d1 % 100 + 2000;
        gmsg->utctime.month = t1 % 100;
        gmsg->utctime.day   = t1 / 100;
    }


    {//utc time
        double d1 = strtod(buffer[5], NULL);
        int t1 = ((int )d1) % 10000;
        gmsg->utctime.hour = d1 / 10000 + 8;
        gmsg->utctime.min  = t1 / 100;
        gmsg->utctime.sec  = t1 % 100;
    }

    {// 
        gmsg->altitude = strtod(buffer[6], NULL);
    }

    gmsg->status = 1;
    return 0;
}



void GPS_printf(GPS_Msg *gmsg) {
    printf("GPS:(%d-%02d-%02d %02d:%02d:%02d) %d %c %d %c %d \n", gmsg->utctime.year, gmsg->utctime.month, gmsg->utctime.day, gmsg->utctime.hour, gmsg->utctime.min, gmsg->utctime.sec, gmsg->latitude, gmsg->nshemi, gmsg->longitude, gmsg->ewhemi, gmsg->status);
}


char *GPS_get(char *buffer, char *start, char *end, int dsz) {
    char *temp = NULL, *buf = NULL;
    int sz = 0;
    temp = strstr(buffer, start);
    if (temp == NULL)
        return NULL;
    buf = strstr(temp + 1, end);
    if (buf == NULL)
        return NULL;
    sz = buf - temp;
    if (dsz >= sz)
        return NULL;
    return temp;
}


int main(int argc, char **argv)
{
    int fd = -1, code = 0, wsz = 0, rsz = 0, type = 0;
    char buffer[1024] = {0};
    unsigned long rsize = 1022;
    char *temp, *temp1;
    GPS_Msg gmsg = {0};
    fd = uart_open(argv[1], 115200, 0, 8, 'N', 1);

    if (argc > 1) 
        type = strtol(argv[2], NULL, 0);
        
    wsz = sprintf(buffer, "AT+CVAUXS=1\r");
    code = uart_write(fd, buffer, wsz);
    sleep(1);

    wsz = sprintf(buffer, "AT+CGPSNMEA=511,3\r");
    code = uart_write(fd, buffer, wsz);
    sleep(1);

    wsz = sprintf(buffer, "AT+CBDS=1\r");
    code = uart_write(fd, buffer, wsz);
    sleep(1);

    wsz = sprintf(buffer, "AT+CGPS=1\r");
    code = uart_write(fd, buffer, wsz);
    sleep(1);

    wsz = sprintf(buffer, "AT+CGPSINFO=5\r");
    code = uart_write(fd, buffer, wsz);
    sleep(1);

    while(1) {
        rsz = 0;
        code = uart_read(fd, buffer, rsize);
        if (code <= 0) {
            sleep(3);
            continue;
        }

        printf("buffer %s\n", buffer);
        
        if (type == 0)
            temp = GPS_get(buffer, "$GPGGA", "$", 32);
        else
            temp = GPS_get(buffer, "+CGPSINFO", "$", 32);

        if (temp != NULL) {
            printf("buffer %s \ntemp %s\n", buffer, temp);
            if (type == 0)
            {
                GPS_GPGGA_InfoGet(&gmsg, temp);
            } else {
                GPS_CGPSINFO_Get(&gmsg, temp);
            }
            break;
        }

        sleep(3);
    }

    uart_close(fd);

    GPS_printf(&gmsg);

    return 0;
}
