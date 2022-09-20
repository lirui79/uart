// example  gpsd client
// compile this way:
//    gcc example1.c -o example1 -lgps -lm
#include "gps.h"     
#include <math.h> 

#define MODE_STR_NUM 4
static char *mode_str[MODE_STR_NUM] = {
    "n/a",
    "None",
    "2D",
    "3D"
};


static void print_fix(struct gps_data_t *gpsdata)
{
    (void)printf("   <trkpt lat=\"%f\" lon=\"%f\">\n",
		 gpsdata->fix.latitude, gpsdata->fix.longitude);
    if ((isnan(gpsdata->fix.altitude) == 0))
	(void)printf("    <ele>%f</ele>\n", gpsdata->fix.altitude);
    if (gpsdata->status == STATUS_DGPS_FIX)
	(void)printf("    <fix>dgps</fix>\n");
    else
	switch (gpsdata->fix.mode) {
	case MODE_3D:
	    (void)printf("    <fix>3d</fix>\n");
	    break;
	case MODE_2D:
	    (void)printf("    <fix>2d</fix>\n");
	    break;
	case MODE_NO_FIX:
	    (void)printf("    <fix>none</fix>\n");
	    break;
	default:
	    /* don't print anything if no fix indicator */
	    break;
	}

    if ((gpsdata->fix.mode > MODE_NO_FIX) && (gpsdata->satellites_used > 0))
	(void)printf("    <sat>%d</sat>\n", gpsdata->satellites_used);
    if (isnan(gpsdata->dop.hdop) == 0)
	(void)printf("    <hdop>%.1f</hdop>\n", gpsdata->dop.hdop);
    if (isnan(gpsdata->dop.vdop) == 0)
	(void)printf("    <vdop>%.1f</vdop>\n", gpsdata->dop.vdop);
    if (isnan(gpsdata->dop.pdop) == 0)
	(void)printf("    <pdop>%.1f</pdop>\n", gpsdata->dop.pdop);

    (void)printf("   </trkpt>\n");
}


int main(int argc, char *argv[])
{
    struct gps_data_t gps_data = {0};
    printf("gpsc start\n");
    //if (0 != gps_open(GPSD_SHARED_MEMORY, NULL, &gps_data)) {
    if (gps_open("localhost", DEFAULT_GPSD_PORT, &gps_data) != 0) {
        printf("Open error.  Bye, bye\n");
        return 1;
    }

	gps_stream(&gps_data, WATCH_DEVICE | WATCH_ENABLE , "/dev/gps2");
    while (1) {
        sleep(2);
        if (-1 == gps_read(&gps_data)) {
            printf("Read error.  Bye, bye\n");
            break;
        }
    
            // Display data from the GPS receiver if valid.
        print_fix(&gps_data);
    }

    // When you are done...
    (void)gps_close(&gps_data);
    printf("gpsc end\n");
    return 0;
}

