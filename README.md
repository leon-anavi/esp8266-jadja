ESP8266-JADJA README
======

This is a smart MQTT client for ESP8266 with the following features:
 - On the first boot an open WiFi network and HTTP server are launched. The user joins the network and 
   configures the credentials for his home Wifi network. After that ESP8266 restarts and works as MQTT client,
 - On each next boot ESP8266 tried to connect to the home WiFi network with the configured credentials and 
   to MQTT brokwer through the Internet connection,
 - Configations can be reset at any time by holding GPIO0 to low for 5 seconds.

Getting Ready
------
Please follow the step by step procedure to install Espressif SDK and required tools for building and flashing the firmware on your personal computer: https://github.com/esp8266/esp8266-wiki/wiki/Toolchain

Source Code
------

This project is based on the open source projecs: esp-httpd and esp_mqtt. This project makes use of heatshrink, which is a git submodule. To fetch the code execute:
```
git submodule init
git submodule update
```

To build the firmware execute:
```
make
```

Please note that you will need to remove the include of stdint.h from lib/heatshrink/heatshrink_decoder.h if you get the following or similar error:
```
/opt/Espressif/ESP8266_SDK/include/stdint.h:83:5: warning: "TENSILICA" is not defined [-Wundef]
```


Flash the code happens in 2 steps. On the first step flash the firmware:
```
make flash
```

On the second step build and flash the HTML:
```
make htmlflash
```

See Also
------

For more information visit: 
https://www.youtube.com/watch?v=RR0QmSZyiNk
https://www.anavi.org/article/184/



