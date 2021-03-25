# Thinkpad-Yoga-Mode-Daemon
Systemd Service and userspace daemons for automatically disabling the touchpad and keyboard on the Thinkpad Yoga 14 (20fy)

NOTE: It should theoretically be possible to tweak this to work for any lenovo 2 in 1 laptop. possibly for other brands depending on how they handle mode switch

This daemon (and userspace application), monitor a value on the machines embedded controller to detect the switch from laptop to tablet mode (and vice versa).

the userspace application does the actual disabling of the touchpad. 

the background service creates a named pipe at /var/run/CurrentMode which gets written with the current mode (Laptop or Tablet). this also allows creation of any custom functionality that just needs to read the named pipe and handle the value change.


still a work in progress so YMMV

### Instructions

1. Install the prequisites `sudo apt install libxi-dev libx11-dev`
2. Clone the repo and open the folder in a terminal
3. Create a build directory with `mkdir build`
4. Navigate into the directory `cd build`
5. Run `cmake ..` to configure the build
6. Run `make && sudo make install` to build and install