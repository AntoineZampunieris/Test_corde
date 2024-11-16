#include <Arduino.h>

#include "AnalogEvent.h"
#include "ButtonEvent.h"

#define PbMode 12
#define SynthCroAna 4
#define Flash 13
#define PulseMode 1
#define ContinusMode 0

unsigned long SignalPeriod=0;
unsigned long LastSignalPeriod=50;
unsigned long BeginSinus;
unsigned long StartFlash;
unsigned long StartTime=0;
float VoltToDelayToFlash;
unsigned long DetayToFlash;
unsigned int State=0;
uint8_t Mode=ContinusMode;

void onChangeAnaDelay(AnalogPortInformation* Sender);
void onUpAudioSignal(ButtonInformation* Sender) {
      
  SignalPeriod=micros()-StartTime;
  if ((SignalPeriod<LastSignalPeriod-1500)||(SignalPeriod>LastSignalPeriod+1500) )
  {
    // Serial.print("SignalPeriod: ");
  // Serial.println(SignalPeriod);
  DetayToFlash=((int)((float)SignalPeriod)*(VoltToDelayToFlash)/(float)100);
  // Serial.print("DetayToFlash: ");
  // Serial.println(DetayToFlash);  
  LastSignalPeriod=SignalPeriod;/* code */
  }
  StartTime=micros();
  State=1;
}

void onDownBpMode(ButtonInformation* Sender) {
  
  Mode=!Mode;
} 
 
void onChangeAnaDelay(AnalogPortInformation* Sender) {
    VoltToDelayToFlash=map(Sender->value,0,1023,0,90);
    DetayToFlash=((unsigned long)((float)SignalPeriod)*(VoltToDelayToFlash)/(float)100);
}


void setup() {
  // put your setup code here, to run once:
 pinMode(Flash, OUTPUT);
 pinMode(PbMode, INPUT_PULLUP);
 digitalWrite(Flash, LOW);
 Serial.begin(115200);
    while (!Serial) {
        // some boards need to wait to ensure access to serial over USB
    }
    Serial.println("SerialBegin ok");    

AnalogEvent.addAnalogPort(A0,        //potentiometer pin
                            onChangeAnaDelay, //onChange event function
                            30);       //hysteresis

ButtonEvent.addButton(SynthCroAna,       //button pin
                        NULL,   //onDown event function
                        onUpAudioSignal,     //onUp event function
                        NULL,   //onHold event function
                        NULL,     //hold time in milliseconds
                        NULL, //double event function
                        NULL);     //double time interval   
ButtonEvent.addButton(PbMode,       //button pin
                        onDownBpMode,   //onDown event function
                        NULL,     //onUp event function
                        NULL,   //onHold event function
                        NULL,     //hold time in milliseconds
                        NULL, //double event function
                        NULL);     //double time interval      


SignalPeriod=micros();
StartTime=SignalPeriod;

}

void loop() {
  // put your main code here, to run repeatedly:
  AnalogEvent.loop();
  ButtonEvent.loop();
  if (Mode==ContinusMode)
  {
    digitalWrite(Flash,HIGH);
  }else
  {
    switch (State)
          {
          case 0:
          break;
          case 1:
          BeginSinus=micros();  
          State=2;
          break;
          case 2:  
          if (micros()>(BeginSinus+DetayToFlash))
          {
            digitalWrite(Flash, HIGH);
            StartFlash=micros();
            State=3;
          }
          break;
          case 3:
          if (micros()>(StartFlash+1000))
          {
            digitalWrite(Flash, LOW);
            State=0;
          }
          break;
          default:
            break;
          }/* code */
  }

}


