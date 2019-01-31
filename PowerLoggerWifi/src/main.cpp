#include <Arduino.h>

#define STATIONNO   10

#include <ESP8266HTTPClient.h>
#include <ESP8266Wifi.h>

#define STARTSEQ (010101L) //indicates the start of communication between the ESP and Arduino

#define WIFISSID "PalMark_Network"
#define WIFIPASSWORD "11235813"

#define DOUBLESIZE 4
//helper type to convert double to 4 bytes(double on atmega328 is 4 bytes)
typedef union _doubleWrapper {
    double num;
    byte b[DOUBLESIZE];
} DoubleWrapper;

#define LONGSIZE 4
//same as above for long
typedef union _longWrapper {
    unsigned long num;
    byte b[LONGSIZE];
} LongWrapper;

DoubleWrapper dWrapper;
LongWrapper lWrapper;

void setup()
{
    Serial.begin(115200); //begin serial
    delay(100); //wait a bit

    WiFi.mode(WIFI_OFF); //reset wifi
    delay(1000);
    WiFi.mode(WIFI_STA); //turn on as simple station

    WiFi.begin(WIFISSID, WIFIPASSWORD); //connect to a network

    delay(10);
    int i = 0;

    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
        delay(500); //wait for connection
    }

    //connected
}

void loop()
{
    // put your
    while (Serial.available() && WiFi.status() == WL_CONNECTED) {
        delay(10); //wait for all the data to arrive

        lWrapper.num = 0L;
        Serial.readBytes(lWrapper.b, LONGSIZE); //STARTSEQ

        if (lWrapper.num != STARTSEQ) { //start sequence not detected, empty the buffer and try again
            //while(Serial.available())   Serial.read();
            Serial.flush();
            return;
        }

/*
        //read the time
        lWrapper.num = 0L;
        Serial.readBytes(lWrapper.b, LONGSIZE);
        unsigned long time = lWrapper.num;
*/
        //read the consumption data
        dWrapper.num = 0.0;
        Serial.readBytes(dWrapper.b, DOUBLESIZE);
        double data = dWrapper.num;

        String postData = "ip=" + WiFi.localIP().toString() + "&station=" + STATIONNO + "&data=" + String(data, 10);


        HTTPClient http;

        http.setTimeout(500);

        http.begin("http://192.168.1.3/test1/index.php"); //Specify request destination
        http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header

        int httpCode = http.POST(postData); //Send the request
        String payload = http.getString(); //Get the response payload

        http.end(); //Close connection
    }
}
