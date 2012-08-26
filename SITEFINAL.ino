
/***********
Journey Through Light Site
Authors: Ace Levenberg(llevenbe@ucsc.edu) and Aidan Seine (aseine@ucsc.edu)
5/12/2012
See Color library for more information
This sketch uses the VirtualWire library radio transmission written by:
Mike McCauley (mikem@open.com.au) 
***************/

#include <VirtualWire.h>
#include <color.h>
/**************
//dont forget to change the id!!!!!
ids used = 
**********
*/
const char id = '7';
int numberOfOrbs = 0;
int siteRate = 0;
int orbId[26];
long int prevOrbTime[26];
char highColor = '0';
int sendCounter = random(400, 800);
int transPin = 4;
long int prevSendTime = 0;

void setup(){
  Serial.begin(9600);
   //initialize the orb ids
  for(int i = 0; i < 26; i++){
    orbId[i] = 0;
  }
  pinMode(transPin, OUTPUT);
  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(4000);	 // Bits per sec
  vw_rx_start();       // Start the receiver PLL running
}

void loop(){
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  //wait until we recieve a message
  vw_wait_rx();
  //decrement all of the orb ids
  decIDs();
  //if we did get a message
  if (vw_get_message(buf, &buflen)){ // Non-blocking
    //debug print of the id of the orb recieved
    Serial.println(buf[2]);
    //if the id has not been seen beofre increment the number of orbs seen
    if(orbId[buf[2] - 97] == 0) numberOfOrbs++;
    //finds the highest color of all of the orbs present
    highColor = findMax(buf[1], highColor);
    //sets the timeout time
    orbId[buf[2] - 97] = 30000;
    //sets the time we have seen the orb
    prevOrbTime[buf[2] - 97] = millis();
      }
  //debug prints    
  Serial.print("Number of orbs");
  Serial.println(numberOfOrbs);
  if (numberOfOrbs > 0){
    blink(highColor, numberOfOrbs);
  }
}
/*
turns the transmitter on
puts together the message needed to be sent by the site
message is of length 4.  
turns the transmitter off
*/
  
  void siteSend(char Color, int peeps){
    digitalWrite(transPin, HIGH);
  char msg[4];
  char peepul = peeps + 48;
  msg[0] = 's';
  msg[1] = Color;
  msg[2] = peepul;
  msg[3] = id;
  for (int i = 0; i < 14; i++){
    vw_send((uint8_t *)msg, strlen(msg));
    vw_wait_tx();
  }
    digitalWrite(transPin, LOW);
}
  



//blink, maps the rate based on the number of orbs and then pulse the correct color and rate
void blink(char Color, int rate){
  color hue = color(Color);
  pulse(hue, map(rate, 0, 6, 40, 1), map(rate,0,6,5,15));
}






/***
decrements the time out array, this code is the old version of the timeout procedure
new procedure to be updated
******/
void decIDs(){
  //walks through every element
  for(int i = 0; i < 26 ; i++){
    long int currTime = millis();
    //debug print with orb 'A'
    if(i == 0){
    Serial.print("prev orb time");
    Serial.println(prevOrbTime[i]);
    Serial.print("orbId");
    Serial.println(orbId[i]);
    }
    //if the difference betwween the last time seen is greater than the time out time
    if(currTime - prevOrbTime[i] >= orbId[i]  && orbId[i] != 0) {
      Serial.print("made it");
      //set it back to zero
      orbId[i] = 0;
      //if it has been decremented all the way to zero, decrement the number of orbs
      numberOfOrbs = constrain(numberOfOrbs - 1, 0, 7);
      if(numberOfOrbs <= 0) highColor = '0';
    }  
  }
  
}
/*
pulses the color given with the given speed and fade value
*/
void pulse(color hue, int rate, int fadeRate){
 if(checkCounter()) siteSend(highColor, numberOfOrbs);
  for (int fadeValue = 245; fadeValue >= 0 ; fadeValue -= fadeRate){
    //map the value from the current red value - fadeValue, from the highest red value for lowest red value to the highest
   //to 0 to the highest fadevalue so the colors pulse correctly
    analogWrite(REDPIN, map(hue.red-fadeValue, hue.red-255, hue.red, 0, hue.red));
    analogWrite(GREENPIN, map(hue.green-fadeValue, hue.green-255, hue.green, 0, hue.green));
    analogWrite(BLUEPIN, map(hue.blue-fadeValue, hue.blue-255, hue.blue, 0, hue.blue));
    delay(rate);
    if(checkCounter()) siteSend(highColor, numberOfOrbs);

  }
 if(checkCounter())siteSend(highColor, numberOfOrbs);
  for (int fadeValue = 0; fadeValue <= 245; fadeValue += fadeRate){
    analogWrite(REDPIN, map(hue.red-fadeValue, hue.red-255, hue.red, 0, hue.red));
    analogWrite(GREENPIN, map(hue.green-fadeValue, hue.green-255, hue.green, 0, hue.green));
    analogWrite(BLUEPIN, map(hue.blue-fadeValue, hue.blue-255, hue.blue, 0, hue.blue));
    delay(rate);
    if(checkCounter()) siteSend(highColor, numberOfOrbs);
  }
  analogWrite(REDPIN, 0);
        analogWrite(GREENPIN, 0);
	analogWrite(BLUEPIN, 0);
  if(checkCounter()) siteSend(highColor, numberOfOrbs);
  
}
/* 
finds the max color of the orbs currently around
old procedure, new procedure to be updated
*/
char findMax(char cur, char maxColor){
  if (cur > maxColor){
    return cur;
  }
  return maxColor;
}
/*
checks to see if the timer for sending is up
if it is return true, if it is ot return false
*/
boolean checkCounter(){
  long int currTime = millis();
  if(currTime - prevSendTime >= sendCounter){
    sendCounter = random(700, 1500);
    prevSendTime = millis();
    return true;
  }
  return false;
}
