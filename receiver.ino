// Kauko-ohjattavan veneen vastaanotinyksikkö
// https://courses.cs.washington.edu/courses/csep567/10wi/lectures/Lecture7.pdf
// https://sites.google.com/site/qeewiki/books/avr-guide/timers-on-the-atmega328

#include <VirtualWire.h>
// VirtualWire muutettu niin että käyttää timer1 sijasta timer2
// servot käyttää timer1

// moottori 115=on, 108=OFF ehkä turvallisemmat 29 ja 26
// perasin 40+29 -- 40+85
// peräsimen kulma  40+57=0astetta. 

int RF_RX_PIN = 2;
int ledPin=13;
int perasinPin=9; //OC1A
int moottoriPin=10; //OC1B
int kulma=57;
int moottori=0;

void setup()
{
  vw_set_rx_pin(RF_RX_PIN);  // Setup receive pin.
  vw_setup(2000); // Transmission speed in bits per second.
  vw_rx_start(); // Start the PLL receiver.
  pinMode(ledPin, OUTPUT);
  pinMode(perasinPin, OUTPUT);
  pinMode(moottoriPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(moottoriPin, LOW);
                                                                
                                                                // COMA COMB -- WGM10  // WGM = Mode = 7 =111 // COM=10 (ON alhaalla)
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11) | _BV(WGM10); // 10   10   -- 11
  //7 Fast PWM, 10bit
                                                                             // FOC -- WGM2 CS    GM // CS=101=kerroin 1024
  TCCR1B = _BV(WGM12) | _BV(CS12); // prescaler 100=256                      // 00  -- 1    101    
  //timer 1 astukset: prescaler 256, COM=10 (set=0 clear=OCR1, WGM mode=7 (Fast PWM, 10bit)
  
  OCR1B = 108; //moottori off
  delay(1000);
  OCR1A = 40+29; // toinen laita
  delay(1000);
  OCR1A = 40+85; // toinen laita
  delay(1000);
  OCR1A = 40+57; // keskikohta
}
 
void loop()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  unsigned long pysaytys=0;
  
  while (true)
  {
    if(vw_get_message(buf, &buflen)) {
      pysaytys=0;
      kulma=buf[0]%100;
      moottori=buf[0]/100;
      digitalWrite(ledPin, HIGH);
      delay(1); 
      digitalWrite(ledPin, LOW);
      OCR1A=40+kulma;
      OCR1B=108+(7*moottori);
    } else if (moottori==1) {
      if (pysaytys==0) { 
        pysaytys=millis();
      } else if (millis()-pysaytys>500) {
        moottori=0;
        OCR1B=108+(7*moottori);
      }
    }
  }
}
