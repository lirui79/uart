all: libgsm.so testGSM

CROSS_COMPILE   = arm-linux-gnueabihf-
#CROSS_COMPILE   = arm-none-eabi-
GXX		= $(CROSS_COMPILE)g++
#GXX             = g++
GCC		= $(CROSS_COMPILE)gcc
#GCC             = gcc


libgsm.so: gsm_rssi.c  gsm_usb.c  gsm_sim.c gsm_4G.c
	$(GCC) gsm_rssi.c  gsm_usb.c  gsm_sim.c gsm_4G.c -fPIC -shared -L./ -o libgsm.so

testGSM: testGSM.c
	$(GCC) testGSM.c -L. -lgsm -o testGSM

libgsmrssi.so: gsm_rssi.c
	$(GCC) gsm_rssi.c  gsm_usb.c -fPIC -shared -L./ -o libgsmrssi.so

testGSMRssi: testGSMRSSI.c
	$(GCC) testGSMRSSI.c -L. -lgsmrssi -o testGSMRssi

libgsmsim.so: gsm_sim.c
	$(GCC) gsm_sim.c  gsm_usb.c -fPIC -shared -L./ -o libgsmsim.so

testGSMSim: testGSMSIM.c
	$(GCC) testGSMSIM.c -L. -lgsmsim -o testGSMSim

clean:
	rm *.so testGSM
