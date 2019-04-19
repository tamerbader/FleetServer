static void sendPulse(struct GPSInfo * gps_info, int id, char m){
  char URL[100] = "us-central1-fleet-8b5a9.cloudfunctions.net";
  char lat_str[12] = {0};
  char long_str[12] = {0};
  parseLatLong(lat_str, long_str, gps_info); //changing gps info to strings for POST
  if( m == 'W'){
    LWiFiClient client;
    Serial.println("Connecting to website");
    while (0 == client.connect(URL,80)){
      Serial.println("Re-Connecting to WebSite");
      delay(1000);
    }
    
    String data = "{\"latitude\":"+ String(lat_str) + ", \"longitude\":" + String(long_str) +", \"deviceID\":" + String(id)+ "}";
    client.print("POST /sendPulse");
    client.println(" HTTP/1.1");
    client.println("Content-Type: application/json");
    client.println("Content-Length: " + String(data.length()));
    client.print("Host: ");
    client.println(URL);
    client.print("\n" + data);
    //client.print(char(26));
    
    Serial.println("waiting HTTP response:");
    while (!client.available()){
      delay(100);
    }
    while (client){
      int v = client.read();
      if (v != -1){
        Serial.print((char)v);
      }else{
        Serial.println("no more content, disconnect");
        client.stop();
      }
    }
  }else if(m == 'G'){
    LGPRSClient client;
    Serial.println("Connecting to website");
    while (0 == client.connect(URL,80)){
      Serial.println("Re-Connecting to WebSite");
      delay(1000);
    }
    
    String data = "{\"latitude\":"+ String(lat_str) + ", \"longitude\":" + String(long_str) +", \"deviceID\":" + String(id)+ "}";
    client.print("POST /sendPulse");
    client.println(" HTTP/1.1");
    client.println("Content-Type: application/json");
    client.println("Content-Length: " + String(data.length()));
    client.print("Host: ");
    client.println(URL);
    client.print("\n" + data);
    //client.print(char(26));
    
    Serial.println("waiting HTTP response:");
    while (!client.available()){
      delay(100);
    }
    while (client){
      int v = client.read();
      if (v != -1){
        Serial.print((char)v);
      }else{
        Serial.println("no more content, disconnect");
        client.stop();
      }
    }
    
    
  }

}
static void parseLatLong(char * lat_str, char * long_str, struct GPSInfo* gps_info){
  double lat = gps_info->latitude;
  double longitude = gps_info->longitude;
  char lat_dir = gps_info->lat_dir;
  char long_dir = gps_info->long_dir;
  double temp = fmod(lat,100.0);
  
  lat = (lat - temp)/100;
  temp = temp/60.0;
  lat = lat + temp;
  temp = fmodf(longitude,100.0);
  longitude = (longitude - temp)/100.0;
  temp = temp/60.0;
  longitude = longitude + temp;
  if(lat_dir == 'S'){
    lat = -lat;
  }
  if(long_dir == 'W'){
    longitude = -longitude;
  }
  sprintf(lat_str,"%10.6f",lat);
  sprintf(long_str,"%10.6f",longitude);
}
static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}
void parseGPGGA(const char* GPGGAstr, struct GPSInfo *gps_info){
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#GGA
   * Sample data: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   */

  int tmp,num; //, hour, minute, second, num ;
  if(GPGGAstr[0] == '$'){
//    tmp = getComma(1, GPGGAstr);
//    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
//    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
//    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');
    // get lat
    tmp = getComma(2, GPGGAstr);
    gps_info->latitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(3,GPGGAstr);
    gps_info->lat_dir = GPGGAstr[tmp];
    
    //get long
    tmp = getComma(4, GPGGAstr);
    gps_info->longitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(5,GPGGAstr);
    gps_info->long_dir = GPGGAstr[tmp];
    
    //get sat number
    tmp = getComma(7, GPGGAstr);
    gps_info->sat_num = getIntNumber(&GPGGAstr[tmp]);
    
    //get hdop
    tmp = getComma(8, GPGGAstr);
    gps_info->hdop = getDoubleNumber(&GPGGAstr[tmp]);
  }else{
    Serial.println("Did not get GPS data"); 
  }
}


