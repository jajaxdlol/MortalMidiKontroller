/*
____________
			|
			|
			|
			|
			|
			|
			|
			|
			|
____________|
*/


//Import libraries
#include <Wire.h>
#include "MIDIUSB.h"
#include <EEPROM.h>


//Define variables
const int ccthresh = 2;
const int clkpin=4;
const int clkinh=5;
const int serial=7;
const int shload = 6;
const int interval = 5;
unsigned long initt = 0;
unsigned long actual = 0;
const int bank1 = 8;
const int bank2 = 16;
const int bank3 = 14;
const int bank4 = 15;

//Gyroscope variables
const int MPU = 0x68;  // I2C address of the MPU-6050
int AcX, AcY;
unsigned long machinewait = 0;
unsigned long machineactual = 0;
int minVal = -18000;
int maxVal = 18000;
int xccval = 64;
int yccval = 64;
int antXCCVAL = 64;
int antYCCVAL = 64;

//Variables for choosing the notes to send
int baseNote = 48;
int shift = 0;
int padnotes_bank1[] = { 
	61, 67, 63, 65,
	60, 59, 57, 55,
	52, 50, 58, 56,
	49, 48, 54, 66 
};

int padnotes_bank2[] = {  
	61, 67, 63, 65,
	60, 59, 57, 55,
	52, 50, 58, 56,
	49, 48, 54, 66
};

int padnotes_bank3[] = {
	61, 67, 63, 65,
	60, 59, 57, 55,
	52, 50, 58, 56,
	49, 48, 54, 66
};

int padnotes_bank4[] = {
	61, 67, 63, 65,
	60, 59, 57, 55,
	52, 50, 58, 56,
	49, 48, 54, 66
};

int *padnotes[] = {
	padnotes_bank1,
	padnotes_bank2,
	padnotes_bank3,
	padnotes_bank4
};

int selected_bank = 1;

//Button states variables
const int nbuttons = 16;
int buttons[] = {  
0, 0, 0, 0,
0, 0, 0, 0,
0, 0, 0, 0,
0, 0, 0, 0
};
int lastbuttonsState[] = {  
0, 0, 0, 0,
0, 0, 0, 0,
0, 0, 0, 0,
0, 0, 0, 0
};

void (*machineList[4])();
byte reg1 = 0;
byte reg2 = 0;

//Function definitions for ReadButtons FMM
void nothing() {
	digitalWrite(clkinh, HIGH);
	digitalWrite(shload, LOW);
	initt = micros();
	machineList[0] = &nothingToLD;
}

void nothingToLD() {
	actual = micros();
	if ((unsigned long)(actual - initt) >= interval) {
		initt = micros();
		machineList[0] = &LD;
	}
}

void LD() {
	digitalWrite(shload, HIGH);
	machineList[0] = &SH;
}
void SH() {
	digitalWrite(clkinh, LOW);
	delayMicroseconds(5);
	machineList[0] = &readButtons;
}

void readButtons() {
	reg1 = 0;
	reg2 = 0;

	for (int i = 0; i < 8; i++) {
		reg1 <<= 1;
		reg1 |= digitalRead(serial);
		digitalWrite(clkpin, HIGH);
		delayMicroseconds(5);
		digitalWrite(clkpin, LOW);
	}
	for (int i = 0; i < 8; i++) {
		reg2 <<= 1;
		reg2 |= digitalRead(serial);
		digitalWrite(clkpin, HIGH);
		delayMicroseconds(5);
		digitalWrite(clkpin, LOW);
	}
	digitalWrite(clkinh, HIGH);
	machineList[0] = &nothing;
}

void noteOn(byte channel, byte pitch, byte velocity) {
	midiEventPacket_t noteOn = { 0x09, 0x90 | channel, pitch, velocity };
	MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
	midiEventPacket_t noteOff = { 0x08, 0x80 | channel, pitch, velocity };
	MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
	midiEventPacket_t event = { 0x0B, 0xB0 | channel, control, value };
	MidiUSB.sendMIDI(event);
}


