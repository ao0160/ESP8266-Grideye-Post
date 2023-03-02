/*
 *    Esp Grideye Post is a derivative of the example:
 *      PostHTTPClient.ino
 *      Created on: 21.11.2016
 *
 *    This is a really basic implementation that simply posts the data from the Grideye Array to the website.
 *
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include "credentials.h"

#define USE_SERIAL Serial

#define SERVER_IP "olvera-dev.com"
//#define DEBUG



// Expires Tue, 04 Apr 2023 09:26:32 GMT
const uint8_t sha1_fingerprint[20] = {
0x38, 0x59, 0x26, 0xE4, 0xC4, 0x73, 0x19, 0x41, 0xFC, 0x76, 0x61, 0xF5 , 0xA0, 0x6A, 0x28, 0x72, 0x8E, 0x39, 0x72, 0xB2  
};

Adafruit_AMG88xx amg;

float grideye[AMG88xx_PIXEL_ARRAY_SIZE];
String grideye_message;
String temp;

void setup() {
  USE_SERIAL.begin(115200);
  srand (time(NULL));

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  bool status;

  // default settings
  status = amg.begin();
  if (!status) {
      USE_SERIAL.println("Could not find a valid AMG88xx sensor, check wiring!");
      while (1);
  }


  // Actually grab the first output of the AMG board. Delay is present to allow the sensor to boot up.
  amg.readPixels(grideye);
  delay(100); // let sensor boot up

  // Pretty print to serial monitor.
  USE_SERIAL.print("Displaying grideye data values:\n");
  for(int n=1; n<=AMG88xx_PIXEL_ARRAY_SIZE; n++){
    USE_SERIAL.print(grideye[n-1]);
    USE_SERIAL.print(", ");
    if( n%8 == 0 ) USE_SERIAL.println();
  }

  WiFi.begin(STASSID, STAPSK);    // If your wifi is protected, you'd pass the SSID and password here.
  //WiFi.begin(STASSID);          // If your wifi is unprotected, you can simply use this instance.

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    USE_SERIAL.print(".");
  }
  USE_SERIAL.println("");
  USE_SERIAL.print("Connected! IP address: ");
  USE_SERIAL.println(WiFi.localIP());

  USE_SERIAL.print("Grideye array generated.\n");   // This is done earlier, but I wanted to have the wifi connection completed after prompting.

  // Reserve 8 bytes for float value storage, as I didn't want String to constantly spam the memory in the POST generation.
  temp.reserve(8);
}

void loop() {
  // wait for WiFi connection.
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    std::unique_ptr<BearSSL::WiFiClientSecure>https_client(new BearSSL::WiFiClientSecure);
    https_client->setFingerprint(sha1_fingerprint);
    HTTPClient https;

    USE_SERIAL.print("[HTTP] begin...\n");
    // configure traged server and url, there's technically another instance of the espupdate script but we will use the one in the root.
    https.begin(*https_client, "https://" SERVER_IP "/esp8266/espupdate.php"); //HTTPs
    https.addHeader("Content-Type", "application/x-www-form-urlencoded,; charset=UTF-8");

    USE_SERIAL.print("[HTTPS] POST...\n");
    // start connection and send HTTP header and body
    // Update is set true, and array is prepared to insert the Grideye data.
    grideye_message = "update=1&array=";
    for (int n=1; n <= AMG88xx_PIXEL_ARRAY_SIZE; n++){
        temp = String(grideye[n-1]);
        if ( n < AMG88xx_PIXEL_ARRAY_SIZE){
          grideye_message += temp + ",";    // Concatenate array values to post. This adds one spare empty line at end, but for now it's fine as it's handled on the server.
        }
        else{
          grideye_message += temp;    // Concatenate array values to post. This adds one spare empty line at end, but for now it's fine as it's handled on the server.
        }  
    }

    USE_SERIAL.println(grideye_message);
    int httpCode = https.POST(grideye_message);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server, Serial echoes the receive message.
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = https.getString();
        USE_SERIAL.println("received payload:\n----------------------------");
        USE_SERIAL.println(payload);
        USE_SERIAL.println("----------------------------");
      }
    } else {
      // Simply print any errors to the console.
      USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  }

  // Grab next instance of capture.
  amg.readPixels(grideye);

  // Display grab on serial so monitor can see any updates if they wish.
  USE_SERIAL.print("Displaying grideye data values:\n");
  for(int n=1; n<=AMG88xx_PIXEL_ARRAY_SIZE; n++){
    USE_SERIAL.print(grideye[n-1]);
    if ( n < AMG88xx_PIXEL_ARRAY_SIZE){
      USE_SERIAL.print(", ");
    }

    if( n%8 == 0 ) USE_SERIAL.println();
  }

  // 15 second delay, I was going to implement a non wait delay but since this isn't really critical I kept the delay provided by the example.
  delay(15*1000);
}
