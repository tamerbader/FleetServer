//helloo
#include <LGPS.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LGPRS.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>

#define WIFI_AP "LinkIt-ONE"
#define WIFI_PASSWORD "4-m8868O"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.

gpsSentenceInfoStruct info;
char buff[256];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("LGPS Power on, and waiting ...");
  LGPS.powerOn();
  
//  LWiFi.begin();
//  Serial.println("Attempting to connect to WiFi");  
//  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH,WIFI_PASSWORD))){
//    Serial.println("Attempting to connect to WiFi");  
//    delay(1000);
//  }
  Serial.println("Attach to GPRS network...");
  while (!LGPRS.attachGPRS("TM","","")){
    delay(1000);
  }
  
  delay(3000);
}

void loop() {
  double latitude;
  double longitude;
  char lat_dir = 0;
  char long_dir = 0;
  // put your main code here, to run repeatedly:
  Serial.println("LGPS loop");
  LGPS.getData(&info);
  Serial.println((char*)info.GPGGA); 
  parseGPGGA((const char*)info.GPGGA, &latitude,&longitude, &lat_dir, &long_dir);
  sendPulse(latitude,longitude,lat_dir,long_dir,123456,'G');

//  while (!c.available()){
//    delay(100);
//  }
//  while (c){
//    int v = c.read();
//    if (v != -1){
//      Serial.print((char)v);
//    }else{
//      Serial.println("no more content, disconnect");
//    }
//  }
  delay(10000);
}
