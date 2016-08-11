#include <math.h>
#include <Wire.h>
#include "SparkFunHTU21D.h"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "esp-ht-mqtt.h"

HTU21D ht;
WiFiClient wifiClient; //for MQTT
PubSubClient client(C.mqtt_server, 1883, callback, wifiClient);

WiFiClient wifiClientTS; //for ThingSpeak

void setup() {

  Serial.begin(9600);
  Serial.println("Humidity/Temp MQTT Sensor");

  Wire.begin(PIN_I2C_SDA,PIN_I2C_SCL);
  delay(10);
  ht.begin();

  Serial.print("ssid:");Serial.println(C.wifi_ssid);
  WiFi.begin(C.wifi_ssid, C.wifi_pwd);

  Serial.print("ESP-01 MAC: ");
  uint8_t mac[6];
  WiFi.macAddress(mac);
  for (int i=0; i<sizeof(mac); ++i) {
    sprintf(MAC_char,"%s%02x:",MAC_char,mac[i]);
  }
  MAC_char[strlen(MAC_char)-1]='\0';
  Serial.println(MAC_char);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Generate client name based on MAC address and last 8 bits of microsecond counter
//  sprintf(MQTTClientName,"%-17s-%02x", MAC_char, micros() & 0xff,16);
//  sprintf(MQTTClientName,"%-17s-%s", MAC_char, "xy"); //For debugging set -xx so that I'm not chasing the clientname
  sprintf(MQTTClientName,"%-17s", MAC_char);
  
  Serial.print("Connecting to ");
  Serial.print(C.mqtt_server);
  Serial.print(" as ");
  Serial.println(MQTTClientName);

  if (client.connect(MQTTClientName)) {
    Serial.println("Connected to MQTT broker");

    snprintf(MQTTTopicConfig,50,"%s/%s/config",C.mqtt_topicbase,MQTTClientName);
    MQTTTopicConfig[50]='\0';
    if (client.subscribe(MQTTTopicConfig,0)) {
      Serial.print("Subscribe to configuration topic ok: ");
      Serial.println(MQTTTopicConfig);
    } else {
      Serial.print("Subscribe to configuration topic failed: ");
      Serial.println(MQTTTopicConfig);
    }

    snprintf(MQTTTopicAnnounce,50,"%s/dev/announce",C.mqtt_topicbase);
    MQTTTopicAnnounce[49]='\0';
    if (client.publish(MQTTTopicAnnounce, MQTTClientName)) {
      Serial.println("Publish to dev/announce topic ok");
    } else {
      Serial.println("Publish to dev/announce topic failed");
    }
  } else {
    Serial.println("MQTT connect failed");
//    Serial.println("Will reset and try again...");
//    abort();
  }

  snprintf(MQTTTopicHumd,50,"%s/%s/humd",C.mqtt_topicbase,MQTTClientName);
  MQTTTopicHumd[49]='\0';
  Serial.print("MQTTTopicHumd:");  Serial.println(MQTTTopicHumd);
  snprintf(MQTTTopicTemp,50,"%s/%s/temp",C.mqtt_topicbase,MQTTClientName);
  MQTTTopicTemp[49]='\0';
  Serial.print("MQTTTopicTemp:");  Serial.println(MQTTTopicTemp);
  snprintf(MQTTTopicDewp,50,"%s/%s/dewp",C.mqtt_topicbase,MQTTClientName);
  MQTTTopicDewp[49]='\0';
  Serial.print("MQTTTopicDewp:");  Serial.println(MQTTTopicDewp);
  snprintf(MQTTTopicPPres,50,"%s/%s/ppres",C.mqtt_topicbase,MQTTClientName);
  MQTTTopicPPres[49]='\0';
  Serial.print("MQTTTopicPPres:");  Serial.println(MQTTTopicPPres);
  snprintf(MQTTTopicSigRH,50,"%s/%s/sigrh",C.mqtt_topicbase,MQTTClientName);
  MQTTTopicSigRH[49]='\0';
  Serial.print("MQTTTopicSigRH:");  Serial.println(MQTTTopicSigRH);
  snprintf(MQTTTopicSigT,50,"%s/%s/sigt",C.mqtt_topicbase,MQTTClientName);
  MQTTTopicSigT[49]='\0';
  Serial.print("MQTTTopicSigT:");  Serial.println(MQTTTopicSigT);

}

