#!/bin/bash

# This script will build ovcli, the HP OneView CLI Client for both
# Linux and MacOS X.

#   _   _ ____     ___           __     ___               
#  | | | |  _ \   / _ \ _ __   __\ \   / (_) _____      __
#  | |_| | |_) | | | | | '_ \ / _ \ \ / /| |/ _ \ \ /\ / /
#  |  _  |  __/  | |_| | | | |  __/\ V / | |  __/\ V  V / 
#  |_| |_|_|      \___/|_| |_|\___| \_/  |_|\___| \_/\_/  

OS=`uname`

echo "Detected Operating System: $OS"
if [ "$OS" == "Darwin" ]; then 
	echo "Building for OSX"
	gcc *.c ./OVUtils/*.c  -I./libjansson/ -I./librabbitmq/ -I ./OVUtils/ -lcurl ./Libraries/osx/libjansson.a ./Libraries/osx/librabbitmq.a -lcrypto -lssl -o ovcli
fi

if [ "$OS" == "Linux" ]; then
	echo "Building for Linux"
	gcc -std=c99 *.c ./OVUtils/*.c  -I./libjansson/ -I./librabbitmq/ -I ./OVUtils/ -lcurl ./Libraries/linux/libjansson.a ./Libraries/linux/librabbitmq.a -lcrypto -lssl -o ovcli
fi

if [ $? -ne 0 ]; then
	echo "Compilation has failed, please ensure that you have the following:"
	echo "	- Curl Libraries"
	echo "  - Crypto Libraries"
	echo "  - Xcode for OSX or the development toolchain for Linux"
	return -1
fi
echo "Compilation Complete, please enjoy"

