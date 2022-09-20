#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <memory.h>

#include<iostream>
#include<errno.h>
#include<pthread.h>

#define UART_NUM_MAX 6
int fd[UART_NUM_MAX];
#define SEND_MSG "123456"

#define TEST_MAX_NUMBER 100

#define ISC_OK (0)

typedef struct _UART_ATTR_S
{
    unsigned int nBaudRate; //串口波特率
    unsigned int RDBlock; //串口是否阻塞， 0为非阻塞， 1为阻塞
    unsigned int nAttr; //配置校验位 ，传入0为 无校验位，8位数据位，1位停止位，禁用CTS/RTS
    unsigned int mBlockData; //阻塞模式下，多少个字节返回
    unsigned int mBlockTime; //阻塞模式下，多长时间返回
} UART_ATTR_S;

static UART_ATTR_S stUartAttr;

int Uart_setUartParms(int fd, int speed, int flow_ctrl, int databits,
		int stopbits, int parity) {
	struct termios options;
	int speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300 };
	int name_arr[] = { 115200, 19200, 9600, 4800, 2400, 1200, 300 };

	if (tcgetattr(fd, &options) != 0) {
		perror("SetupSerial 1");
		return -1;
	}
	//设置波特率
	for (int i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
		if (speed == name_arr[i]) {
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
		}
	}

	//set control model
	options.c_cflag |= CLOCAL;
	options.c_cflag |= CREAD;

	//set flow control
	switch (flow_ctrl) {

	case 0: //none
		options.c_cflag &= ~CRTSCTS;
		break;

	case 1: //use hard ware
		options.c_cflag |= CRTSCTS;
		break;
	case 2: //use sofware
		options.c_cflag |= IXON | IXOFF | IXANY;
		break;
	}

	//select data bit
	options.c_cflag &= ~CSIZE;
	switch (databits) {
	case 5:
		options.c_cflag |= CS5;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data size\n");
		return -1;
	}
	//select parity bit
	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	case 's':
	case 'S':
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return -1;
	}
	// set stopbit
	switch (stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported stop bits\n");
		return -1;
	}

	//set raw data output
	options.c_oflag &= ~OPOST;

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//options.c_lflag &= ~(ISIG | ICANON);

	//set wait time
	options.c_cc[VTIME] = 1;
	options.c_cc[VMIN] = 1;

	tcflush(fd, TCIFLUSH);

	//set the attribute to HiSerial device
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		printf("attr %d set error!\n", fd);
		return -1;
	}

	return 0;
}

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop,int nWaitMinByte,int nWaitTime)
{
	struct termios newtio,oldtio;
	if ( tcgetattr( fd,&oldtio) != 0) { 
	printf("SetupSerial 1");
	return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag |= CLOCAL | CREAD; 
	newtio.c_cflag &= ~CSIZE; 

	switch( nBits )
	{
	case 7:
	newtio.c_cflag |= CS7;
	break;
	case 8:
	newtio.c_cflag |= CS8;
	break;
	}

	switch( nEvent )
	{
	case 'O':
	newtio.c_cflag |= PARENB;
	newtio.c_cflag |= PARODD;
	newtio.c_iflag |= (INPCK | ISTRIP);
	break;
	case 'E': 
	newtio.c_iflag |= (INPCK | ISTRIP);
	newtio.c_cflag |= PARENB;
	newtio.c_cflag &= ~PARODD;
	break;
	case 'N': 
	newtio.c_cflag &= ~PARENB;
	break;
	}

	switch( nSpeed )
	{
	case 110:
		cfsetispeed(&newtio, B110);
		cfsetospeed(&newtio, B110);
		break;
	case 300:
		cfsetispeed(&newtio, B300);
		cfsetospeed(&newtio, B300);
		break;
	case 600:
		cfsetispeed(&newtio, B600);
		cfsetospeed(&newtio, B600);
		break;
	case 1200:
		cfsetispeed(&newtio, B1200);
		cfsetospeed(&newtio, B1200);
		break;
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 19200:
		cfsetispeed(&newtio, B19200);
		cfsetospeed(&newtio, B19200);
		break;
	case 38400:
		cfsetispeed(&newtio, B38400);
		cfsetospeed(&newtio, B38400);
		break;
	case 57600:
		cfsetispeed(&newtio, B57600);
		cfsetospeed(&newtio, B57600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	case 230400:
		cfsetispeed(&newtio, B230400);
		cfsetospeed(&newtio, B230400);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}
	if( nStop == 1 )
	newtio.c_cflag &= ~CSTOPB;
	else if ( nStop == 2 )
	newtio.c_cflag |= CSTOPB;
	newtio.c_cc[VTIME] = nWaitTime;
	newtio.c_cc[VMIN] = nWaitMinByte;
	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
	printf("com set error");
	return -1;
	}
	//printf("set done!\n");
	return 0;
}

int Uart_OpenUartDev(unsigned int index, UART_ATTR_S stUartAttr)
{
	if (index >= UART_NUM_MAX) {
		printf("serial %d not exist \n",index);
		return -1;
	}

	if (fd[index] <= 0) {
		char path[15];
		sprintf(path, "/dev/ttyS%d",index);
		fd[index] = open(path, O_RDWR|O_NOCTTY|O_NDELAY);
		if (fd[index] <= 0) {
			printf("open /dev/ttyS%d is failed ! \n", index);
			return -1;
		}

		if (stUartAttr.RDBlock == 0) {
			printf(">>>>%s,%s,%d\n",__FILE__,__func__,__LINE__);
			if (fcntl(fd[index], F_SETFL, FNDELAY) < 0) //非阻塞，覆盖前面open的属性
			{
				printf("fcntl failed\n");
			} else {
				//printf("fcntl=%d\n", fcntl(fd, F_SETFL, FNDELAY));
				printf(">>>>%s,%s,%d\n",__FILE__,__func__,__LINE__);
			}
		} else {
			if (fcntl(fd[index], F_SETFL, 0) < 0)  //阻塞
				printf("fcntl failed!\n");
			else
				printf("fcntl=%d\n", fcntl(fd[index], F_SETFL, 0));
		}

		int res = -1; //= Uart_setUartParms(fd[index], stUartAttr.nBaudRate, 0, 8, 1,'N');
		if((res=set_opt(fd[index],stUartAttr.nBaudRate,8,'N',1,stUartAttr.mBlockData,stUartAttr.mBlockTime))<0){
			printf("set_opt error\n");
			return -1;
		}

	}
	return ISC_OK;
}

int Uart_WriteUart(unsigned int index, unsigned char *pData,unsigned int nDataSize) {
	if (index >= UART_NUM_MAX) {
		printf("serial %d not exist \n",index);
		return -1;
	}
	int len = 0;
	len = write(fd[index], pData, nDataSize);
	tcflush(fd[index], TCOFLUSH);
	return len;
}

int Uart_WriteUart_Delay(unsigned int index, unsigned char *pData,unsigned int nDataSize,int us)
{
	if (index >= UART_NUM_MAX) {
		printf("serial %d not exist \n",index);
		return -1;
	}
	int len = 0;
	len = write(fd[index], pData, nDataSize);
	usleep(us);
	return len;
}

void Uart_WriteFlush(unsigned int index)
{
	if (index >= UART_NUM_MAX) {
		printf("serial %d not exist \n",index);
		return;
	}
	tcflush(fd[index], TCOFLUSH);
}


int Uart_ReadUart(unsigned int index, unsigned char *pData,unsigned int nDataSize)
{
	if (index >= UART_NUM_MAX) {
		printf("serial %d not exist \n");
		return -1;
	}
	int len;
	len = read(fd[index], pData, nDataSize);
//	tcflush(fd[index], TCIFLUSH);
	return len;
}

void Uart_ReadFlush(unsigned int index)
{
	if (index >= UART_NUM_MAX) {
		printf("serial %d not exist \n");
		return;
	}
	tcflush(fd[index], TCIFLUSH);
}

int Uart_CloseUartDev(unsigned int nUartDevIndex) {
	if (nUartDevIndex >= UART_NUM_MAX) {
		printf("serial %d not exist \n");
		return -1;
	}

	if(fd[nUartDevIndex] > 0){
		close(fd[nUartDevIndex]);
	    fd[nUartDevIndex] = -1;
	    return ISC_OK;
	}
	return -1;
}


void *UartWriteThread(void *arg)
{
    
	char suartNo[2];
	sprintf(suartNo,"%d",arg);
	int uartNo=atoi(suartNo);
	printf("%s %s[%d],arg=%d,uartNo=%d \n", __FILE__, __FUNCTION__, __LINE__,arg,uartNo);

	sleep(1);
	for (int i = 0; i < TEST_MAX_NUMBER; i++)
	{
		//printf("%s %s[%d] i=%d,SEND_MSG=%s\n", __FILE__, __FUNCTION__, __LINE__,i,SEND_MSG);
		Uart_OpenUartDev(uartNo, stUartAttr);
		Uart_WriteUart(uartNo, (unsigned char*) SEND_MSG, strlen(SEND_MSG));
		usleep(200*1000);
	}
}

void *UartReadThread(void *arg)
{
	char suartNo[2];
	sprintf(suartNo,"%d",arg);
	int uartNo=atoi(suartNo);
	printf("%s %s[%d],arg=%d,uartNo=%d \n", __FILE__, __FUNCTION__, __LINE__,arg,uartNo);

	sleep(1);

	char szRecv[6] = { 0 };
	Uart_OpenUartDev(uartNo, stUartAttr);
	while (true)
	{
		usleep(100*1000);
        memset(szRecv,sizeof(szRecv),0x0);
		int recvlen= Uart_ReadUart(uartNo, (unsigned char*) szRecv, sizeof(szRecv));
	    if (recvlen>0)		
		  printf("%s %s[%d] uart%d ok szRecv=%s,recvlen=%d\n", __FILE__, __FUNCTION__, __LINE__,arg, szRecv,recvlen);

	}
}

int main()
{
	stUartAttr.RDBlock = 0;
	stUartAttr.nAttr = 8;
	stUartAttr.nBaudRate = 115200;

	pthread_t nUart5WriteThread;
	pthread_t nUart5ReadThread;
	pthread_create(&nUart5WriteThread, NULL, UartWriteThread, (int *)5);
	pthread_create(&nUart5ReadThread, NULL, UartReadThread, (int *)5);
    pthread_join(nUart5WriteThread,NULL);
	pthread_join(nUart5ReadThread,NULL);	

	
}