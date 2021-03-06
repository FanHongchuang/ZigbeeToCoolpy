/*
  Author:Frank
  2015-02-09T23:25
  Modified by Frank
  achieve the " InQueue " operate in " HardwareSerial.cpp "
  2015-02-20T16:30:12
 */

#include <SPI.h>
#include <Ethernet.h>

#include "CycQueue.h"

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; the mac be use to generate ip address

IPAddress ip(192,168,1,160); // if DHCP is Failed, use this ip address

//Server UrL
char server[] = "192.168.1.110"; // the ip address of host computer which runing coolpy
//Server port
int port = 1337; // the coolpy port
//User Access Key
char ukey[] = "b4876754-a930-4c10-9d51-d524676b4d8f";  // my key
//Device ID
char device[]="1";
//Sensor ID
byte sensor=2;

EthernetClient client;
//String json = "";
//double randNumberFloat;
//long randNumber;
char buffer[25];  
float temperature;     //store the temperature value
float humidity;       //store the humidity value
String jsonT,jsonH;   //temperature and humidity( String format ) for "POST"


CycQueue *q;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
    Serial.println(Ethernet.localIP());
  }
  //randomSeed(analogRead(0));
  // give the Ethernet shield a second to initialize:
  delay(1000);
  
  q = CycQueueInit();
  //pinMode(5, OUTPUT);
}

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
unsigned long getLastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 1000; // delay between 2 datapoints, 1s, post
const unsigned long getingInterval = 100; // delay between 2 datapoints, 100ms, get

DATA *d;    //for quit queue
int i = 0;  //for "Serial"( each 5 characters into queue )
DATA data;  //for into queue

String returnValue = ""; //store the HTTP return value

boolean flag = false;//for "POST" ( POST(Node_T), then POST(Node_H) )

boolean ResponseBegin = false; //for "GET" return value

byte sw_flag = 1; //for "GET"( GET(16),then GET(17),then GET(18) ... )
byte ch;
void loop()
{ 
  /*if(Serial1.available() > 0)
  {
    data.buf[i++] = ch = Serial1.read();
    Serial.println(ch);
    if(i == 5)
    {
      CycQueueIn(q,data);  //into queue
      i=0;
    }
  }*/
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    //Serial.print(c);
    if (c == '{')
        ResponseBegin = true;
      else if (c == '}')
        ResponseBegin = false;

      if (ResponseBegin)
        returnValue += c; 
  }
