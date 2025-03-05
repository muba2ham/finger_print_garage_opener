### Capacitive Fingerprint Sensor Garage Door Opener

Equipment:

1. Arduino Uno Rev3
2. Relay Shield (1 relay for Capacitive Fingerprint Sensor - low power mode, (n)th relay depending on number of garage doors) - [Link to relay shield used in project](https://www.dfrobot.com/product-496.html?srsltid=AfmBOoppcom_Iqqaia1uMwy3AK4ZsfnLBc_QPRHhg9jxC8jAdyBSBhXW)
3. Capacitive Fingerprint Sensor - [Link to finger print sensor used in project](https://wiki.dfrobot.com/Capacitive_Fingerprint_Sensor_SKU_SEN0348)
4. Garage door opener - should already be installed for this project to be functional
5. Few jumper wires and wire long enough to run from finger print sensor to arduino and ardunio to both garage door opener terminals
6. Arduino Uno 5v Power Supply

**Note**

*You will need to determine what signal input the garage opener requires to lift up/down the door, you can use a multimeter to test the voltage while pressing down on your garage opener remote. Most openers only require a forced short circuit, take a jumper wire and put it into both the garage opener terminals (NO/COM) for a second, doors should start moving (up/down) - this project is based on this model, you can customize as per your own needs the signal required.*

LED Customization:

Parameter 1:<LEDMode>
eBreathing   eFastBlink   eKeepsOn    eNormalClose
eFadeIn      eFadeOut     eSlowBlink

Paramerer 2:<LEDColor>
eLEDGreen  eLEDRed      eLEDYellow   eLEDBlue
eLEDCyan   eLEDMagenta  eLEDWhite

Parameter 3:<number of blinks> 0 represents blinking all the time
Max 3 blinks - 0, 1, 2, 3
This parameter will only be valid in mode eBreathing, eFastBlink, eSlowBlink

*For security purposes while device is used in production env methods such as fingerprint delete and fingerprint registration aren't accessible. We can force to register, by setting i to meet the following condition `(i >= 15 && i < 30)`, otherwise set i to anything else to force always matching mode. To delete a single finger print you can customize the fingerprintDeleteAll() function to delete by a single ID. To delete all finger prints, reset Ardunio EEPROM memory banks by running the following project below - it will reset Arduino memory banks and then install garage_fingerprint.ino on Arduino. This will force delete during setup.*

> #include <EEPROM.h>
> 
> void setup() {
>   Serial.begin(115200);
>   while (!Serial);
> 
>   for (int i = 0; i < EEPROM.length(); i++) {
>     EEPROM.write(i, 0);
>     Serial.print("Updating memory block ");
>     Serial.print(i);
>     Serial.println("");
>   }
> 
>   Serial.println("Finished resetting memory");
> }
> 
> void loop() {
> }

Max finger prints 80, customize as per how many you need
`#define MAX_FINGER_PRINT_LIMIT 12`

Define your own logic on how you want finger prints to interface with garage doors
`   if ((ret % 2) == 0) {
      // even
      digitalWrite(STORAGE_RELAY, HIGH);
      delay(200);
      digitalWrite(STORAGE_RELAY, LOW);
      delay(3000);
    }else {
      // odd
      digitalWrite(CAR_RELAY, HIGH);
      delay(200);
      digitalWrite(CAR_RELAY, LOW);
      delay(3000);
    }`

I've stored 2 finger prints from each hand as even/odd IDs (4 total per person - 2 even IDs/2 odd IDs) that control either garage door 1 or garage door 2, customize the above as per your needs, if you have 1 garage door or even 3 etc.

**In order to install the garage_fingerprint.ino firmware on Arduino Uno, you need to remove the TX (1)/RX (0) pins, once installed, you can insert pins back. Uno uses the connection once connected via usb to IDE. Due to `SoftwareSerial` 115200 Baud rate issues on Uno, the Hardware Serial provided by Uno supports the 115200 Baud rate.**

Wire Layout

**Capacitive Fingerprint Sensor  ->  Arduino Uno**
- GND  ->  GND
* RX  ->  Digital Pin 0
+ TX  ->  Digital Pin 1
- IRQ  ->  Digital Pin 6

**Capacitive Fingerprint Sensor  ->  Relay Shield**
- VIN   ->  Relay Shield C0M3
+ 3.3V Power Supply  ->  Relay Shield N03

**Arduino Uno  ->  Relay Shield**
- 3.3V Power Supply  ->  Relay Shield COM3

**Relay Shield  ->  Garage Door (n)th Terminal**
- Relay Shield N0 (n)th  ->  Relay N0 Input
+ Relay Shield COM (n)th  ->  Relay COM Input

*example of capacitive fingerprint sensor **without** relay shield*
![capacitive fingerprint sensor without relay shield one](/example_images/uno_pic_one.jpg)
![capacitive fingerprint sensor without relay shield two](/example_images/uno_pic_two.jpg)

*example of capacitive fingerprint sensor **with** relay shield*
![capacitive fingerprint sensor with relay shield](/example_images/relay_shield.jpg)

*example of garage door opener terminals*
![garage door opener terminals](/example_images/garage_terminal.jpg)

*The project can easily be scaled to support further functionality like wireless capabilities, connecting a 9v battery to the finger print sensor and a wifi or bluetooth module. The relay shield module supports Xbee modules like wifi, bluetooth, they can be controlled via IoT devices or mobile devices/apps*
