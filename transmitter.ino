// veneen lähetin

#include <VirtualWire.h>
// virtual viren timer vaihdettu timer1->timer2
 
int RF_TX_PIN = 2;              
const int ledPin=13;
const int potPin=A0;
unsigned char buffer[64];
int perasin=57;
int moottori=0;
int potikka=0;

void setup()
{
  Serial.begin(9600);
  vw_set_tx_pin(RF_TX_PIN); // Setup transmit pin
  vw_setup(2000); // Transmission speed in bits per second.
  pinMode(ledPin, OUTPUT);
}

void loop()
{

  if (Serial.available()>0) {
    char incomingByte = Serial.read();
    if (incomingByte=='a' && perasin>29) perasin=perasin-2;
    if (incomingByte=='d' && perasin<85) perasin=perasin+2;
    if (incomingByte=='w') moottori=1;
    if (incomingByte=='s') moottori=0;
  }    
//  perasin=29+analogRead(potPin)/18;
  buffer[0]=perasin+(moottori*100);
  vw_send(buffer,1);
  vw_wait_tx(); // ehkä näin 10ms
}
