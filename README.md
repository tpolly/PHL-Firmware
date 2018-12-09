# Building instructions for BLE Nano 1 and 2
Note: So far, Sender has only been built on BLE Nano 2 and receiver has only been built on BLE Nano 1 (1.0 and 1.5)

Requires Arduino IDE (https://www.arduino.cc/en/Main/Software)

Caveat: Will only include build tools in 32bit.
For BLE Nano build tools:
dpkg --add-architecture i386
apt install libc6:i386 libstdc++6:i386 libudev1:i386

# Install BLE Nano boards
Inside Arduino IDE, open File -> Preferences -> Additional boards manager URLS
Enter 'https://redbear.github.io/arduino/package_redbear_nRF5x_index.json'

# Install TLC library
For original library (won't work!) Open Sketch -> Include Library... -> Manage Libraries... -> Install Adafruit TLC50711

Patched library is in repository, install or symlink to ~/Arduino/libraries/

# For Linux: Allow serial port usage

Write new udev rule for BLE Nano:

echo <<EOF
# mbed CMSIS-DAP
ATTRS{idVendor}=="0d28", ATTRS{idProduct}=="0204", MODE="664", GROUP="plugdev"
KERNEL=="hidraw*", ATTRS{idVendor}=="0d28", ATTRS{idProduct}=="0204", MODE="664", GROUP="plugdev"
EOF > /etc/udev/rules.d/98-blenano.rules

Add yourself to plugdev group if not already part of it
