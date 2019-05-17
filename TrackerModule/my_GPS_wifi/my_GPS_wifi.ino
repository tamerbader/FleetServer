#include <LGPS.h>
#include <LAudio.h>
#include <LBattery.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LGPRS.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>
#include <ArduinoJson.h>

//Start connection setting
#define WIFI_START 0
#define GSM_START 1
#define COMM_MODE 'G'

//GSM settings
#define APN "TM"
#define U_NAME ""
#define P_WORD ""

//WIFI settings
#define WIFI_AP "LinkIt-ONE"
#define WIFI_PASSWORD "4-m8868O"
#define WIFI_AUTH LWIFI_WPA

//other constant
#define CONN_ATTEMPTS 5
#define CONN_ERR -2
#define JSON_PARSE_ERR -3
#define RES_OK 1

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
  int alarm;
};
char wifi_on = 0;
char gsm_on = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  LAudio.begin();
  Serial.println("Playing audio...");
  LAudio.playFile(storageFlash, (char*)"boot.mp3");
  LAudio.setVolume(6);
  Serial.println("LGPS Power on, and waiting ...");
  LGPS.powerOn();
  char attp_count = 0;
  if(WIFI_START){
      LWiFi.begin();
      Serial.println("Attempting to connect to WiFi");
      
      while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH,WIFI_PASSWORD))){
        attp_count++;
        if(attp_count == CONN_ATTEMPTS){
          Serial.println("Failed to connect to WiFi after 6 attempts");
          wifi_on = 0;
          break;
        }
        Serial.println("Trying to connect to WiFi again");
        delay(1000);
      }
      wifi_on = 1;
      attp_count = 0;
  }
  if(GSM_START){
    Serial.println("Connect to GPRS network...");
    while (!LGPRS.attachGPRS(APN,U_NAME,P_WORD)){
      attp_count++;
      if(attp_count == CONN_ATTEMPTS){
        Serial.println("Failed to connect to GSM after 6 attempts");
        gsm_on = 0;
        break;
      }
      Serial.println("Trying to connect to GSM again");
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
  struct resParameters res_info; //struct to store response parameters into
  int ping_freq = 10; // ping frequency in seconds;
  int rt = 0;
  if(wifi_on || gsm_on){
    Serial.println("GPS loop");
    LGPS.getData(&info);
    Serial.println((char*)info.GPGGA); 
    parseGPGGA((const char*)info.GPGGA, &gps_info);
    rt = sendPulse(&gps_info,178967,COMM_MODE, &res_info);
    if(rt == RES_OK){
      ping_freq = res_info.ping_freq;
      Serial.print("Set ping frequency to: ");
      Serial.println(ping_freq);
      if(res_info.alarm){
        Serial.println("Playing audio...");
        LAudio.playFile(storageFlash, (char*)"sound.mp3");
        LAudio.setVolume(6);
      }
    }

  }else{
    Serial.println("Could not connect to any network");
  }

  delay(ping_freq * 1000);
}