void loop() {

  client.loop();

  float humd = ht.readHumidity();
  float temp = ht.readTemperature();

  float sigRH = (humd+6)     * ( (float)65536 / 125);
  float sigT  = (temp+46.85) * ( (float)65536 / 175.72);
  
  float ppres= pow(10, (DewConstA - (DewConstB / (temp+DewConstC))));
  
  float dewp = (humd * ppres / 100);
  dewp = log10(dewp) - DewConstA;
  dewp = DewConstB / dewp;
  dewp = -(dewp + DewConstC);


sendThingspeak (temp, humd, ppres, sigRH, sigT, dewp );

  Serial.print("Time:");
  Serial.println(millis());
  
  Serial.print("    Temperature:");
  Serial.print(temp, 1);
  Serial.print("C");
  snprintf(strbuff,STRBUFF_LEN,"%s C",dtostrf(temp,5,1,strbuff)); strbuff[STRBUFF_LEN-1]='\0';
  Serial.print(" ["); Serial.print(strbuff); Serial.print("] "); 
  if (client.publish(MQTTTopicTemp, strbuff)) {
    Serial.print(" [Publish "); Serial.print(MQTTTopicTemp); Serial.print(" ok] ");
  } else {
    Serial.print(" [Publish "); Serial.print(MQTTTopicTemp); Serial.print(" failed] ");
  }
  Serial.println();

  Serial.print("    Humidity:");
  Serial.print(humd, 1);
  Serial.print("%");
  snprintf(strbuff,STRBUFF_LEN,"%s \%RH",dtostrf(humd,4,1,strbuff)); strbuff[STRBUFF_LEN-1]='\0';
  Serial.print(" ["); Serial.print(strbuff); Serial.print("] "); 
  if (client.publish(MQTTTopicHumd, strbuff)) {
    Serial.print(" [Publish "); Serial.print(MQTTTopicHumd); Serial.print(" ok] ");
  } else {
    Serial.print(" [Publish "); Serial.print(MQTTTopicHumd); Serial.print(" failed] ");
  }
  Serial.println();

  Serial.print("    PPressure:");
  Serial.print(ppres, 2);
  Serial.print("mmHg");
  snprintf(strbuff,STRBUFF_LEN,"%s mmHg",dtostrf(ppres,5,2,strbuff)); strbuff[STRBUFF_LEN-1]='\0';
  Serial.print(" ["); Serial.print(strbuff); Serial.print("] "); 
  if (client.publish(MQTTTopicPPres, strbuff)) {
    Serial.print(" [Publish "); Serial.print(MQTTTopicPPres); Serial.print(" ok] ");
  } else {
    Serial.print(" [Publish "); Serial.print(MQTTTopicPPres); Serial.print(" failed] ");
  }
  Serial.println();

  Serial.print("    Signal (Raw) Relative Humidity:");
  Serial.print(sigRH, 0);
  Serial.print("");
  snprintf(strbuff,STRBUFF_LEN,"%s",dtostrf(sigRH,6,0,strbuff)); strbuff[STRBUFF_LEN-1]='\0';
  Serial.print(" ["); Serial.print(strbuff); Serial.print("] "); 
  if (client.publish(MQTTTopicSigRH, strbuff)) {
    Serial.print(" [Publish "); Serial.print(MQTTTopicSigRH); Serial.print(" ok] ");
  } else {
    Serial.print(" [Publish "); Serial.print(MQTTTopicSigRH); Serial.print(" failed] ");
  }
  Serial.println();

  Serial.print("    Signal (Raw) Temperature:");
  Serial.print(sigT, 0);
  Serial.print("");
  snprintf(strbuff,STRBUFF_LEN,"%s",dtostrf(sigT,6,0,strbuff)); strbuff[STRBUFF_LEN-1]='\0';
  Serial.print(" ["); Serial.print(strbuff); Serial.print("] "); 
  if (client.publish(MQTTTopicSigT, strbuff)) {
    Serial.print(" [Publish "); Serial.print(MQTTTopicSigT); Serial.print(" ok] ");
  } else {
    Serial.print(" [Publish "); Serial.print(MQTTTopicSigT); Serial.print(" failed] ");
  }
  Serial.println();
    
  Serial.print("    Dew Point:");
  Serial.print(dewp, 1);
  Serial.print("C");
  snprintf(strbuff,STRBUFF_LEN,"%s C",dtostrf(dewp,5,1,strbuff)); strbuff[STRBUFF_LEN-1]='\0';
  Serial.print(" ["); Serial.print(strbuff); Serial.print("] "); 
  if (client.publish(MQTTTopicDewp, strbuff)) {
    Serial.print(" [Publish "); Serial.print(MQTTTopicDewp); Serial.print(" ok] ");
  } else {
    Serial.print(" [Publish "); Serial.print(MQTTTopicDewp); Serial.print(" failed] ");
  }
  Serial.println(); Serial.println();

  delay(C.sensor_period);
}
