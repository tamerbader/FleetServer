#include <LGPS.h>
#include <LWiFi.h>
#include <LAudio.h>
#include <LWiFiClient.h>
#include <LGPRS.h>
#include <LStorage.h>
#include <LSD.h>
#include <LFlash.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>
#include <ArduinoJson.h>
#include <base64.hpp>

//Start settings
#define WIFI_START 1
#define GSM_START 0
#define GPS_START 0

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
#define Drv LFlash

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
LFile myFile;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting setup...");
  pinMode(10, OUTPUT);
  Drv.begin();
  if(GPS_START){
      Serial.println("LGPS Power on, and waiting ...");
      LGPS.powerOn();
  }
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
  Serial.println("Done setup...");
  Serial.flush();
  LAudio.begin();
  delay(3000);
}

void loop() {
  Serial.println("Starting loop");
  LAudio.playFile(storageFlash, (char*)"sound.mp3");
  LAudio.setVolume(6);
  char URL[100] = "us-central1-fleet-8b5a9.cloudfunctions.net";
  char buff[256];
  int rt = 0;
  int i = 0;
  char attp_count = 0;
  myFile = Drv.open("doge.jpg", FILE_READ);
  int fileSize = myFile.size();
  unsigned char * pic = (unsigned char *) malloc(sizeof(char) * fileSize);
  for(i = 0; i < fileSize; i++){
    pic[i] = myFile.read();
  }
  myFile.close();
  unsigned char * base64_pic = (unsigned char *) malloc(sizeof(char) * fileSize*2);
  Serial.println("Starting base64 encoding..");
  int base64_length = encode_base64(pic, fileSize, base64_pic);
  free(pic);
  Drv.remove("temp_base64");
  LFile myTempFile = Drv.open("temp_base64", FILE_WRITE);
  for(i = 0; i < base64_length; i++){
    myTempFile.write(base64_pic[i]);
  }
  myTempFile.close();
  
  Serial.println("Done base64 encoding..");
  LWiFiClient client;
  Serial.println("Connecting to website");
  while (0 == client.connect(URL,80)){
    attp_count++;
    if(attp_count == CONN_ATTEMPTS){
      Serial.println("Failed to connect to website after 6 attempts");
    }
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  /*myTempFile = Drv.open("temp_base64", FILE_READ);*/
  String data = "{\"deviceId\":" + String(178967) + ", \"image\":\""+ String((char*)base64_pic) + "\"}";
  Serial.println(data.length());
  client.print("POST /uploadImage");
  client.println(" HTTP/1.1");
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + String(data.length()));
  client.print("Host: ");
  client.println(URL);
  client.print("\n" + data+"\n");
 /* char buf[2] = {0};
  for( i = 0; i < base64_length; i++){
    buf[0] = myTempFile.read();
    buf[1] = 0;
    client.print(buf);
  }
  client.println("}");
  myTempFile.close();*/
  free(base64_pic);
  int res_count = 0;
  Serial.println("waiting HTTP response:");
  while (!client.available() || res_count > 50){
    delay(100);
    res_count++;
  }
  if(res_count > 50){
    delay(5000);
    return;
  }
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("No response or invalid response!");
  }
  i = 0;
  while (client){
    int v = client.read();
    if (v != -1){
      Serial.print((char)v);
    }else{
      Serial.println("no more content, disconnect");
      client.stop();
    }
  }
  delay(10000);
}
