// Slave
// -----
//
// For: http://forum.arduino.cc/index.php?topic=415439.0
// Tested with Arduino IDE 1.6.10 and two Arduino Uno boards.
// The two boards are connected via SDA, SCL and GND.

// I2C_Anything from : http://www.gammon.com.au/i2c
//Important library, this handles the I2C data transfer between the Uno and Due. Written by Nick Gammon
//You need to link the "I2C_Anything.h" file to this file, by having it in the same Arduino project folder, and opening it as an attached tab in the IDE. 


#include "I2C_Anything.h"

const int slaveAddress = 9;
volatile float volts;

String latValueString = "0";
String lonValueString = "0";
String altValueString = "0";

float lat;
float lon;
float alt;
float tst;


void setup()
{
  Serial.begin( 9600);

  Wire.begin( slaveAddress);            // Join I2C bus with this address
  Wire.onRequest( requestEvent);         //
}

void loop()
{
  volts = analogRead(A0) * 5.0 / 1024.0;


  String gpsDataString;
  String values[3];
  
  if (Serial.available() > 0) {
    
    while (Serial.available()) {    
      delay(1);
      gpsDataString = gpsDataString + (char)Serial.read();
    }

    int stringPlace = 0;
    int arrayIndex = 0;

    for (int i = 0; i < gpsDataString.length(); i++){
      if (gpsDataString.charAt(i) == ','){
        values[arrayIndex] = "";

        values[arrayIndex] = gpsDataString.substring(stringPlace, i);
        stringPlace = (i+1);
        arrayIndex++;
        
        }
      }

      latValueString = values[0];
      lonValueString = values[1];
      altValueString = values[2];
      
      lat = latValueString.toFloat(); 
      lon = lonValueString.toFloat(); 
      alt = altValueString.toFloat(); 

      Serial.println();
      Serial.print("lat = ");
      Serial.print(latValueString);
      Serial.println();
      Serial.print("lon = ");
      Serial.print(lonValueString);
      Serial.println();
      Serial.print("alt = ");
      Serial.print(altValueString);
      Serial.println();
  }

  delay(500);
}

void requestEvent()
{
  I2C_writeAnything( volts);
  I2C_writeAnything( lat);
  I2C_writeAnything( lon);
  I2C_writeAnything( alt);

}
