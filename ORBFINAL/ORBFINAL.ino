
/***********
Journey Through Light Orb
Authors: Ace Levenberg(llevenbe@ucsc.edu) and Aidan Seine (aseine@ucsc.edu)
5/12/2012
See Color library for more information
This sketch uses the VirtualWire library radio transmission written by:
Mike McCauley (mikem@open.com.au) 
***************/


#include <VirtualWire.h>
#include <color.h>
#include <math.h>

/**************
//dont forget to change the id!!!!!
ids used = a, b, e, c
**********
*/

typedef struct{
  boolean seen;
  long int timeSeen;
  char color;
} orb;


const char id = 'g';
orb orbs[26];
int numberOfOrbs = 0;
int siteRate;
//long int prevOrbTime[26];
char sites[10];
long int prevSiteTimes[10];
int colorCounter = 0;
int colors[7];
//char orbColors[26];
color normColor = color('1');
color siteColor = color('/0');
int sendCounter = random(600, 1000);
int transPin = 4;
long int prevSendTime = 0;

void setup(){
  Serial.begin(9600);
  //initialize the timout times for coming into contact with sites
  for(int i = 0; i < 10; i++){
    prevSiteTimes[i] = 0;
  }
  //initialize the timeout ties for coming into contact with other orbs
  for(int i = 0; i < 26; i++){
   // prevOrbTime[i] = 0;
    orbs[i].seen = false;
  }
  for(int i = 0; i < 7; i++){
    colors[i] = 0;
  }
  colors[0] = INFINITY;
  pinMode(transPin, OUTPUT);
  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(4000);	 // Bits per sec
  vw_rx_start();       // Start the receiver PLL running
}

void loop(){
  //buffers for incoming messages
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  //if the send timer is up send a message
  if(checkCounter()) orbSend();
  //if we are still at a site blink the siteColor
  if(checkAtSite()) blink(normColor, siteRate);
  //check to see if we timed out from other orbs
  else {
    checkTimeOut();
    blink(getColor(), numberOfOrbs);
  }
  //wait for a message for a randomtime between 500 or 750 milliseconds
  if (vw_wait_rx_max(random(500, 750))){
    //if you get a message
    if (vw_get_message(buf, &buflen)){
      //debug prints, prints the ID of the Orb it recieved and the number of Orbs
      Serial.print("Number of Orbs if got message  ");
      Serial.println(numberOfOrbs);
      Serial.print("id of orb or site = ");
      Serial.println(char(buf[2]) );
      Serial.print("color of orb ");
      Serial.println(char(buf[1]));
     
      //if the message is a site message
      if (buf[0] == 's') site(buf[1], buf[2], buf[3]);
      //if the message is just another orb
      else {
        int Orb = buf[2]-97;
        if(!orbs[buf[2]-97].seen){
          orbs[Orb].seen = true;
          orbs[Orb].timeSeen = millis();
          orbs[Orb].color = buf[1];
          colors[buf[1]-48]++;
        }
        else {
          orbs[Orb].timeSeen = millis();
          if(orbs[Orb].color != buf[1]){
            colors[orbs[Orb].color-48]--;
            orbs[Orb].color = buf[1];
            colors[buf[1]-48]++;
          }
        }
        
        /*prevOrbTime[buf[2] - 97] = millis();
        orbColors[buf[2] - 97] = buf[1];*/
      }
    }
  }
 // else Serial.println("fail");//if we are at a site and we did not recieve a message 
  /*else if(numberOfOrbs > 0) {
    blink(getColor(), numberOfOrbs);
  }*/
  //send the message if the counter of the send time is up 
 if(checkCounter()) orbSend();
}
/*
turns the transmitter on
puts together the message needed to be sent by the Orb
message is of length 3.  
turns the transmitter off
*/
void orbSend(){
  digitalWrite(transPin, HIGH);
  //message is a string with 3 characters
  char msg[3];
  //orb identifier
  msg[0] = 'o';
  //the color of the orb
  msg[1] = normColor.colorNum;
  //the individual id
  msg[2] = id;
  vw_send((uint8_t *)msg, strlen(msg));
  vw_wait_tx();
  digitalWrite(transPin, LOW);
}

