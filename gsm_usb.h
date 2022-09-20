#ifndef __4G_GSM_USB_H___
#define __4G_GSM_USB_H___


#ifdef __cplusplus
extern "C"{
#endif

int  gsm_open(const char* devname, int mode);

int  gsm_close(int fd);

int  gsm_set_serial(int fd, int speed, int bit, char event, int stop);

int  gsm_read(int fd, char *output, int *sz);

int  gsm_write(int fd, const char *input, int sz);


int  gsm_rssi(int gsm_fd, int *rssi, int *ber);

int  gsm_ccid(int gsm_fd, char ccid[256]);

int  gsm_ppp(int gsm_fd);


int  gsm_calling(int fd, const char *telcode);

int  gsm_listen(int fd);

int  gsm_hang(int fd);

// 移动： at+cgdcont=1,"ip","cmnet"
// 联通： at+cgdcont=1,"ip","3gnet"
// 电信： at+cgdcont=1,"ip","ctnet"
int  gsm_apn_set(int fd, int cid, const char *type, const char *apn);

#ifdef __cplusplus
}
#endif

#endif