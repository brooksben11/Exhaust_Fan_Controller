//Temp values to trigger fan speed changes (speed 0 to speed 1, 1 to 0, 1 to 2, etc.)
int Temp01 = 95;
int Temp10 = 85;
int Temp12 = 110;
int Temp21 = 100;
int Temp23 = 125;
int Temp32 = 115;

//Smoke reading values to trigger fan speed changes (speed 0 to speed 1, 1 to 0, 1 to 2, etc.)
int Smoke01 = 2200;
int Smoke10 = 2400;
int Smoke12 = 2000;
int Smoke21 = 2200;
int Smoke23 = 1500;
int Smoke32 = 1800;

//Delcare all the variables/flags
bool Publish_Values_Flag = false;
bool Button_Mode = false;
bool Speed_Check = false;
int Fan_Speed = 0;
bool Light_Button = false;
bool Fan_Button1 = false;
bool Fan_Button2 = false;
bool Fan_Button3 = false;
String Status = "Fan Off";
float TempC; //Sensor is calibrated for Celsius
float TempF;
int Temp;
int Temp_Current_Reading;
int Smoke_Current_Reading;

//Found the analog temp sensor to give me errant readings fairly frequently. Averaging weeded these out.
const int TempnumReadings = 60;     //The number of Temp readings that will be averaged (used to reduce noise)
int Tempreadings[TempnumReadings];  //The array of readings from the analog Temp input
int TempreadIndex = 0;              //The index of the current Temp reading
int Temptotal = 0;                  //The running total of Temp readings
int Tempaverage = 0;                //The average of the Temp readings
int Tempcounter = 0;                //Counter to average readings before full sample

//Timer to only average the temp values every second (in milliseconds)
Timer timer1(1000, Temp_Average);

//Timer to only change the fan speed every 10 seconds (in milliseconds)
//Added this to allow the system to stabilize and keep the fan from ramping up or down too quickly.
Timer timer2(10000, Fan_Check);

//Timer to publish the smoke values every 10 seconds (in milliseconds)
//Use this primarily for calibration purposes. Will probably comment out at some point.
Timer timer3(10000, Smoke_Check);

//Define all the pins
#define Temp_Pin A0
#define Smoke_Pin A1
#define Light_Button_Pin D7
#define Fan_Button1_Pin D0
#define Fan_Button2_Pin D1
#define Fan_Button3_Pin D2
#define Light_Relay D3
#define Fan_Relay1 D4
#define Fan_Relay2 D5
#define Fan_Relay3 D6


void setup() {
//Start all the timers
timer1.start();
timer2.start();
timer3.start();

//Set all the pin modes
pinMode(Light_Relay, OUTPUT);
pinMode(Fan_Relay1, OUTPUT);
pinMode(Fan_Relay2, OUTPUT);
pinMode(Fan_Relay3, OUTPUT);
pinMode(Light_Button_Pin, INPUT_PULLUP);
pinMode(Fan_Button1_Pin, INPUT_PULLUP);
pinMode(Fan_Button2_Pin, INPUT_PULLUP);
pinMode(Fan_Button3_Pin, INPUT_PULLUP);

//Initialize all the relays to off/open
digitalWrite(Light_Relay,LOW);
digitalWrite(Fan_Relay1,LOW);
digitalWrite(Fan_Relay2,LOW);
digitalWrite(Fan_Relay3,LOW);

//Initialize all the Particle variables so I can see the real-time status of everything.
Particle.variable("Status", Status);
Particle.variable("Temp", Temp);
Particle.variable("Smoke", Smoke_Current_Reading);
Particle.variable("Light_Button", Light_Button);
Particle.variable("Fan_Button1", Fan_Button1);
Particle.variable("Fan_Button2", Fan_Button2);
Particle.variable("Fan_Button3", Fan_Button3);
}

