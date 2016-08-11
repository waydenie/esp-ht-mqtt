const char thingspeak_update_POST[] PROGMEM = R"=====(
POST /update HTTP/1.1
Host: %s
Connection: close
X-THINGSPEAKAPIKEY: %s
Content-Type: application/x-www-form-urlencoded
Content-Length: %d

%s
)=====";

void sendThingspeak (float temp, float humd, float ppres, float sigRH, float sigT, float dewp ) {

  char postStr[64];
  char htmlBuff[512];
//  char strbuff[STRBUFF_LEN];

  char tempstr[10];
  char humdstr[10];
  char ppresstr[10];
  char sigRHstr[10];
  char sigTstr[10];
  char dewpstr[10];

  snprintf(postStr,64,"1=%s&2=%s&3=%s&4=%s&5=%s&6=%s",
    dtostrf(temp,1,1,tempstr),
    dtostrf(humd,1,1,humdstr),
    dtostrf(ppres,1,2,ppresstr),
    dtostrf(sigRH,1,0,sigRHstr),
    dtostrf(sigT,1,0,sigTstr),
    dtostrf(dewp,1,1,dewpstr)
  );
  postStr[64-1]='\0';

  snprintf_P(htmlBuff,512,thingspeak_update_POST,
    C.thingspeak_addr,
    C.thingspeak_apiKey,
    strlen(postStr),
    postStr
  );
  htmlBuff[512-1]='\0';

  if (wifiClientTS.connect(C.thingspeak_addr,80)) {  
     wifiClientTS.print(htmlBuff);
  }
  wifiClientTS.stop();

  Serial.println("====thingspeak update====");Serial.println(htmlBuff);Serial.println("====end====");
}