void getPressedButtons() {
	byte displ = 1;
	for (int i = 0; i < nbuttons/2; i++) {
		if (!(reg1 & displ)) {
			buttons[i] = 1;
		}
		else {
			buttons[i] = 0;
		}
		displ <<= 1;
	}

	displ = 1;
	for (int i = nbuttons/2 ; i < nbuttons; i++) {
		if (!(reg2 & displ)) {
			buttons[i] = 1;
		}
		else {
			buttons[i] = 0;
		}
		displ <<= 1;
	}
}

void readAndSendNotes() {
	MidiUSB.flush();
	getPressedButtons();
	for (int i = 0; i < nbuttons; i++) {
		if (buttons[i] == 1 && lastbuttonsState[i] == 0) {
			noteOn(0, padnotes[selected_bank][i], 127);
			MidiUSB.flush();
      Wire.beginTransmission(10);
      Wire.write(1);
      Wire.write(i);
      Wire.endTransmission();
		}
		if (buttons[i] == 0 && lastbuttonsState[i] == 1){
			noteOff(0, padnotes[selected_bank][i], 127);
			MidiUSB.flush();
      Wire.beginTransmission(10);
      Wire.write(0);
      Wire.write(i);
      Wire.endTransmission();
		}
	}

	for (int i = 0; i < nbuttons; i++) {
		lastbuttonsState[i] = buttons[i];
	}

	if(xccval - antXCCVAL > ccthresh || xccval - antXCCVAL < -ccthresh){
		controlChange(0, 12, xccval);
		MidiUSB.flush();
		antXCCVAL = xccval;
	}
	if (yccval - antYCCVAL > ccthresh || yccval - antYCCVAL < -ccthresh) {
		controlChange(0, 13, yccval);
		MidiUSB.flush();
		antYCCVAL = yccval;
	}
}

void readBanks() {
	if (digitalRead(bank1))
	{
		selected_bank = 1;
	}
	else if (digitalRead(bank2)) 
	{
		selected_bank = 2;
	}
	else if (digitalRead(bank3))
	{
		selected_bank = 3;
	}
	else if (digitalRead(bank4))
	{
		selected_bank = 4;
	}
}

void readGyro() {
	Wire.beginTransmission(MPU);
	Wire.write(0x3B);
	Wire.endTransmission(false);
	Wire.requestFrom(MPU, 14, true);
	AcX = Wire.read() << 8 | Wire.read();
	AcY = Wire.read() << 8 | Wire.read();
	xccval = map(AcX, minVal, maxVal, 0, 127);
	yccval = map(AcY, minVal, maxVal, 0, 127);
	machinewait = micros();
	machineList[3] = &waitGyro;
}

void waitGyro() {
	machineactual = micros();
	if ((unsigned long)(machineactual - machinewait) >= interval*20)
		machineList[3] = &readGyro;
}

// The setup() function runs once each time the micro-controller starts
void setup()
{
	MidiUSB.flush();
	Wire.begin();
	Wire.beginTransmission(MPU);
	Wire.write(0x6B);  // PWR_MGMT_1 register
	Wire.write(0);     // set to zero (wakes up the MPU-6050)
	Wire.endTransmission(false);
	Wire.write(0x6B);  // PWR_MGMT_1 register
	Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
	Serial.begin(9600);
	machineList[0] = &nothing;
	machineList[1] = &readAndSendNotes;
	machineList[2] = &readBanks;
	machineList[3] = &readGyro;
	pinMode(clkpin, OUTPUT);
	pinMode(clkinh, OUTPUT);
	pinMode(shload, OUTPUT);
	pinMode(bank1, INPUT);
	pinMode(bank2, INPUT);
	pinMode(bank3, INPUT);
	pinMode(bank4, INPUT);
	pinMode(serial, INPUT);
	/*if (EEPROM.read(0) != 0) {
		Uncomment this part when the user preset part is ready
	}*/

	digitalWrite(clkinh, HIGH);
	digitalWrite(shload, HIGH);
	digitalWrite(clkpin, LOW);

  Wire.beginTransmission(10);
  Wire.write(0);
  Wire.write(24);
  Wire.endTransmission();
}

// Add the main program code into the continuous loop() function
void loop()
{
	(*machineList[0])();
	(*machineList[2])();
	(*machineList[3])();
	(*machineList[1])();
}
