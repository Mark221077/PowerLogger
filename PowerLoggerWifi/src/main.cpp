#include <Arduino.h>

#define STATIONNO 10

#include <ESP8266HTTPClient.h>
#include <ESP8266Wifi.h>

#define STARTSEQ (010101L) //indicates the start of communication between the ESP and Arduino

#define WIFISSID "PalMark_Network"
#define WIFIPASSWORD "11235813"

#include "wrapperUnions.h"

FloatWrapper fWrapper;
LongWrapper lWrapper;

float sinceLastSuccess = 0.0; //used to store the sum of deltas, when sending to DB failed

void setup()
{
    Serial.begin(115200); //begin serial
    delay(100);           //wait a bit

    WiFi.mode(WIFI_OFF); //reset wifi
    delay(1000);
    WiFi.mode(WIFI_STA); //turn on as simple station

    WiFi.begin(WIFISSID, WIFIPASSWORD); //connect to a network

    delay(10);

    //connected

    sinceLastSuccess = 0.0;
    
}

void loop()
{
    // put your
    while (Serial.available())
    {
        delay(10); //wait for all the data to arrive

        lWrapper.num = 0L;
        Serial.readBytes(lWrapper.b, LONGSIZE); //STARTSEQ

        if (lWrapper.num != STARTSEQ)
        { //start sequence not detected, empty the buffer and try again
            //while(Serial.available())   Serial.read();
            Serial.flush();
            return;
        }

        //read the delta
        fWrapper.num = 0.0;
        Serial.readBytes(fWrapper.b, FLOATSIZE);
        float delta = sinceLastSuccess + fWrapper.num; //the difference between the last sent and the current

        String postData = "ip=" + WiFi.localIP().toString() + "&station=" + STATIONNO + "&delta=" + String(delta, 10);

        int httpCode = 500; //assume failure

        if (WiFi.status() == WL_CONNECTED)
        {
            //connected to wifi
            HTTPClient http;


            http.begin("http://192.168.1.3/powerlogger/log.php");                //Specify request destination
            http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header


            http.setTimeout(5000);

            httpCode = http.POST(postData);    //Send the request
            
            http.end(); //Close connection
        }


        if (httpCode == 200 || httpCode == 201)
        {
            //success
            sinceLastSuccess = 0;
        }
        else
        {
            //something failed
            //store the deltas not sent
            sinceLastSuccess = delta;
        }
    }
}
