#include <X11/extensions/XInput2.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ec-access.h"

int touchpad = 14;
int trackpoint = 15;
int current_yoga_mode;

static int running = 0;

Display *displayref;
Atom deviceEnabledProp;

const char enabled[1] = {1};
const char disabled[1] = {0};

void handle_signal(int sig) {
    switch(sig) {
        case SIGINT:
        case SIGTERM:
            running = 0;
            signal(SIGINT, SIG_DFL);
            signal(SIGTERM, SIG_DFL);
            XCloseDisplay(displayref);
            break;
    }
}

int init()
{
    if (ioperm(EC_DATA, 1, 1) != 0 || ioperm(EC_SC, 1, 1) != 0)
        return -1;
    return 0;
}

void wait_ec(const uint32_t port, const uint32_t flag, const char value)
{
    uint8_t data;
    int i;

    i = 0;
    data = inb(port);

    while ((((data >> flag) & 0x1) != value) && (i++ < 100))
    {
        usleep(1000);
        data = inb(port);
    }

    if (i >= 100)
    {
        syslog(LOG_ERR, "wait_ec error on port 0x%x, data=0x%x, flag=0x%x, value=0x%x\n", port, data, flag, value);
    }
}

uint8_t read_ec(const uint32_t port)
{
    uint8_t value;

    wait_ec(EC_SC, IBF, 0);
    outb(EC_SC_READ_CMD, EC_SC);
    wait_ec(EC_SC, IBF, 0);
    outb(port, EC_DATA);
    //wait_ec(EC_SC, EC_SC_IBF_FREE);
    wait_ec(EC_SC, OBF, 1);
    value = inb(EC_DATA);

    return value;
}

void write_ec(const uint32_t port, const uint8_t value)
{
    wait_ec(EC_SC, IBF, 0);
    outb(EC_SC_WRITE_CMD, EC_SC);
    wait_ec(EC_SC, IBF, 0);
    outb(port, EC_DATA);
    wait_ec(EC_SC, IBF, 0);
    outb(value, EC_DATA);
    wait_ec(EC_SC, IBF, 0);
}

void set_input_device(int mode) {
    if(displayref != NULL) {
        if(mode == LAPTOP_MODE_VALUE) {
            syslog(LOG_INFO, "Enabling Trackpoint and Touchpad");
            XIChangeProperty(displayref, touchpad, deviceEnabledProp, XA_INTEGER, 8, PropModeReplace, (unsigned char*)enabled, 1);
            XIChangeProperty(displayref, trackpoint, deviceEnabledProp, XA_INTEGER, 8, PropModeReplace, (unsigned char*)enabled, 1);
        }
        else if(mode == TABLET_MODE_VALUE) {
            syslog(LOG_INFO, "Disabling Trackpoint and Touchpad");
            XIChangeProperty(displayref, touchpad, deviceEnabledProp, XA_INTEGER, 8, PropModeReplace, (unsigned char*)disabled, 1);
            XIChangeProperty(displayref, trackpoint, deviceEnabledProp, XA_INTEGER, 8, PropModeReplace, (unsigned char*)disabled, 1);
        }
        
        XFlush(displayref);
    }
    else {
        syslog(LOG_ERR, "[ERROR]\t Display Reference or Device property is Null");
    }
}

int main(int argc, char *argv[]) {
    openlog(argv[0], LOG_PID|LOG_CONS, LOG_DAEMON);

    if (init() != 0) {
        syslog(LOG_ERR, "[ERROR]\t Failed to set permissions to access the port!\n");
        exit(1);
    }

    current_yoga_mode = read_ec(YOGA_MODE_PORT);

    if(argc > 1) {
        const char *gpuref = (argv[1] == (char*)"nvidia") ? ":1" : ":0";
        displayref = XOpenDisplay(gpuref);
    }
    else {
        syslog(LOG_INFO, "Assuming display 0, no gpu specified");
        displayref = XOpenDisplay(NULL);
    }

    deviceEnabledProp = XInternAtom(displayref, "Device Enabled", False);

    set_input_device(current_yoga_mode);
    running = 1;

    syslog(LOG_INFO, "Started Yoga Mode Detection Daemon");

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    while(running == 1) {
        if(current_yoga_mode != read_ec(YOGA_MODE_PORT)) {
            set_input_device(read_ec(YOGA_MODE_PORT));
            current_yoga_mode = read_ec(YOGA_MODE_PORT);
        }
    }

    syslog(LOG_INFO, "Stopping Yoga Mode Detection Daemon");
	return EXIT_SUCCESS;
}