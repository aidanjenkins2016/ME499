
#include <Servo.h>
#include <TFT.h>  // Arduino LCD library
#include <SPI.h>

// pin definition for SPI
#define cs   10
#define dc   9
#define rst  8

//create instance
Servo channel1out, channel2out, channel3out;
TFT TFTscreen = TFT(cs, dc, rst);

volatile int autonomous_flag = 0; // 0==human, 1==autonomous
volatile unsigned long timer_start1, timer_start2, timer_start3;
volatile int pulse_time1, pulse_time2, pulse_time3;
volatile int old_interrupt_time1, old_interrupt_time2, old_interrupt_time3;
int channel1in = 21, channel2in = 22 , channel3in = 23 ; //input pin
int channel1out_pin = 2, channel2out_pin = 3;
int steering_center = 1514, throttle_center = 1498; //figure out a way to read this in

//screen variables
char channel1Printout[5], channel2Printout[5];


//Interrupts for rx channels
void readChannel1() {
  //ISR for input pin
  old_interrupt_time1 = micros(); //get time since start of the loop
  if (digitalRead(channel1in) == HIGH)
  {
    timer_start1 = micros(); //if input pin is high get time
  }
  else
  {
    if (timer_start1 != 0)
    {
      /*if the timer start is nonzero, that means you are endin a pulse
         and need to do math to get the net time in the pulse
      */
      pulse_time1 = micros() - timer_start1; //new pulse width in microseconds

      if (autonomous_flag == 0)
      {
        channel1out.writeMicroseconds(pulse_time1);
      }

      timer_start1 = 0; //reset timer
    }
  }

}

void readChannel2() {
  //ISR for input pin
  old_interrupt_time2 = micros(); //get time since start of the loop
  if (digitalRead(channel2in) == HIGH)
  {
    timer_start2 = micros(); //if input pin is high get time
  }
  else
  {
    if (timer_start2 != 0)
    {
      /*if the timer start is nonzero, that means you are endin a pulse
         and need to do math to get the net time in the pulse
      */
      pulse_time2 = micros() - timer_start2; //new pulse width in microseconds

      if (autonomous_flag == 0)
      {
        channel2out.writeMicroseconds(pulse_time2);
      }

      timer_start2 = 0; //reset timer
    }
  }

}

void readChannel3() {
  //ISR for input pin
  old_interrupt_time3 = micros(); //get time since start of the loop
  if (digitalRead(channel3in) == HIGH)
  {
    timer_start3 = micros(); //if input pin is high get time
  }
  else
  {
    if (timer_start3 != 0)
    {
      /*if the timer start is nonzero, that means you are endin a pulse
         and need to do math to get the net time in the pulse
      */

      pulse_time3 = micros() - timer_start3; //new pulse width in microseconds

      //control autonomous signal
      if (pulse_time3 <= 1500)
      {
        autonomous_flag = 0;
      }
      else
      {
        autonomous_flag = 1;
        //do nothing
      }
      //channel3out.writeMicroseconds(pulse_time3);
      timer_start3 = 0; //reset timer
    }
  }

}


void setup() {
  // put your setup code here, to run once:

  //screen stuff
  TFTscreen.begin();
  // clear the screen with a black background
  TFTscreen.background(0, 0, 0);

  // write the static text to the screen
  // set the font color to white
  TFTscreen.stroke(255, 255, 255);
  // set the font size
  TFTscreen.setTextSize(1);
  TFTscreen.text("Channel 1 us:\n ", 32, 0);
  TFTscreen.text("Channel 2 us:\n ", 32, 8);

  //initialize timers
  timer_start1 = 0; timer_start2 = 0; timer_start3 = 0;

  //config pins as inputs
  pinMode(channel1in, INPUT_PULLUP); pinMode(channel2in, INPUT_PULLUP); pinMode(channel3in, INPUT_PULLUP);//config read pin as input

  //output signal pins
  channel1out.attach(channel1out_pin); channel2out.attach(channel2out_pin); //channel3out.attach(23);

  //assign input interrupts
  attachInterrupt(digitalPinToInterrupt(channel1in), readChannel1, CHANGE); //assign interrupt to pin
  attachInterrupt(digitalPinToInterrupt(channel2in), readChannel2, CHANGE); //assign interrupt to pin
  attachInterrupt(digitalPinToInterrupt(channel3in), readChannel3, CHANGE); //assign interrupt to pin
  //Serial.begin(115200); //serial communication

}

uint32_t timer = millis();
int erased=1;

void loop() {
  // put your main code here, to run repeatedly:

  if (autonomous_flag == 1)
  {
    channel1out.writeMicroseconds(steering_center);
    channel2out.writeMicroseconds(throttle_center);
    //do stuff in autonomous mode

    //need another flag for learning points vs just driving around
    //look like 2 or 3 points ahead
    //create vector for point to point,
    //check current pos often
    //PID steering
  }

  if (timer > millis()) timer = millis(); //rollover
  
  if (erased==1) {
   

    String channel1Val = String(pulse_time1);
    String channel2Val = String(pulse_time2);

    // convert the reading to a char array
    channel1Val.toCharArray(channel1Printout, 5);
    channel2Val.toCharArray(channel2Printout, 5);


    // set the font color
    TFTscreen.stroke(255, 255, 255);
    // print the sensor value
    TFTscreen.text(channel1Printout, 32 + 80, 0);
    TFTscreen.text(channel2Printout, 32 + 80, 8);
    erased=0;
  }
  // wait for a moment
  if (millis() - timer > 250) {
    timer=millis();
    // erase the text you just wrote
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.text(channel1Printout, 32 + 80, 0);
    TFTscreen.text(channel2Printout, 32 + 80, 8);
    erased=1;
  }
  //print current I/O pulsewidths
  //Serial.print(pulse_time1);
  //Serial.print(" | ");
  //Serial.print(pulse_time2); //print pulse width in us
  //Serial.print(" | ");
  //Serial.println(pulse_time3);


}
