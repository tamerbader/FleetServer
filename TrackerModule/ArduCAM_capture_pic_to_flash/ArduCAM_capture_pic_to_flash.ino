// ArduCAM Mini demo (C)2015 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini 2MP camera, and can run on any Arduino platform.
//
// This demo was made for ArduCAM Mini OV2640 2MP Camera.
// This program requires the ArduCAM V3.4.1 (or later) library and ArduCAM Mini 2MP camera
// and use Arduino IDE 1.5.8 compiler or above

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>
#include "memorysaver.h"

#define Drv LFlash
#define FILENAME "example.jpg"
#define RESOLUTION OV2640_1600x1200

void read_fifo_burst(ArduCAM myCAM);

const int CS = 7;

int start_capturee = 1;
LFile myFile;
ArduCAM myCAM(OV2640, CS);

void setup() {
  // put your setup code here, to run once:
  uint8_t vid, pid;
  uint8_t temp;
  Wire.begin();
  Serial.begin(115200);
  while(!Serial.available()); //wait till we send something on serial to start capture
  Serial.println("ArduCAM Start!");

  // set the CS as an output:
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  
  // initialize SPI:
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV32); //IMPORTANT
  
  //Check if the ArduCAM SPI bus is OK
  delay(100);
  while(1){
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55){
      Serial.println("SPI interface Error!");
      delay(1000);continue;
    }else{
      Serial.println("SPI interface OK.");break;
    }
  }
  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);  
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26) || (pid != 0x42))
    Serial.println("Can't find OV2640 module!");
  else
    Serial.println("OV2640 detected.");

  //Change to JPEG capture mode and initialize the OV2640 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(RESOLUTION);
  delay(100);
  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES, 0x00);

  Serial.print("Initializing Drv card...");
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the Drv library functions will not work.
  pinMode(10, OUTPUT);
  Drv.begin();
  Serial.println("initialization done.");
  if (Drv.exists(FILENAME)) {
      Drv.remove(FILENAME);
  }
  myFile = Drv.open(FILENAME, FILE_WRITE);
}

void loop() {
  char temp;
  // put your main code here, to run repeatedly:
  
  if (start_capturee == 1){
    Serial.println("CAM start single shoot.");
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    myCAM.start_capture();
    start_capturee = 0;
  }
  if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)){
    Serial.println("CAM Capture Done!");
    read_fifo_burst(myCAM);
    //Clear the capture done flag
    myCAM.clear_fifo_flag();
    myFile.close();
    Serial.println("File written!");
  }
  delay(500);
}

void read_fifo_burst(ArduCAM myCAM)
{
  uint8_t temp, temp_last;
  uint32_t length = 0;
  boolean is_header = false;
  length = myCAM.read_fifo_length();
  if (length >= 393216 ) // 384kb
  {
    Serial.println("Over size.");
    return;
  }
  if (length == 0 ) //0 kb
  {
    Serial.println("Size is 0.");
    return;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode
  temp = SPI.transfer(0x00);
  length--;
  while ( length-- ){
    temp_last = temp;
    temp =  SPI.transfer(0x00);//read a byte from spi
    if (is_header == true){
      myFile.write(temp);
    }else if ((temp == 0xD8) & (temp_last == 0xFF)){
      is_header = true;
      myFile.write(temp_last);
      myFile.write(temp);
    }
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
      break;
    delayMicroseconds(12);
  }
  is_header = false;
  myCAM.CS_HIGH();
}
