# SpaNet MQTT ESP32 bridge

SpaNet serial to mqtt bridge, including HomeAssitant autodiscovery.

Developed for the ESP32 Dev board but should work on any ESP32 platform. By default uses UART2 for communications with the SpaNet controller.

Discussion on this (and other) SpaNet projects can be found here https://discord.com/channels/967940763595980900/967940763595980903

## Configuration
On first boot or whenever the enable key is press the board will enter hotspot mode.  Connect to the hotspot to configure wifi & mqtt settings.  
The default SSID name for the hotspot is: eSpa
The default password for the hotspot is: eSPA-Password

The hotspot SSID name and password are configurable in the webui. The SSID name will match the spa name.

## Web-UI

Web interface is available on the devices ip address for configuration and troubleshoooting.

## Logging

Debug / log functionality is available by telneting to the device's ip address


## Circuit
To keep things as simple as possible, off the shelf modules have been used.  
NOTE: The resistors on the RX/TX pins are recommended but optional.  

<img src="circuit/circuit.png" alt="Circuit" title="Circuit"/>  

<img src="images/board.png" alt="Assembly" title="Assembly" width="33%"/>
<img src="images/disassembled.png" alt="Components" title="Components" width="33%"/>
<img src="images/wiring.png" alt="Wiring" title="Wiring" width="33%"/>

