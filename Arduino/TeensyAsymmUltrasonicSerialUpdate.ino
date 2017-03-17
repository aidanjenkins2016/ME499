
#define outPin A21 //DAC pins are A21 and A22
#include <math.h>

//----------------------------------------------------------------------------------
//*********************************Variables****************************************
//----------------------------------------------------------------------------------

//Skew Signal Variables-----------------------------------
//--------------------------------------------------------

//Skew variables (CHANGE THESE PARAMETERS FOR TESTING)
float shape = 1; //well(1),hill(2)
float xmid1=9;
float xmid2=11;
float width1=4;
float width2=6;

//Gaussian parameters
float a = 1; //Height of Gaussian
float b1 = xmid1; //Location of shape
float b2 = xmid2; //Location of shape
float c1 = width1/6; //Width of shape 1
float c2 = width2/6; //Width of shape 2

//Normalization variables
float max1 = 0;
float max2 = 0;

//General Variables --------------------------------------
//--------------------------------------------------------

//Timer Variables
IntervalTimer myTimer1;
IntervalTimer myTimer2;

//LED and button
const int buttonPin = 35;     // the number of the pushbutton pin
const int ledPin1 =  37;      // the number of the LED pin
const int ledPin2 =  36;
boolean buttonState = true;         // variable for reading the pushbutton status
unsigned long debouncing_time = 50; //ms
volatile unsigned long last_micros;

//Other Variables
float freq=40; //vibration frequency
float Fs=10000; //sampling frequency
float Pi = 3.14159;
float skew = 0.5;
volatile int level;
float t = 0;
float t_max=1/freq;
float w1=skew*t_max, w2=(1-skew)*t_max;
volatile bool play_flag; // false == unplayed  | true == played

//Serial Communication
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];  // temporary array for use when parsing

// variables to hold the parsed data
char messageFromPC[numChars] = {0};
float serial_width1 = 0.0, serial_width2 = 0.0;
boolean newData = false;

//Ultrasonic Variables------------------------------------
//--------------------------------------------------------

const int TRIG_PIN = 24;
const int ECHO_PIN = 25;

unsigned long t1;
unsigned long t2;
unsigned long pulse_width;
float cm;
float in;

// Anything over 400 cm (23200 us pulse) is "out of range"
const unsigned int MAX_DIST = 23200;


//----------------------------------------------------------------------------------
//***********************************Setup******************************************
//----------------------------------------------------------------------------------

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  analogWriteResolution(12); //Max 13 bit DAC resolution, 12bit=4096 levels
  analogReadResolution(12); // reading from potentiometer for testing
  pinMode(outPin, OUTPUT); //config output pin

  // initialize the LED pins as outputs:
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  // The Trigger pin will tell the ultrasonic sensor to range find
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(ECHO_PIN, INPUT);

  // Setup Interrupts
  attachInterrupt(digitalPinToInterrupt(ECHO_PIN),echo_timer,CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttonPin), button_debouncer, RISING);
  int interrupt_callback_time = (1 / Fs) * 1000000;
  myTimer1.begin(play_level, (int)interrupt_callback_time);
  //myTimer2.begin(print_dist, 60000); // interrupt for printing over serial

  //Setup normalization
  normalize_skew_function();
  
  delay(3000);
}

//----------------------------------------------------------------------------------
//*********************************Main Loop****************************************
//----------------------------------------------------------------------------------

void loop() {

  //check for serial data
  recvWithStartEndMarkers();
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0
    parseData();
    showParsedData();//Send received data back for integrity?
    reset_inputs(serial_width1,serial_width2);
    newData = false;
  }

  //Calculate Skew Signal
  if (play_flag) {
    t = t + 1 / Fs;
    if (t > t_max) {
      t = 0;
      
      w1= skew*t_max;
      w2= (1-skew)*t_max;
    }
    level = sample_level(t,w1,w2);
    //Serial.println(t);
  }

  //Get Position from Ultrasonic Sensor
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10); // Hold the trigger pin high for at least 10 us
  digitalWrite(TRIG_PIN, LOW);

  //Calculate distance in centimeters and inches.
  //cm = pulse_width / 58.0;
  in = pulse_width / 148.0;

  //Update Skew
  if ( pulse_width > MAX_DIST ) {
    //Serial.println("Out of range");
  } 
  else {
    //Test Case 1
    if (buttonState == true){
      skew_calc(shape,b1,c1,max1,in);
      digitalWrite(ledPin1, HIGH);
      digitalWrite(ledPin2, LOW);
    }
    //Test Case 2
    else{
      skew_calc(shape,b2,c2,max2,in);
      digitalWrite(ledPin1, LOW);
      digitalWrite(ledPin2, HIGH);
    }
    //Serial.println(skew,4);
    //delay(500);
  }
}

