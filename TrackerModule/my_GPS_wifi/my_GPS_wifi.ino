#include <LGPS.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LGPRS.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>
#include <ArduinoJson.h>

//Start connection setting
#define WIFI_START 1
#define GSM_START 0
#define COMM_MODE 'G'
//GSM settings
#define APN "TM"
#define U_NAME ""
#define P_WORD ""

//WIFI settings
#define WIFI_AP "LinkIt-ONE"
#define WIFI_PASSWORD "4-m8868O"
#define WIFI_AUTH LWIFI_WPA
#define CONN_ATTEMPTS 5

struct GPSInfo{
  double latitude;
  double longitude;
  char lat_dir;
  char long_dir;
  double hdop;
  double orient;
  int sat_num;
};
struct resParameters{
  int ping_freq;
};
char wifi_on = 0;
char gsm_on = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("LGPS Power on, and waiting ...");
  LGPS.powerOn();
  char attp_count = 0;
  if(WIFI_START){
      LWiFi.begin();
      Serial.println("Attempting to connect to WiFi");
      
      while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH,WIFI_PASSWORD))){
        Serial.println("Trying to connect to WiFi again");
        attp_count++;
        if(attp_count == 5){
          Serial.println("Failed to connect to WiFi after 6 attempts");
          wifi_on = 0;
        }
        delay(1000);
      }
      wifi_on = 1;
      attp_count = 0;
  }
  if(GSM_START){
    Serial.println("Connect to GPRS network...");
    while (!LGPRS.attachGPRS(APN,U_NAME,P_WORD)){
      Serial.println("Trying to connect to GSM again");
      attp_count++;
      if(attp_count == 5){
        Serial.println("Failed to connect to GSM after 6 attempts");
        gsm_on = 0;
      }
      delay(1000);
    }
    gsm_on = 1;
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
  int ping_freq = 0;
  if(wifi_on || gsm _on){
    Serial.println("GPS loop");
    LGPS.getData(&info);
    Serial.println((char*)info.GPGGA); 
    parseGPGGA((const char*)info.GPGGA, &gps_info);
    sendPulse(&gps_info,178967,COMM_MODE, &ping_freq);
  }else{
    Serial.println("Could not connect to any network");
  }

  delay(10000);
}
