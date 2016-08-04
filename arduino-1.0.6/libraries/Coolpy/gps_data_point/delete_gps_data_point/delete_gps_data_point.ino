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
char sensor[]="2";
//Gen data point key
char dpkey[] = "2012-03-15T16:13:14";

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
  
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    // Make a HTTP request:
    client.print("GET /v1.0/device/");
    client.print(device);
    client.print("/sensor/");
    client.print(sensor);
    client.print("/datapoint/");
    client.print(dpkey);
    client.println("?method=delete HTTP/1.1");
    client.print("Host:");
    client.println(server);
    client.println("Accept:");
    client.print("U-ApiKey:");
    client.println(ukey);
    client.println("Connection: close");
    client.println();
  } 
  else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

String inStr="";
boolean isOnData=false;
int packer = 0;

void loop()
{
  if (client.available()) {
    char c = client.read();
    if(c == '{'){
      if(!isOnData){
         inStr = "";
         isOnData = true;
      }else{
         packer++;
      }
        inStr += c;
    }else if(c == '}'){
      inStr += c;
      if(packer>0){
        packer--; 
      }else{
       isOnData = false;
       
        ///finish read data from coolpy server
        Serial.println("");
        Serial.println(inStr);  
      
      }  
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

