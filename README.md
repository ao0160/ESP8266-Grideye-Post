# ESP8266-Grideye-Post
Posting data to a remote server utilizing an ESP8266 and an AMG8833 sensor. 

## Setup
System utilizes an ESP8266 as well as the Arduino IDE and Adafruit AMG8833 library.

## Function
The system obtains an 8x8 temperature array and uploads to a remote server via HTTPS and POST. 
The remote server stores the data. Additionally, the remote server allows you to browse entries by converting them into images with 
colors corresponding to temperatures. To better replicate the example by adafruit, the remote server replicates the same color mapping
as their TFT thermal cam example.
