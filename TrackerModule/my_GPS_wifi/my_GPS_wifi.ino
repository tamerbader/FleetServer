#include <LGPS.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LGPRS.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>

//Start connection setting
#define WIFI_START 1
#define GSM_START 0
#define COMM_MODE 'W'
//GSM settings
#define APN "TM"
#define U_NAME ""
#define P_WORD ""

//WIFI settings
#define WIFI_AP "LinkIt-ONE"
#define WIFI_PASSWORD "4-m8868O"
#define WIFI_AUTH LWIFI_WPA

struct GPSInfo{
  double latitude;
  double longitude;
  char lat_dir;
  char long_dir;
  double hdop;
  double orient;
  int sat_num;
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("LGPS Power on, and waiting ...");
  LGPS.powerOn();
  if(WIFI_START){
      LWiFi.begin();
      Serial.println("Attempting to connect to WiFi");  
      while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH,WIFI_PASSWORD))){
        Serial.println("Attempting to connect to WiFi");  
        delay(1000);
      }
  }
  if(GSM_START){
    Serial.println("Connect to GPRS network...");
    while (!LGPRS.attachGPRS(APN,U_NAME,P_WORD)){
      delay(1000);
    }
  }

  delay(3000);
}

void loop() {
  gpsSentenceInfoStruct info;
  char buff[256];
  double latitude;
  double longitude;
  char lat_dir = 0;
  char long_dir = 0;
  struct GPSInfo gps_info; //struct to put parsed gps info into
  
  Serial.println("LGPS loop");
  LGPS.getData(&info);
  Serial.println((char*)info.GPGGA); 
  parseGPGGA((const char*)info.GPGGA, &gps_info);
  sendPulse(&gps_info,178967,COMM_MODE);

  delay(10000);
}
