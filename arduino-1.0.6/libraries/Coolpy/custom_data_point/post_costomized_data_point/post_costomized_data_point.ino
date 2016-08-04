/*
  Power By Coolpy
  Site:http://icoolpy.com
 
  This example code is in the public domain.
 */

#include <SPI.h>
#include <Ethernet.h>

byte mac[6];
//Server UrL
char server[] = "api.icoolpy.com";
//User Access Key
char ukey[] = "ec1f29bd460a4e8bb64c1e3cf7a81ddc";
//Device ID
char device[]="1";
//Sensor ID
char sensor[]="3";

EthernetClient client;

void setup() {
  Serial.begin(9600);
  mac[0]=random(1, 254);
  mac[1]=random(1, 254);
  mac[2]=random(1, 254);
  mac[3]=random(1, 254);
  mac[4]=random(1, 254);
  mac[5]=random(1, 254);
  if (Ethernet.begin(mac) == 0) {
    for(;;)
      ;
  }
  
  // give the Ethernet shield a second to initialize:
  delay(1000);
  
  char buffer[25];
  //Edit here code to your Customs must be json string
  //"{\"key\":\"mygpskey\",\"value\":{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}}"
  String json = "";
  json += String("{\"key\":\"mygpskey\",\"value\":{\"sensor\":\"gps \",\"time\":") 
       + String(1351824120) + String(",\"data\":[") + dtostrf(48.756080, 10, 6, buffer) + String(",") 
       + dtostrf(2.302038, 10, 6, buffer) + String("]}}");
  Post(json);
}

String inStr="";
boolean isOnData=false;

void loop()
{
  if (client.available()) {
    char c = client.read();
    if(c == '{'){
     inStr = "";
     isOnData = true;
     inStr += c;
    }else if(c == '}'){
      isOnData = false;
      inStr += c;
      
      ///finish read data from coolpy server
      Serial.println("");
      Serial.println(inStr); 
      
    }else if(isOnData){
      inStr += c;
    }
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();

    // do nothing forevermore:
    while(true);
  }
}

void Post(String PostData)
{
// if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    // Make a HTTP request:
    client.print("POST /v1.0/device/");
    client.print(device);
    client.print("/sensor/");
    client.print(sensor);
    client.println("/datapoints HTTP/1.1");
    client.print("Host:");
    client.println(server);
    client.println("Content-Type:application/json");
    client.print("U-ApiKey:");
    client.println(ukey);
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(PostData.length()+1);
    client.println();
    client.println(PostData);
  } 
  else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

