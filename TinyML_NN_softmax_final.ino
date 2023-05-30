/**
 * MIT License
 *
 * Copyright (c) 2022 Avi G
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// This sketch could be used for debugging. This prints neural network output, softmax output and posterior filter output.

#include <NDP.h>
#include <NDP_utils.h>
#include <Arduino.h>
#include "TinyML_init.h"
#include "NDP_init.h"
#include "SAMD21_init.h"
#include "SAMD21_lowpower.h"

#define DO_NOT_WAIT_FOR_SERIAL_PORT 1                       // For battery powered autonomus usage, use this option
#define WAIT_FOR_SERIAL_PORT 0                              // For debugging purposes use this option, program will wait until serial port is turned on, Arduino IDE Tools --> Serial Monitor
#define NDP_MICROPHONE 0                                    // Model type (Audio or Sensor) and this slection MUST match //JRIHA pokud mam NDP_MICROPHONE 1 a NDP_SENSOR 0 tak mi zvuk nedetekuje. pokud je to obracene, tak vse funguje OK. Je to divne...
#define NDP_SENSOR 1                                        // Model type (Audio or Sensor) and this slection MUST match
#define NUMBER_OF_CLASSIFIERS 3                            // Number of classfiers including softmax //JRIHA musi zde byt hodnota 3 presto ze mam v modelu casifiery 2 (SIREN, NOISE). Pokud zadam 2, tak model nic nedetekuje
#define SOFTMAX_ADDRESS_START 0x2000143C                    // NDP specific address, some old models may have address 0x1fffd41c or 0x2000143C, this may change if SDK changes

uint32_t softMaxAddress;                                    // Address to get Softmax value
uint32_t softMaxValue;                                      // Softmax value
uint32_t startingFWAddress;                                 // This variable becomes pointer for the current internal pointer
uint32_t currentPointer;                                    // This is the pointer where new data is written
uint32_t oldPointer=0;                                      // This variable is to determine if there is new data written or not
uint32_t tankRead;                                          // Local variable to hold value of value read from NDP
int match;
uint32_t detection_count = 0;                                //Promenna filteru
extern bool sirenDetected;

void turnOnLED(int classifier, int openSetClassifier){
  if (classifier < openSetClassifier){                        // For classifiers other than openSet
    if (classifier==0)
                    {
                    sirenDetected = false;
                    digitalWrite(LED_RED, LOW);            // Turning on RED LED for Classifier 0
                    digitalWrite(LED_GREEN, LOW);           // Turning OFF GREEN LED for Classifier 0
                    digitalWrite(LED_BLUE, LOW);            // Turning OFF BLUE LED for Classifier 0
                    }
    if (classifier==1)                                      // Toto je zvuk sireny
                    {
                    detection_count++;                      //Jednoduchý counter, který má fungovat jako filtr proti náhodnému poblikávání (V podstate mi to vyresilo tento problem)
                    if(detection_count > 15)                 //Jednoduchý counter, který má fungovat jako filtr proti náhodnému poblikávání (V podstate mi to vyresilo tento problem)
                    {
                      sirenDetected = true;
                      digitalWrite(LED_GREEN, HIGH);          // Turning OFF GREEN LED for Classifier 1
                      digitalWrite(LED_RED, LOW);             // Turning OFF RED LED for Classifier 1
                      digitalWrite(LED_BLUE, HIGH);            // Turning on BLUE LED for Classifier 1
                    }

                    }
     else{                                                      //Jednoduchý counter, který má fungovat jako filtr proti náhodnému poblikávání
           detection_count = 0;                                 //Jednoduchý counter, který má fungovat jako filtr proti náhodnému poblikávání
         }
   
 //   if (classifier==1)digitalWrite(LED_BLUE, HIGH);           // Turning on GREEN LED for Classifier 2 // upto 7 distinct colors can be generated  
  }   
  else{                                                       // For classifiers openSet
    digitalWrite(LED_RED, LOW);                               // Turning off All RGB LEDs
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);         
  }
                                                
}

void setup(void) {
  //SAMD21_init(DO_NOT_WAIT_FOR_SERIAL_PORT);               // Setting up SAMD21 (0) will not wait for serial port
  SAMD21_init(WAIT_FOR_SERIAL_PORT);                        // Setting up SAMD21 (1) will wait and RGB LED will be Red
  NDP_init("ei_model.bin",NDP_MICROPHONE);                // Setting up NDP, Stuck Blue LED means model is not read 
  //NDP_init("ei_model_sensor.bin",NDP_SENSOR);               // Setting up NDP, Stuck Blue LED means model is not read 
  Serial.println(" \n Setup done"); 
  delay(2000);                                              // Flushing out buffer
  NDP.poll();                                               // Flushing out buffer
  startingFWAddress = indirectRead(0x1fffc0c0);             // Finding pointer which holds the value of the current tank pointer
  ledLogo_init();
}

uint8_t extractByteValue(uint32_t input, uint8_t bytePosition){ //Fron 32 bit NDP memory map value is parsed into 8 bit uint
   uint8_t value = input >> bytePosition*8; // BytePosition can be 0, 1, 2, 3
   return value;
}

void loop() {
    delay(22);                                                         // This is set a little lower than audio inferencing period which is 24ms
    currentPointer = indirectRead(startingFWAddress);                  // Finding location where new new data is written in the holding tank
    if (currentPointer != oldPointer){                                 // Checking if new data is written or not
      oldPointer = currentPointer;                                     // Updating oldPointer variable
      Serial.print("Current Pointer : ");   
      Serial.print(currentPointer);     
      Serial.print(" Neural network output : ");
      for (int i=0; i<NUMBER_OF_CLASSIFIERS; i += 4){                  // Saving classifier values before posterior filter
         tankRead = indirectRead(0x60062310+i);
         for (int bytePosition=0; bytePosition<(4); bytePosition++ ){  // 4 values will be written at a time, if classifiers are not in multiple of 4
              Serial.print(extractByteValue(tankRead,bytePosition));   // then ignore unused classifiers on the right side, left most value represents
              Serial.print(",");                                       // classifier 0
         }
         Serial.print("  ");                                           // Adding space for every 4 neural network outputs for easy read in chunks of 4
      }
      Serial.print("Softmax outputs : ");
      for (int i=0; i<NUMBER_OF_CLASSIFIERS; i++){                     // Saving classifier values before posterior filter
           softMaxAddress = SOFTMAX_ADDRESS_START + i*4;               // Generation Address SoftMax
           softMaxValue = indirectRead(softMaxAddress);                 // Finding value of SoftMax
           if (softMaxValue>33000) turnOnLED(i, NUMBER_OF_CLASSIFIERS-1);  // The threshold could be changed to turn-on LED //JRIHA Ladeni hodnoty sice ovlivnuje citivost, ale poblikavani LEDky me to nezbavilo
           Serial.print(softMaxValue);
           Serial.print(" , ");
      }
      Serial.print("Winning Classifier + 1 : ");                       // 1 is added as Match is reported as 0 when openset is triggered, Classifier 0 is indicated by 1 etc    
      match = NDP.poll();                                              // This API fetches last assertion of the match, in this sketch polling is done without interrupt evaluation
      Serial.print("Match : ");                                        
      Serial.println(match);
    }
    logoRoutine();
}
