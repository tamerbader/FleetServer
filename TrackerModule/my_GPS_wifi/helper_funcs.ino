static void sendUpdate(){
  
}
static int sendPulse(struct GPSInfo * gps_info, int id,struct resParameters * resInfo){
  char URL[100] = "us-central1-fleet-8b5a9.cloudfunctions.net";
  char lat_str[12] = {0};
  char long_str[12] = {0};
  char response[100] = {0};
  int i = 0;
  char attp_count = 0;
  parseLatLong(lat_str, long_str, gps_info); //changing gps info to strings for POST

  LGPRSClient client;
  client.setTimeout(10000);
  Serial.println("Connecting to website");
  while (0 == client.connect(URL,80)){
    attp_count++;
    if(attp_count == CONN_ATTEMPTS){
      Serial.println("Failed to connect to website after 6 attempts");
      return CONN_ERR;
    }
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  
  String data = "{\"latitude\":"+ String(lat_str) + ", \"longitude\":" + String(long_str) +", \"deviceID\":" + String(id)+ ", \"hdop\":" + String(gps_info->hdop)+"}";
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
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("No response or invalid response!");
  }
  while (client){
    int v = client.read();
    if (v != -1){
      response[i++] = v;
      Serial.print((char)v);
    }else{
      Serial.println("no more content, disconnect");
      client.stop();
    }
  }
  return parsePulseJsonResponse(response,resInfo);
  
  
}
static int parsePulseJsonResponse( char * response, struct resParameters * resInfo){
  const int capacity=JSON_OBJECT_SIZE(3); //change as parameters in response increase
  StaticJsonDocument<capacity> doc;
  DeserializationError err = deserializeJson(doc, response);
  int temp_ping_freq = 0;
  if(err){
    Serial.print("deserializeJson() failed with code ");
    Serial.println(err.c_str());
    return JSON_PARSE_ERR;
  }
  temp_ping_freq = doc["pingFrequency"];
  if(temp_ping_freq == 0){
    Serial.print("Failed to parse ping frequency");
    return JSON_PARSE_ERR;
  }
  resInfo->ping_freq = temp_ping_freq;
  return RES_OK;
}
