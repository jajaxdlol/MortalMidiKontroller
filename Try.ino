//Import libraries
#import <Wire.h>

//Define variables
#define clkpin	11
#define	clkinh	5
#define	serial	3
#define shload	6

void (*machineList[4])();
byte colums = 0;

//Function definitions for ReadButtons FMM
void nothing() {
	pinMode(shload, LOW);
	machineList[0] = &LD;
}

void LD() {
	pinMode(shload, HIGH);
	machineList[0] = &SH;
}

void SH() {
	pinMode(clkinh, LOW);
	machineList[0] = &INH;
}

void INH(){
	machineList[0] = &read;
}

void read() {
	colums = 0;
	for (int i = 0; i < 8; i++) {
		colums <<= 1;
		colums |= digitalRead(serial);
	}
	machineList[0] = &nothing;
}

// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(9600);
	machineList[0] = &nothing;
	pinMode(clkpin, OUTPUT);
	pinMode(clkinh, OUTPUT);
	pinMode(serial, INPUT);
	pinMode(shload, OUTPUT);
	digitalWrite(clkinh, HIGH);
	digitalWrite(shload, HIGH);

	TCCR1A = bit(COM1A0);  
	TCCR1B = bit(WGM12) | bit(CS10);  
	OCR1A = 255;
}

// Add the main program code into the continuous loop() function
void loop()
{
	(*machineList[0])(); 
	Serial.println(colums,DEC);

}
