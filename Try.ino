//Import libraries
#import <Wire.h>

//Define variables
#define clkpin	4
#define	clkinh	5
#define	serial	3
#define shload	6

void (*machineList[4])();
byte colums = 0;

//Function definitions
void readButtons() {
	int i = 0;
	colums = 0;
	digitalWrite(shload, LOW);
	digitalWrite(clkpin, HIGH);
	digitalWrite(shload, HIGH);
	digitalWrite(clkpin, LOW);

	
}


// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(9600);
	machineList[0] = &readButtons;
	pinMode(clkpin, OUTPUT);
	pinMode(clkinh, OUTPUT);
	pinMode(serial, INPUT);
	pinMode(shload, OUTPUT);
	digitalWrite(clkinh, HIGH);
	digitalWrite(clkpin, LOW);
	digitalWrite(shload, HIGH);
}

// Add the main program code into the continuous loop() function
void loop()
{
	(*machineList[0])(); 
	Serial.println("algo");
	Serial.println(colums,DEC);

}
