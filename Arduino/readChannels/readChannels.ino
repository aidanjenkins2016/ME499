
#include <Servo.h>

Servo channel1out, channel2out, channel3out;
const int ledPin = 13;
bool Serial_print = HIGH ;
volatile int autonomous_flag=0;// 0==human, 1==autonomous
volatile unsigned long timer_start1, timer_start2, timer_start3;
volatile int pulse_time1, pulse_time2, pulse_time3;
volatile int old_interrupt_time1, old_interrupt_time2, old_interrupt_time3;

//Pins
int channel1in = 10, channel2in=9 , channel3in=23 ; //input pin
int channel1out_pin = 16, channel2out_pin = 17;
int steering_center=1514, throttle_center=1498; //figure out a way to read this in


//Interrupts for rx channels
void readChannel1() {
  //ISR for input pin
  old_interrupt_time1=micros(); //get time since start of the loop
  if(digitalRead(channel1in)==HIGH)
  {
    timer_start1=micros(); //if input pin is high get time
  }
  else
  {
    if(timer_start1 != 0)
    {
      /*if the timer start is nonzero, that means you are endin a pulse
       * and need to do math to get the net time in the pulse
       */
      pulse_time1=micros()-timer_start1; //new pulse width in microseconds
      
      if(autonomous_flag==0)
      {
      channel1out.writeMicroseconds(pulse_time1);
      }
      
      timer_start1=0; //reset timer
     }
  }
  
}

void readChannel2() {
  //ISR for input pin
  old_interrupt_time2=micros(); //get time since start of the loop
  if(digitalRead(channel2in)==HIGH)
  {
    timer_start2=micros(); //if input pin is high get time
  }
  else
  {
    if(timer_start2 != 0)
    {
      /*if the timer start is nonzero, that means you are endin a pulse
       * and need to do math to get the net time in the pulse
       */
      pulse_time2=micros()-timer_start2; //new pulse width in microseconds
      
      if(autonomous_flag==0)
      {
      channel2out.writeMicroseconds(pulse_time2);
      }
      
      timer_start2=0; //reset timer
     }
  }
  
}

void readChannel3() {
  //ISR for input pin
  old_interrupt_time3=micros(); //get time since start of the loop
  if(digitalRead(channel3in)==HIGH)
  {
    timer_start3=micros(); //if input pin is high get time
  }
  else
  {
    if(timer_start3 != 0)
    {
      /*if the timer start is nonzero, that means you are endin a pulse
       * and need to do math to get the net time in the pulse
       */
            
      pulse_time3=micros()-timer_start3; //new pulse width in microseconds
      
      //control autonomous signal
      if(pulse_time3 <= 1500)
      {
        autonomous_flag=0;
      }
      else
      {
        autonomous_flag=1;
        //do nothing
      }   
      //channel3out.writeMicroseconds(pulse_time3);
      timer_start3=0; //reset timer
     }
  }
  
}


void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); 
  //initialize timers
  timer_start1=0;timer_start2=0;timer_start3=0;
  
  //config pins as inputs
  pinMode(channel1in, INPUT_PULLUP); pinMode(channel2in, INPUT_PULLUP); pinMode(channel3in, INPUT_PULLUP);//config read pin as input
  
  //output signal pins
  channel1out.attach(channel1out_pin); channel2out.attach(channel2out_pin); //channel3out.attach(23);
  
  //assign input interrupts
  attachInterrupt(digitalPinToInterrupt(channel1in), readChannel1, CHANGE); //assign interrupt to pin
  attachInterrupt(digitalPinToInterrupt(channel2in), readChannel2, CHANGE); //assign interrupt to pin
  attachInterrupt(digitalPinToInterrupt(channel3in), readChannel3, CHANGE); //assign interrupt to pin
  Serial.begin(9600); //serial communication

}

void loop() {
// put your main code here, to run repeatedly:

if(autonomous_flag==1)
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

if(Serial_print){
//print current I/O pulsewidths
Serial.print(pulse_time1);
Serial.print(" | ");
Serial.print(pulse_time2); //print pulse width in us
Serial.print(" | ");
Serial.println(pulse_time3);

delay(1000);
}

}
