//Import libraries
#import <Wire.h>
#import "MIDIUSB.h"
#import <EEPROM.h>

//Define variables
int clkpin=4;
int clkinh=5;
int	serial=3;
int	shload = 6;
int interval = 5;
unsigned long initt = 0;
unsigned long actual = 0;

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
int buttons[nbuttons];
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
// El tiempo está jodiendo de alguna forma la lectura
void SH() {
	digitalWrite(clkinh, LOW);
	delayMicroseconds(5);
	machineList[0] = &read;
}

void read() {
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
	getPressedButtons();
	for (int i = 0; i < nbuttons; i++) {
		if (buttons[i] == 1 && lastbuttonsState[i] == 0) {
			noteOn(0, padnotes[selected_bank][i], 127);
			MidiUSB.flush();
		}
		if (buttons[i] == 0 && lastbuttonsState[i] == 1){
			noteOff(0, padnotes[selected_bank][i], 127);
			MidiUSB.flush();
		}
	}

	for (int i = 0; i < nbuttons; i++) {
		lastbuttonsState[i] = buttons[i];
	}
}


// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(9600);
	machineList[0] = &nothing;
	machineList[1] = &readAndSendNotes;
	pinMode(clkpin, OUTPUT);
	pinMode(clkinh, OUTPUT);
	pinMode(shload, OUTPUT);
	pinMode(serial, INPUT);
	/*if (EEPROM.read(0) != 0) {
		Uncomment this part when the user preset part is ready
	}*/

	digitalWrite(clkinh, HIGH);
	digitalWrite(shload, HIGH);
	digitalWrite(clkpin, LOW);
}

// Add the main program code into the continuous loop() function
void loop()
{
	(*machineList[0])();
	(*machineList[1])();
}