if (returnValue.length() !=0 && (ResponseBegin == false))
  {
    //Serial.println(returnValue);
    if(returnValue.charAt(returnValue.length() - 19) == '1' || returnValue.charAt(returnValue.length() - 20) == '1' ) // "- 19"  or "-20" is the return value which represent switch status
    {
     switch(returnValue.charAt(returnValue.length() - 10)) //"-10" is the number of node
     {
       case  '6': //16
         Serial1.write("11");//'1' represent node, '1' represent turn on led
       break;
       
        case  '7'://17
         Serial1.write("21");//'2' represent node, '1' represent turn on led
       break;
       
       case  '8'://18
         Serial1.write("31");//'3' represent node, '1' represent turn on led
       break;
       
       case  '9'://19
         Serial1.write("41");//'4' represent node, '1' represent turn on led
       break;
       
       case  '0'://20
         Serial1.write("51");//'5' represent node, '1' represent turn on led
       break;
     }
    }else if(returnValue.charAt(returnValue.length() - 19) == '0' || returnValue.charAt(returnValue.length() - 20) == '0' ){
      switch(returnValue.charAt(returnValue.length() - 10))
     {
       case  '6':
         Serial1.write("10");//'1' represent node, '0' represent turn off led
       break;
       
        case  '7':
         Serial1.write("20");//'2' represent node, '0' represent turn off led
       break;
       
       case  '8':
         Serial1.write("30");//'3' represent node, '0' represent turn off led
       break;
       
       case  '9':
         Serial1.write("40");//'4' represent node, '0' represent turn off led
       break;
       
       case  '0':
         Serial1.write("50");//'5' represent node, '0' represent turn off led
       break;
     }
    }
    
     returnValue = "";  //clear the HTTP return value
  }
  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    //Serial.println();
    //Serial.println("disconnecting.");
    client.stop();
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if( !client.connected() && (millis() - lastConnectionTime > postingInterval) ) {
    Serial.println(CycQueueLen(q),HEX);
    if(flag = !flag) //out queue every twice
    {
      d = CycQueueOut(q); 
      if( d == NULL )  //if the queue is empty, return.(but this will result in the number of "GET" cut in half)
      {
        Serial.println("queue is empty return!");
        lastConnected = client.connected();
        return;
      }
    Serial.println(CycQueueLen(q),HEX);
    
    Serial.print(d->buf[0]);
    Serial.print(d->buf[1],HEX);
    Serial.print(d->buf[2],HEX);
    Serial.print(d->buf[3],HEX);
    Serial.println(d->buf[4],HEX);
    
    temperature = (d->buf[1])*10 + (d->buf[2]);
    //Serial.println(temperature);
    jsonT = String("{\"value\":") + dtostrf(temperature, 5, 2, buffer) + String("}") ;
    Serial.println(jsonT);
    
    humidity = (d->buf[3])*10 + (d->buf[4]);
    //Serial.println(humidity);
    jsonH = String("{\"value\":") + dtostrf(humidity, 5, 2, buffer) + String("}") ;
    Serial.println(jsonH);
    }
     
    switch (d->buf[0])
    {
      case '1':
        Serial.println("Node1");
        if(flag)
        {
          Post(jsonT,2);
        }else{
          Post(jsonH,3);
        }
      break;
      
      case '2':
      Serial.println("Node2");
      if(flag)
        {
          Post(jsonT,4);
        }else{
          Post(jsonH,5);
        }
      break;
      
      case '3':
        Serial.println("Node3");
        if(flag)
        {
          Post(jsonT,6);
        }else{
          Post(jsonH,7);
        }
      break;
      
      case '4':
        Serial.println("Node4");
        if(flag)
        {
          Post(jsonT,12);
        }else{
          Post(jsonH,13);
        }
      break;
      
      case '5':
        Serial.println("Node5");
        if(flag)
        {
          Post(jsonT,14);
        }else{
          Post(jsonH,15);
        }
      break;
    }
    }
  // store the state of the connection for next time through
  // the loop:
  if(!client.connected() && (millis() - getLastConnectionTime > getingInterval)) {
   if(sw_flag == 1){
     Get(16);
     sw_flag += 1;
   }else if(sw_flag == 2){
     Get(17);
     sw_flag += 1;
   }else if(sw_flag == 3){
     Get(18);
     sw_flag += 1;
   }else if(sw_flag == 4){
     Get(19);
     sw_flag += 1;
   }else if(sw_flag == 5){
     Get(20);
     sw_flag = 1;
   }
  }
  
  lastConnected = client.connected();
}

void Post(String PostData,byte sensorID)
{
// if you get a connection, report back via serial:
  if (client.connect(server, port)) {
    //Serial.println("post connection success.");
    // Make a HTTP request:
    client.print("POST /v1.0/device/");
    client.print(device);
    client.print("/sensor/");
    client.print(sensorID);
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
     //If you didn't get a connection to the server:
    //Serial.println("post connection failed");
  }
  lastConnectionTime = millis();
}


// this method makes a HTTP connection to the server:
void Get(byte sensorID) { 
  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {
    //Serial.println("get connection success.");
    // Make a HTTP request:
    client.print("GET /v1.0/device/");
    client.print(device);
    client.print("/sensor/");
    client.print(sensorID);
    client.print("/datapoint");
    client.println(" HTTP/1.1");
    client.print("Host:");
    client.println(server);
    client.println("Content-Type:application/json");
    client.print("U-ApiKey:");
    client.println(ukey);
    client.println("Connection: close");
    client.println();
    
    // note the time that the connection was made:
    getLastConnectionTime = millis();
  } 
  else {
    // if you couldn't make a connection:
    //Serial.println("get connection failed");
    //Serial.println("disconnecting.");
    client.stop();
  }
}