void loop() {
//Get the current temp and smoke readings
Temp_Current_Reading = analogRead(Temp_Pin);
Smoke_Current_Reading = analogRead(Smoke_Pin);

//Read the current fan/light button settings
Light_Button = (digitalRead(Light_Button_Pin)==LOW);
Fan_Button1 = (digitalRead(Fan_Button1_Pin)==LOW);
Fan_Button2 = (digitalRead(Fan_Button2_Pin)==LOW);
Fan_Button3 = (digitalRead(Fan_Button3_Pin)==LOW);

//Smoke readings periodic publish loop
if (Publish_Values_Flag==true) {
String Smoke_Publish = String(Smoke_Current_Reading);
Particle.publish("Smoke", Smoke_Publish, PRIVATE);
Publish_Values_Flag=false;
}

//Serries of if-statements to turn the light on or off based on the button
if (Light_Button == true){
    digitalWrite(Light_Relay, HIGH);
}
else if (Light_Button == false){
    digitalWrite(Light_Relay, LOW);
}

// Series of if tests to override auto mode and set fan speed based on manual button settings
if (Fan_Button1 == true or Fan_Button2 == true or Fan_Button3 == true){
    Button_Mode = true;
    Status = "Button_Mode";
}
if (Button_Mode == true){
    if (Fan_Button1 == true){
        digitalWrite(Fan_Relay1, HIGH);
    }
    else if (Fan_Button1 == false){
        digitalWrite(Fan_Relay1, LOW);
    }
    if (Fan_Button2 == true){
        digitalWrite(Fan_Relay2, HIGH);
    }
    else if (Fan_Button2 == false){
        digitalWrite(Fan_Relay2, LOW);
    }    
    if (Fan_Button3 == true){
        digitalWrite(Fan_Relay3, HIGH);
    }
    else if (Fan_Button3 == false){
        digitalWrite(Fan_Relay3, LOW);
    }
    if (Fan_Button1 == false and Fan_Button2 == false and Fan_Button3 == false){
        digitalWrite(Fan_Relay1, LOW);
        digitalWrite(Fan_Relay2, LOW);
        digitalWrite(Fan_Relay3, LOW);
        Button_Mode = false;
        Status = "Fan Off";
        Fan_Speed = 0;
    }
}

//Series of if-statements/tests to adjust the fan speed based on current temp and smoke readings
if (Speed_Check == true and Button_Mode == false){
    if (Fan_Speed == 0){
        if (Smoke_Current_Reading < Smoke01 or Temp > Temp01 ){
            digitalWrite(Fan_Relay1,HIGH);
            digitalWrite(Fan_Relay2,LOW);
            digitalWrite(Fan_Relay3,LOW);
            Fan_Speed = 1;
            Status = "Fan Low";
        }
    }
    else if (Fan_Speed == 1){
            if (Smoke_Current_Reading < Smoke12 or Temp > Temp12 ){
            digitalWrite(Fan_Relay1,LOW);
            digitalWrite(Fan_Relay2,HIGH);
            digitalWrite(Fan_Relay3,LOW);
            Fan_Speed = 2;
            Status = "Fan Med";
            }
            else if (Smoke_Current_Reading > Smoke10 and Temp <Temp10 ){
            digitalWrite(Fan_Relay1,LOW);
            digitalWrite(Fan_Relay2,LOW);
            digitalWrite(Fan_Relay3,LOW);
            Fan_Speed = 0;
            Status = "Fan Off";
        }
    }
    else if (Fan_Speed == 2){
        if (Smoke_Current_Reading < Smoke23 or Temp > Temp23 ){
            digitalWrite(Fan_Relay1,LOW);
            digitalWrite(Fan_Relay2,LOW);
            digitalWrite(Fan_Relay3,HIGH);
            Fan_Speed = 3;
            Status = "Fan High";
        }
        else if (Smoke_Current_Reading > Smoke21 and Temp <Temp21 ){
            digitalWrite(Fan_Relay1,HIGH);
            digitalWrite(Fan_Relay2,LOW);
            digitalWrite(Fan_Relay3,LOW);
            Fan_Speed = 1;
            Status = "Fan Low";
        }    
    }
    else if (Fan_Speed == 3){
        if (Smoke_Current_Reading > Smoke32 and Temp < Temp32 ){
            digitalWrite(Fan_Relay1,LOW);
            digitalWrite(Fan_Relay2,HIGH);
            digitalWrite(Fan_Relay3,LOW);
            Fan_Speed = 2;
            Status = "Fan Med";
        }    
    }
    Speed_Check = false;
}
}

//Script to average the temp readings.
void Temp_Average()
{
//Average the Temp readings to help reduce noise
//Subtract the last Temp reading:
Temptotal = Temptotal - Tempreadings[TempreadIndex];
//Read from the Temp sensor:
Tempreadings[TempreadIndex] = Temp_Current_Reading;
//Add the Temp reading to the total:
Temptotal = Temptotal + Tempreadings[TempreadIndex];
//Advance to the next position in the array:
TempreadIndex = TempreadIndex + 1;
//If we're at the end of the array...
if (TempreadIndex >= TempnumReadings) {
//...wrap around to the beginning:
TempreadIndex = 0;
}
if (Tempcounter < TempnumReadings) {
    Tempcounter = Tempcounter + 1;
    Tempaverage = Temptotal / Tempcounter;
}
    else {
    //Calculate the average Temp reading:
    Tempaverage = Temptotal / TempnumReadings;
}

//Calculate temperature using the average read value
TempC=((100*((float)Tempaverage/4095)*3.3)-50); //Pin1=Power, Pin2=Vout, Pin3=Ground
TempF=((TempC*9/5)+32);
Temp=TempF; //Select TempF or TempC
}

void Fan_Check()
{
Speed_Check = true;
}

void Smoke_Check()
{
Publish_Values_Flag=true;
}