//----------------------------------------------------------------------------------
//**************************Function Prototypes*************************************
//----------------------------------------------------------------------------------

//Serial Functions----------------------------------------
//--------------------------------------------------------

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

//--------------------------------------------------------

void parseData() {      // split the data into its parts

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ",");     // get the first part - the string
  //strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC
  serial_width1 = atof(strtokIndx);

  strtokIndx = strtok(NULL, ",");
  serial_width2 = atof(strtokIndx);     // convert this part to a float
}

//--------------------------------------------------------

void showParsedData() {
  //    Serial.print("Message ");
  //    Serial.println(messageFromPC);
//  Serial.print("serial_width1 ");
  //Serial.print(serial_width1,4);
  //Serial.print(", ");
  //Serial.println(serial_width2,4);
}

//Matlab Input Reset--------------------------------------
//--------------------------------------------------------

void reset_inputs(float width1_new, float width2_new) {
  
  xmid1 = random(1,101);
  xmid1 = xmid1/100;
  xmid1 = xmid1 + random(7,11);

  xmid2 = random(1,101);
  xmid2 = xmid2/100;
  xmid2 = xmid2 + random(7,11);

  b1 = xmid1;
  b2 = xmid2;
  
  width1 = width1_new;
  width2 = width2_new;

  c1 = width1/6; //Width of shape
  c2 = width2/6;
  
  max1 = 0;
  max2 = 0;

  normalize_skew_function();

  buttonState = true;
  
}

//Debounce Function---------------------------------------
//--------------------------------------------------------

void button_debouncer() {
  if ((long)micros() - last_micros >= debouncing_time * 1000) {
    buttonState = !buttonState;
    last_micros = micros();
  }
}

//Skew Normalize Function---------------------------------
//--------------------------------------------------------

void normalize_skew_function(){
  
  float dx = 0.005;
  float dy1;
  float dy2;
  
  for (float x=0; x<20; x=x+dx){
    dy1 = (a*(b1-x)*exp(-sq(x-b1)/(2*sq(c1))))/sq(c1);
    dy2 = (a*(b2-x)*exp(-sq(x-b2)/(2*sq(c2))))/sq(c2);
    
    if (dy1 > max1){
      max1 = dy1;
    }
    
    if (dy2 > max2){
      max2 = dy2;
    }
 }
}

//Ultrasonic Echo Timer Function--------------------------
//--------------------------------------------------------

void echo_timer(){
  if (digitalRead(ECHO_PIN) == 1){
    t1 = micros();
  }
  else{
    t2 = micros();
    pulse_width = t2 - t1;
  }
}

//Play Level Function-------------------------------------
//--------------------------------------------------------

void play_level(void) {
  //play value on analog out pin
  //Serial.println("interrupt");
  if (!play_flag) {
    analogWrite(outPin, level);
    play_flag = true;
  }
}

//Skew Calculation Function-------------------------------
//--------------------------------------------------------

void skew_calc(int shape, float b, float c, float normalize, float x) {
  //Calculates skew based on current position and derivative of Gaussian function

  //Well
  if (shape == 1) {
    skew = -a*(b-x)*exp(-sq(x-b)/(2*sq(c)))/sq(c);
    skew = 0.5+(0.5*skew)/normalize;
  }
  //Hill
  else{
    skew = a*(b-x)*exp(-sq(x-b)/(2*sq(c)))/sq(c);
    skew = 0.5+(0.5*skew)/normalize;
  }
}

//Sample Level Function-----------------------------------
//--------------------------------------------------------

int sample_level(float t, float w1, float w2) {
  //do math to create sample
  //do we want to compute period stuff in here?
  //returning ampl level here so we can play a tone separately to
  //asynchronously compute the next output level
  float ampl;
  if (t <= w1 / 2) {
    ampl = sin( t * Pi / w1 ) * 2048 + 2048; // 4096/2=2048
  }
  else if ( (t > w1 / 2) && (t <= (w1 / 2 + w2) )) {
    ampl = sin(t * Pi / w2 + (Pi / 2) * (1 - w1 / w2)) * 2048 + 2048;

  }
  else if ( (t > w1 / 2 + w2) && (t <= w1 + w2)) {
    ampl = sin(t * Pi / w1 + Pi * (1 - w2 / w1)) * 2048 + 2048;
  }

  play_flag = false; // set flag to we know this level hasn't played

  return (int)ampl ;//needs to be int, discrete val for DAC
}

//Print Interrupt-----------------------------------------
//--------------------------------------------------------

void print_dist(void){
  //Serial.println(in,4);
}
