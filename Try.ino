//Import libraries
#import <Wire.h>

//Define variables
int clkpin=4;
int clkinh=5;
int	serial=3;
int	shload = 6;

void (*machineList[4])();
byte colums = 0;

//Function definitions for ReadButtons FMM
void nothing() {
	digitalWrite(clkinh, HIGH);
	digitalWrite(shload, LOW);
	machineList[0] = &LD;
}

void LD() {
	delayMicroseconds(5);
	digitalWrite(shload, HIGH);
	machineList[0] = &SH;
}

void SH() {
	digitalWrite(clkinh, LOW);
	machineList[0] = &read;
}

void read() {
	colums = 0;

	for (int i = 0; i < 8; i++) {
		colums <<= 1;
		colums |= digitalRead(serial);
		digitalWrite(clkpin, HIGH);
		delayMicroseconds(5);
		digitalWrite(clkpin, LOW);
	}
	digitalWrite(clkinh, HIGH);
	machineList[0] = &nothing;
}

// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(9600);
	machineList[0] = &nothing;
	pinMode(clkpin, OUTPUT);
	pinMode(clkinh, OUTPUT);
	pinMode(shload, OUTPUT);
	pinMode(serial, INPUT);

	digitalWrite(clkinh, HIGH);
	digitalWrite(shload, HIGH);
	digitalWrite(clkpin, LOW);
}

// Add the main program code into the continuous loop() function
void loop()
{
	(*machineList[0])(); 
	Serial.println(colums,DEC);

}
