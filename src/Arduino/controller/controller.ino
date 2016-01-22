/*******************************************************************************************************
controller.ino
Created on:	19.12.2015

Pong is a homage to the famous arcade game Pong with the capability of old-school controllers using an Arduino Uno board. 

Copyright (C) 2015-2016 Markus Diem <markus@nomacs.org>

This file is part of Pong.

Pong is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Pong is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************************************/

int debugVal = -1;
int LED = 13;  // the nice orange thingy

struct Controller {
  int pin = 0;
  int val = 0;
  int output = 0;
  int digital = 0;  
};

Controller c0;	// player 1
Controller c1;	// player 2
Controller c2;	// speed controll
Controller c3;  // generic
Controller c4;  // generic
Controller c5;  // generic
Controller pc;  // power button

// the setup function runs once when you press reset or power the board
void setup() {
  
  c0.pin = 0;
  c1.pin = 1;
  c2.pin = 2;
  c3.pin = 3;
  c4.pin = 4;
  c5.pin = 5;
  
  pc.pin = 7;
  pc.digital = 1;
  pc.val = digitalRead(pc.pin);  // do not fire at the beginning
      
  // initialize digital pin 13 as an output.
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)

  Serial.begin(9600);          //  setup serial
}

// the loop function runs over and over again forever
void loop() {
  
  update(&c0);  // update controller 0
  update(&c1);  // update controller 1
  update(&c2);  // update controller 2
  update(&c3);  // update controller 3
  update(&c4);  // update controller 4
  update(&c5);  // update controller 5
  update(&pc);  // update power button

  Serial.flush();
  delay(7);
  
}

// check for a new value and print it to the serial port...
void update(struct Controller* c) {
  
  if (!c) {
    Serial.println(-1);  // illegal state
    return;
  }
  
  int ar = 0;
  
  if (c->digital == 1) {
    ar = digitalRead(c->pin);
     if (ar == c->val)  // ignore noise
      return;
  }
  else {
    ar = analogRead(c->pin); 
    if (abs(ar - c->val) < 4)  // ignore noise
      return;
  }
  
  c->val = ar;
  merge(c);  // merges pin & value for the serial output
  put(c);
}

// merges controller pin & value to one short
void merge(struct Controller* c) {

  c->output = 0;  // clear
  c->output = c->pin << 10;  // shift the pin above 1024 - the analog input is max 1023
  c->output += c->val;
}

void put(struct Controller* c) {
  Serial.write(42);	// first write a marker (to align the next two bytes)
  Serial.write((const char*)&c->output, 2);  // write pin & value to serial as short
}
