/*
  ButtonEvent.cpp - Event-Based Library for Arduino.
  Copyright (c) 2011, Renato A. Ferreira
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of the author nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ButtonEvent.h"

ButtonEventClass::ButtonEventClass() {
	this->count = 0;
	this->mallocSize = 0;
	this->initialCapacity = sizeof(ButtonInformation);
}

void ButtonEventClass::addButton(short pin, void (*onDown)(ButtonInformation* Sender), void (*onUp)(ButtonInformation* Sender), void (*onHold)(ButtonInformation* Sender), unsigned long holdMillisWait, void (*onDouble)(ButtonInformation* Sender), unsigned long doubleMillisWait) {
	if (this->count > 0) {
		//determine if the buffer free space fits the next object
		if (this->mallocSize < (sizeof(ButtonInformation)*(this->count+1))) {
			this->mallocSize = sizeof(ButtonInformation)*(this->count+1);
			//alocate more memory space
			this->buttons = (ButtonInformation*) realloc(this->buttons, this->mallocSize);
		}
	} else {
		//determine if initial capacity parameter fits the first object
		if (this->initialCapacity >= sizeof(ButtonInformation)) {
			this->mallocSize = this->initialCapacity;
		} else {
			this->mallocSize = sizeof(ButtonInformation);
		}
		//create the buffer size
		this->buttons = (ButtonInformation*) malloc(this->mallocSize);
	}

	this->setPosition(this->count);
	this->currentButton->pin = pin;
	this->currentButton->analogValue = NOT_ANALOG;
	this->currentButton->onDown = onDown; 
	this->currentButton->onUp = onUp;
	this->currentButton->onHold = onHold;
	this->currentButton->holdMillisWait = holdMillisWait;
	this->currentButton->onDouble = onDouble;
	this->currentButton->doubleMillisWait = doubleMillisWait;
	
	pinMode(this->currentButton->pin, INPUT_PULLUP);
	
	this->count++;
}

void ButtonEventClass::addButton(short pin, short analogValue, byte deviation, void (*onDown)(ButtonInformation* Sender), void (*onUp)(ButtonInformation* Sender), void (*onHold)(ButtonInformation* Sender), unsigned long holdMillisWait, void (*onDouble)(ButtonInformation* Sender), unsigned long doubleMillisWait) {
	if (this->count > 0) {
		//determine if the buffer free space fits the next object
		if (this->mallocSize < (sizeof(ButtonInformation)*(this->count+1))) {
			this->mallocSize = sizeof(ButtonInformation)*(this->count+1);
			//alocate more memory space
			this->buttons = (ButtonInformation*) realloc(this->buttons, this->mallocSize);
		}
	} else {
		//determine if initial capacity parameter fits the first object
		if (this->initialCapacity >= sizeof(ButtonInformation)) {
			this->mallocSize = this->initialCapacity;
		} else {
			this->mallocSize = sizeof(ButtonInformation);
		}
		//create the buffer size
		this->buttons = (ButtonInformation*) malloc(this->mallocSize);
	}

	this->setPosition(this->count);
	this->currentButton->pin = pin;
	this->currentButton->analogValue = analogValue;
	this->currentButton->deviation = deviation;
	this->currentButton->onDown = onDown; 
	this->currentButton->onUp = onUp;
	this->currentButton->onHold = onHold;
	this->currentButton->holdMillisWait = holdMillisWait;
	this->currentButton->onDouble = onDouble;
	this->currentButton->doubleMillisWait = doubleMillisWait;

	pinMode(14+this->currentButton->pin, INPUT);
	digitalWrite((14+this->currentButton->pin), HIGH);
	
	this->count++;
}

void ButtonEventClass::setPosition(short Position) {
	this->currentButton = this->buttons+Position;
}

void ButtonEventClass::loop() {
	for (this->index = 0; this->index < this->count; this->index++) {
		this->setPosition(this->index);
		
		if (this->currentButton->analogValue == NOT_ANALOG) {
			this->nextPressed = (digitalRead(this->currentButton->pin) == LOW);
		} else {
			this->nextAnalogRead = analogRead(this->currentButton->pin);
			this->nextPressed = ((this->nextAnalogRead >= (this->currentButton->analogValue-this->currentButton->deviation)) && (this->nextAnalogRead <= (this->currentButton->analogValue+this->currentButton->deviation)));
		}
		
		//down event
		if (this->nextPressed) {
			if (this->currentButton->pressed) {
				//hold event
				if (!this->currentButton->hold && this->currentButton->onHold != NULL && this->currentButton->holdMillisWait > 0) {
					this->currentButton->holdMillis = millis() - this->currentButton->startMillis; //calculate time
					
					if (this->currentButton->holdMillis >= this->currentButton->holdMillisWait) {
						this->currentButton->onHold(this->currentButton); //call event
						this->currentButton->hold = true;
					}
				}
			} else {
				//double event
				if (this->currentButton->onDouble != NULL && this->currentButton->doubleMillisWait > 0) {
					this->lastMillis = millis();
					this->currentButton->doubleMillis = this->lastMillis - this->currentButton->startMillis; //calculate time
					this->currentButton->startMillis = this->lastMillis;
					
					if (this->currentButton->doubleMillis <= this->currentButton->doubleMillisWait)
						this->currentButton->onDouble(this->currentButton); //call event
					else
						if (this->currentButton->onDown != NULL)
							//down event
							this->currentButton->onDown(this->currentButton); //call event
				} else {
					//down event
					this->currentButton->startMillis = millis();
					
					if (this->currentButton->onDown != NULL)
						this->currentButton->onDown(this->currentButton); //call event
				}
			}
		}
		
		//up event
		if (!this->nextPressed && this->currentButton->pressed) {
			if (this->currentButton->onUp != NULL) {
				this->currentButton->holdMillis = millis() - this->currentButton->startMillis; //calculate time
				
				this->currentButton->onUp(this->currentButton); //call event
			}
			
			this->currentButton->hold = false;
		}

		this->currentButton->pressed = this->nextPressed;
	}
}

ButtonEventClass ButtonEvent;
