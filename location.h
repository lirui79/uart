#ifndef __4G_LOCATION_H___
#define __4G_LOCATION_H___


#ifdef __cplusplus
extern "C"{
#endif

typedef struct gpslocation_t {
    float  lat;         //  latitude 
    float  lon;         //  longitude 
    float  radius;      //  error-circular radius
    char   address[256];//  specific address
} gpslocation;

/*  get gps location from 4g base station
 *  hostname - web server address    default - api.cellocation.com  you can see http://www.cellocation.com/interfac/#cell
 *  port     - web server port       default - 82
 *  devname  - 4g module file node   default - /dev/ttyUSB2  example:  /dev/ttyUSB0 | /dev/ttyUSB2
 *  gloc     - gps location
 *  return   -  >= 0 success  < 0  failed
 */
int getgpsbybasestation(const char *hostname, short port, const char *devname, struct gpslocation_t *gloc);

#ifdef __cplusplus
}
#endif

#endif