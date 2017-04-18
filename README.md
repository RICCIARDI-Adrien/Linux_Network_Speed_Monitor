# Linux Network Speed Monitor

## Description
This program displays in real-time all network interfaces reception and transmission rates.  
Here is an example output of the program :

```
Linux netspeed. (C) 2017 Adrien Ricciardi. Hit Ctrl+C to exit.
lo 	: RX = 0.0  bit/s (0.0  byte/s), TX = 0.0  bit/s (0.0  byte/s)
wlan0 	: RX = 0.0  bit/s (0.0  byte/s), TX = 0.0  bit/s (0.0  byte/s)
eth0 	: RX = 0.0  bit/s (0.0  byte/s), TX = 0.0  bit/s (0.0  byte/s)
wwan0 	: RX = 50.2 Kbit/s (6.3 Kbyte/s), TX = 88.5 Kbit/s (11.1 Kbyte/s)
```

## Installation
Build the program with the following command :
```
make
```
You can install the program in /usr/bin by typing :
```
sudo make install
```
The program can also be uninstalled :
```
sudo make uninstall
```