color getColor(){
  while(1){
    colorCounter = (colorCounter >= 6 ) ? 0 : colorCounter + 1;
    if(colors[colorCounter]) {
      Serial.print("color counter = ");
      Serial.println(colorCounter);
      return color(colorCounter + 48);
    }
  }
}

/*
calls pulse function with maped delay time (dpending on number of orbs)
maps the fade rate depending on the number of orbs
*/
void blink(color hue, int rate){
  
  pulse(hue, map(rate, 0, 6, 40, 10), map(rate,0,6,5,10));
}

/*
checks all of the sites timeout times to see if the Orb has been to a site 
and the timeout time is not up
*/
boolean checkAtSite(){
  long int currTime = millis();
   for(int i = 0; i < 10; i++){
		//if we have never been to a site before the prevTime is zero
     if(prevSiteTimes[i] != 0){
      if(currTime - prevSiteTimes[i] < 20000) return true;
    }
  }
  return false;
}


/*
call when the Orb is at a site
the Orb will blink the color the site tells it to
it will also blink at the speed the site tells it to
*/
void site(char Color, char people, char siteId){ 
  //set the timoue value for a site to 3.
  //if it is not already in the site array 
  if (prevSiteTimes[siteId-48] == 0 ){
    //update the color to the new color if there is more than one person at the site and we have not yet een to the site
    normColor = (normColor.colorNum == '6') ? normColor : color(normColor.colorNum + 1);
  }  
 prevSiteTimes[siteId-48] = millis(); 
  /*if(people > '1') {
    prevSiteTimes[siteId-48] = millis();
  } */
  //set the site color equal to the color the site tells us to send.
  siteColor = color(Color);
  siteRate = people - 48;
  //blink with the siteColor and the SiteRate
  blink(normColor, siteRate);
}
/*
checks to see if the any of the Orbs this Orb has seen
have timed out.  Modifies the number of orbs to the needed
value
*/
void checkTimeOut(){
  numberOfOrbs = 0;
  long int currTime = millis();
  for(int i = 0; i < 26 ; i++){
    if(orbs[i].seen == true) {
    //if the certain value is greater than zero
      if(currTime - orbs[i].timeSeen < 20000) numberOfOrbs = (numberOfOrbs >= 7) ? 7 : numberOfOrbs+1;
      else {
        colors[orbs[i].color-48]--;
        orbs[i].seen = false;
      }
    }
  }
  //blink(normColor, numberOfOrbs);
  
}
/*
pulses the color given with the given speed and fade value
*/
void pulse(color hue, int rate, int fadeRate){
        
        if(checkCounter())orbSend();
        //start fading up
        
	for (int fadeValue = 255; fadeValue >= 0 ; fadeValue -= fadeRate){
                //map the value from the current red value - fadeValue, from the highest red value for lowest red value to the highest
                //to 0 to the highest fadevalue so the colors pulse correctly
		analogWrite(REDPIN, map(hue.red-fadeValue, hue.red-255, hue.red, 0, hue.red));
                //same for green pin
		analogWrite(GREENPIN, map(hue.green-fadeValue, hue.green-255, hue.green, 0, hue.green));
                //same for blue pin
		analogWrite(BLUEPIN, map(hue.blue-fadeValue, hue.blue-255, hue.blue, 0, hue.blue));	
		delay(rate);
                if(checkCounter())orbSend();
               	
	}
        //send the message
        //fade back down
	for (int fadeValue = 0; fadeValue <= 255; fadeValue += fadeRate){
		analogWrite(REDPIN, map(hue.red-fadeValue, hue.red-255, hue.red, 0, hue.red));
		analogWrite(GREENPIN, map(hue.green-fadeValue, hue.green-255, hue.green, 0, hue.green));
		analogWrite(BLUEPIN, map(hue.blue-fadeValue, hue.blue-255, hue.blue, 0, hue.blue));
		delay(rate);
                if(checkCounter())orbSend();		
	}
        analogWrite(REDPIN, 0);
        analogWrite(GREENPIN, 0);
	analogWrite(BLUEPIN, 0);
        //send one more time.
        if(checkCounter()) orbSend();
}

/*
checks to see if the timer for sending is up
if it is return true, if it is ot return false
*/
boolean checkCounter(){
  long int currTime = millis();
  if(currTime - prevSendTime >= sendCounter){
    sendCounter = random(600, 1000);
    prevSendTime = millis();
    return true;
  }
  return false;
}

