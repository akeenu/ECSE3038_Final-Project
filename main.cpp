#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial esp(18,19);

String sendData(String command, const int timeout, boolean debug)
{
    String response ="";
    esp.print(command);
    long int time = millis();
    while( (time+timeout) > millis())
    {
      while(esp.available())
      {
        char c = esp.read();
        response+=c;
      }
    }
    if(debug)
    {
     Serial.print(response);
    }
    return response;
}

void setup() {
  // put your setup code here, to run once:
  esp.begin(115200);
  Serial.begin(115200);

sendData("AT+RST\r\n" ,1000,true);
sendData("AT+CWLAP\r\n" , 1000, true);
}

void loop() {
  // put your main code here, to run repeatedly:
  sendData("AT+CIPSTART=\"TCP\", \"10.22.60.87\",\"5000\")
  String post = "POST /data HTTP/1.1\r\n Host:192.168.1.199:5000\r\n
}



