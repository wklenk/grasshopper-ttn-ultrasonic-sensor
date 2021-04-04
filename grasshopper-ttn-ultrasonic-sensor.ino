/*
  Example how to periodically send distance information of a ultrasonic
  sensor (MaxBotix MB1242 I2C MaxSonar EZ4) using a Grasshopper LoRaWAN 
  Development Board and a LM3671 as kind of power switch to consume as 
  little energy as possible. Data is sent over The Things Network (TTN).

  Copyright (C) 2021  Wolfgang Klenk <wolfgang.klenk@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <STM32L0.h>
#include "Wire.h"
#include "LoRaWAN.h"

const char *appEui = "70B3xxxxxxxxxxxx";
const char *appKey = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char *devEui = "xxxxxxxxxxxxxxxx"; 

void setup()
{
  // Pin A0 controls the EN pin of the LM3671
  // LM3671 acts as kind of power supply that can be switched on or off.
  pinMode(A0, OUTPUT);
  
  Wire.begin();

  LoRaWAN.begin(EU868);
  LoRaWAN.setSubBand(2); // for TTN 
  LoRaWAN.joinOTAA(appEui, appKey, devEui);
}

void loop()
{
    int size;
    uint8_t data[2];

    // Pull "enable" of LM3671 to high in order to provide supply power
    // to the MB1242 sensor.
    digitalWrite(A0, HIGH);

    // Give the sensor some time to start up.
    delay(200);

    Wire.beginTransmission(224 >> 1);
    Wire.write(81);
    Wire.endTransmission();

    delay(200);  // Give the sensor some time to do a range reading

    size = Wire.requestFrom(224 >> 1, 2);

    if (size)
    {
        Wire.read(data, size);

        if (!LoRaWAN.busy() && LoRaWAN.joined()) {
        LoRaWAN.beginPacket();
        LoRaWAN.write(data[0]);
        LoRaWAN.write(data[1]);
        LoRaWAN.endPacket();
        }
    }

    // Pull "enable" of LM3671 to low in order to cut-off power
    // from the MB1242 sensor.
    digitalWrite(A0, LOW);

    // Sleep 30 minutes
    STM32L0.stop(30 * 60 * 1000);
}
