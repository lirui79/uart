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

#include "uart.h"


int uart_open(const char *devname, int speed, int flow_ctrl, int databits, char parity, int stopbits) {
	struct termios options;
	int fd = open(devname, O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd < 0) {
		printf("%s open wrong\n", devname);
		return -1;
	}

	if (tcgetattr(fd, &options) != 0) { 
		printf("%s %d tcgetattr wrong\n", devname, fd);
		close(fd);
		return -2;
	}

	memset(&options, 0, sizeof(struct termios));  
	options.c_cflag |= CLOCAL | CREAD; 
	options.c_cflag &= ~CSIZE; 

	switch(speed)
	{
		case 110:
			cfsetispeed(&options, B110);
			cfsetospeed(&options, B110);
			break;
		case 300:
			cfsetispeed(&options, B300);
			cfsetospeed(&options, B300);
			break;
		case 600:
			cfsetispeed(&options, B600);
			cfsetospeed(&options, B600);
			break;
		case 1200:
			cfsetispeed(&options, B1200);
			cfsetospeed(&options, B1200);
			break;
		case 2400:
			cfsetispeed(&options, B2400);
			cfsetospeed(&options, B2400);
			break;
		case 4800:
			cfsetispeed(&options, B4800);
			cfsetospeed(&options, B4800);
			break;
		case 9600:
			cfsetispeed(&options, B9600);
			cfsetospeed(&options, B9600);
			break;
		case 19200:
			cfsetispeed(&options, B19200);
			cfsetospeed(&options, B19200);
			break;
		case 38400:
			cfsetispeed(&options, B38400);
			cfsetospeed(&options, B38400);
			break;
		case 57600:
			cfsetispeed(&options, B57600);
			cfsetospeed(&options, B57600);
			break;
		case 115200:
			cfsetispeed(&options, B115200);
			cfsetospeed(&options, B115200);
			break;
		case 230400:
			cfsetispeed(&options, B230400);
			cfsetospeed(&options, B230400);
			break;
		default:
			cfsetispeed(&options, B9600);
			cfsetospeed(&options, B9600);
			break;
	}

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


	switch(databits)
	{
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
	}

	switch(parity)
	{
		case 'O':
		case 'o':
			options.c_cflag |= PARENB;
			options.c_cflag |= PARODD;
			options.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E':
		case 'e':
			options.c_iflag |= (INPCK | ISTRIP);
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			break;
		case 'N':
		case 'n':
			options.c_cflag &= ~PARENB;
			break;
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
		options.c_cflag &= ~CSTOPB;
		break;
	}

	options.c_cc[VTIME] = 1;
	options.c_cc[VMIN] = 128;
	tcflush(fd,TCIFLUSH);

	if((tcsetattr(fd,TCSANOW,&options))!=0) {
		printf("com set error");
		close(fd);
		return -4;
	}
	return fd;
}


int uart_write(int fd, unsigned char *data, unsigned long wsz) {
	int size = write(fd, data, wsz);
	tcflush(fd, TCOFLUSH);
	return size;
}

int uart_wflush(int fd) {
	tcflush(fd, TCOFLUSH);
	return 0;
}

int uart_rsize(int fd) {
	int  bytes = 0, code = 0;
    code = ioctl(fd, FIONREAD, &bytes);
    return bytes;
}

int uart_read(int fd, unsigned char *data, unsigned long rsz) {
	int size = read(fd, data, rsz);
//	tcflush(fd[index], TCIFLUSH);
	return size;
}

int uart_rflush(int fd) {
	tcflush(fd, TCIFLUSH);
	return 0;
}

int uart_close(int fd) {
	if (fd >= 0) {
		close(fd);
	}
	return 0;
}

