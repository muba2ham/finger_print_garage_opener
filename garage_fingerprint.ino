#include <DFRobot_ID809.h>
#include <EEPROM.h>

#define BOOT_UNFINISHED 0
#define BOOT_FINISHED 1
#define BOOT_MEMORY_ADDRESS 0 // flag used for deleting all finger prints

#define BLINK_NONE 0 // finger print led
#define BLINK_ONCE 1 // finger print led
#define BLINK_TWICE 2 // finger print led
#define BLINK_THRICE 3 // finger print led

#define MAX_FINGER_PRINT_LIMIT 12  // you can set a limit of 80 finger prints

#define VERIFY_NUMBER 3  // the # of times to cycle through verifying finger print during registration 
#define IRQ 6  //IRQ pin 
#define FPSerial Serial

#define CAR_RELAY 2
#define STORAGE_RELAY 7
#define FINGER_PRINT_RELAY 8
#define NOT_USED_RELAY 10

byte relayPins[4] = {CAR_RELAY, STORAGE_RELAY, FINGER_PRINT_RELAY, NOT_USED_RELAY};

DFRobot_ID809 fingerprint;

long long curr = 0;
uint8_t sleepFlag = 0;

void setup(){
  // setup all relay output pins
  for(int i = 0; i < (sizeof(relayPins) / sizeof(relayPins[0])); i++){
    pinMode(relayPins[i],OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }

  /*Init print serial port */
  Serial.begin(115200,SERIAL_8N1);

  /*Init FPSerial*/
  FPSerial.begin(115200,SERIAL_8N1);

  /*Take FPSerial as communication port of the module*/
  fingerprint.begin(FPSerial);

  delay(100);  //Delay 100ms to wait for the start of the module

  /*Wait for Serial to open*/
  while(!Serial);

  /*Wait for FPSerial to open*/
  while(!FPSerial);

  /*Test whether the device can communicate properly with mainboard 
    Return true or false
    */
  while(fingerprint.isConnected() == false){
    Serial.println("Communication with device failed, please check connection");
    delay(100);
  }

  if (readFirstBoot() == BOOT_UNFINISHED) {
    Serial.println("Deleting all finger prints");
    fingerprintDeleteAll();
    writeFirstBoot(BOOT_FINISHED);
  }

  Serial.println("Module init succeeds Setup");
}

void loop(){
  delay(100);

  readData();
  fallAsleep();
}

void readData() {
  if(digitalRead(IRQ) && (sleepFlag == 0)){
    fingerprint.ctrlLED(/*LEDMode = */fingerprint.eBreathing, /*LEDColor = */fingerprint.eLEDBlue, /*blinkCount = */BLINK_NONE);

    uint16_t i = 0;
    /*Capture fingerprint image, 5s idle timeout, if timeout=0,Disable the collection timeout function
      Return 0 if succeed, otherwise return ERR_ID809
     */
    digitalWrite(FINGER_PRINT_RELAY, HIGH);
    Serial.println("Exit sleep mode");
    delay(100);  //Delay 100ms to wait for the start of the module
    sleepFlag = 1;

    while(fingerprint.isConnected() == false){
      Serial.println("Looper communication with device failed, please check connection");
      delay(100);
    }

    Serial.println("Module communication back to normal");

    if((fingerprint.collectionFingerprint(/*timeout=*/5)) != ERR_ID809){
      /*Wait for finger to relase */

      fingerprint.ctrlLED(/*LEDMode = */fingerprint.eFastBlink, /*LEDColor = */fingerprint.eLEDBlue, /*blinkCount = */BLINK_NONE);

      while(fingerprint.detectFinger()){
        delay(50);
        i++;
      }
    } else {
      Serial.println("Fingerprint ERROR");
      fingerprint.ctrlLED(/*LEDMode = */fingerprint.eFastBlink, /*LEDColor = */fingerprint.eLEDRed, /*blinkCount = */BLINK_TWICE);
      delay(100);
    }

    // you can customize logic for i, comment out i below and expand condition statement
    // find matching, register finger print, delete finger print
    i = 10;

    if(i >= 15 && i < 30){
      Serial.println();
      Serial.print("Enter fingerprint registration mode  ");
      Serial.print(fingerprint.getEnrollCount());
      Serial.print(" ");
      Serial.print(MAX_FINGER_PRINT_LIMIT);

      /*Registrate fingerprint*/
      if (fingerprint.getEnrollCount() <= MAX_FINGER_PRINT_LIMIT) {
        fingerprint.ctrlLED(/*LEDMode = */fingerprint.eFastBlink, /*LEDColor = */fingerprint.eLEDYellow, /*blinkCount = */BLINK_THRICE);
        fingerprintRegistration();
      } else {
        fingerprint.ctrlLED(/*LEDMode = */fingerprint.eKeepsOn, /*LEDColor = */fingerprint.eLEDBlue, /*blinkCount = */BLINK_NONE);
        fingerprintMatching();
      }

    }else{
      Serial.println("Enter fingerprint comparison mode");
      /*Compare fingerprints*/
      fingerprint.ctrlLED(/*LEDMode = */fingerprint.eKeepsOn, /*LEDColor = */fingerprint.eLEDBlue, /*blinkCount = */BLINK_NONE);
      fingerprintMatching();
    }

    curr = millis();

    if (digitalRead(IRQ)) { 
      sleepFlag = 0;
    }
  }
}

void fallAsleep() {
  if((sleepFlag == 1)&&(millis() - curr > 20000)){
    fingerprint.ctrlLED(/*LEDMode = */fingerprint.eNormalClose, /*LEDColor = */fingerprint.eLEDBlue, /*blinkCount = */BLINK_NONE);
    fingerprint.enterStandbyState();  //Let your module enter standby module first, then disconnect VIN power 
    digitalWrite(FINGER_PRINT_RELAY, LOW);
    sleepFlag = 0;
    Serial.println("Enter sleep mode");
  } else if ((sleepFlag == 1)&&digitalRead(IRQ)) {
    sleepFlag = 0;
    readData();
  }
}

// FUNCTION Compare fingerprints
void fingerprintMatching(){
  /*Compare the captured fingerprint with all fingerprints in the fingerprint library
    Return fingerprint ID number(1-80) if succeed, return 0 when failed
   */
  uint8_t ret = fingerprint.search();
  if(ret != 0){
    /*Set fingerprint LED ring to always ON in green*/
    Serial.print("Successfully matched,ID=");
    Serial.println(ret);

    fingerprint.ctrlLED(/*LEDMode = */fingerprint.eFastBlink, /*LEDColor = */fingerprint.eLEDGreen, /*blinkCount = */BLINK_THRICE);

    if ((ret % 2) == 0) {
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
    }
  }else{
    Serial.println("Matching failed");
    fingerprint.ctrlLED(/*LEDMode = */fingerprint.eFastBlink, /*LEDColor = */fingerprint.eLEDRed, /*blinkCount = */BLINK_THRICE);
    delay(800);
  }
  fingerprint.ctrlLED(/*LEDMode = */fingerprint.eBreathing, /*LEDColor = */fingerprint.eLEDBlue, /*blinkCount = */BLINK_NONE);
}

//FUNCTION Fingerprint Registration
void fingerprintRegistration(){
  uint8_t ID,i;
  /*Compare the captured fingerprint with all fingerprints in the fingerprint library
    Return fingerprint ID number(1-80) if succeed, return 0 when failed
    Function: clear the last captured fingerprint image
   */
  uint8_t ret = fingerprint.search();
  if(ret != 0){
    Serial.print("Duplicate fingerprint - already stored");
    fingerprint.ctrlLED(/*LEDMode = */fingerprint.eFastBlink, /*LEDColor = */fingerprint.eLEDRed, /*blinkCount = */BLINK_THRICE);
    delay(1500);
    fingerprint.ctrlLED(/*LEDMode = */fingerprint.eBreathing, /*LEDColor = */fingerprint.eLEDBlue, /*blinkCount = */BLINK_NONE);
  } else {
    /*Get a unregistered ID for saving fingerprint 
      Return ID number when succeed 
      Return ERR_ID809 if failed
    */
    if((ID = fingerprint.getEmptyID()) == ERR_ID809){
      while(1){
        delay(100);
      }
    }
    Serial.print("Unregistered ID,ID=");
    Serial.println(ID);

    i = 0;   //Clear verify times 

    /*Fingerprint Verify */
    while(i < VERIFY_NUMBER){
    
      /*Set fingerprint LED ring to breathing lighting in blue*/
      fingerprint.ctrlLED(/*LEDMode = */fingerprint.eBreathing, /*LEDColor = */fingerprint.eLEDBlue, /*blinkCount = */BLINK_NONE);
      Serial.print("The fingerprint sampling of the");
      Serial.print(i+1);
      Serial.println("(th) time is being taken");
      Serial.println("Please press down your finger");
      /*Capture fingerprint image, 10s idle timeout 
        If succeed return 0, otherwise return ERR_ID809
      */
      if((fingerprint.collectionFingerprint(/*timeout = */5)) != ERR_ID809){
        /*Set fingerprint LED ring to quick blink in yellow 3 times*/
        fingerprint.ctrlLED(/*LEDMode = */fingerprint.eFastBlink, /*LEDColor = */fingerprint.eLEDYellow, /*blinkCount = */BLINK_THRICE);
        delay(500);
        Serial.println("Capturing succeeds");
        i++;   //Sampling times +1 
      }else{
        Serial.println("Capturing fails");
        /*Get error code information*/
        //desc = fingerprint.getErrorDescription();
        //Serial.println(desc);
        /*Set fingerprint LED ring to quick blink in red 3 times*/
        fingerprint.ctrlLED(/*LEDMode = */fingerprint.eFastBlink, /*LEDColor = */fingerprint.eLEDRed, /*blinkCount = */BLINK_THRICE);
        delay(500);
      }
      Serial.println("Please release your finger");
      /*Wait for finger to release
        Return 1 when finger is detected, otherwise return 0 
      */
      fingerprint.ctrlLED(/*LEDMode = */fingerprint.eFastBlink, /*LEDColor = */fingerprint.eLEDBlue, /*blinkCount = */BLINK_NONE);
      while(fingerprint.detectFinger());
    }
  
    /*Save fingerprint information into an unregistered ID*/
    if(fingerprint.storeFingerprint(/*Empty ID = */ID) != ERR_ID809){
      Serial.print("Saving succeed ID=");
      Serial.println(ID);
    }else{
      Serial.println("Saving failed");
    }

    fingerprint.ctrlLED(/*LEDMode = */fingerprint.eBreathing, /*LEDColor = */fingerprint.eLEDBlue, /*blinkCount = */BLINK_NONE);
  }
}

// FUNCTION EEPROM Write First Boot
void writeFirstBoot(uint8_t *value) {
  EEPROM.write(BOOT_MEMORY_ADDRESS, value);
}

// FUNCTION EEPROM Read First Boot
uint8_t readFirstBoot() {
  return EEPROM.read(BOOT_MEMORY_ADDRESS);
}

// FUNCTION Delete All Fingerprints 
void fingerprintDeleteAll() {
  fingerprint.ctrlLED(/*LEDMode = */fingerprint.eBreathing, /*LEDColor = */fingerprint.eLEDRed, /*blinkCount = */BLINK_NONE);

  uint8_t _id = fingerprint.getEnrollCount();

  while (_id != 0) {
    fingerprint.delFingerprint(_id--);
  }

  delay(500);
}