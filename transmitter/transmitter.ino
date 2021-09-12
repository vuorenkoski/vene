// versio 1.1 (1000 baudia)
// Kauko-ohjattavan veneen lähetinyksikkö
// koodin pituus: 20bit tauko, 8bit tunnus, 24bit koodi=52bit = käskyjen taajuus=1000/52=19.2 käskyä sekunnissa
// kellolaitekeskeytys 16mhz/1khz=16000 pulssin välein.
// prescaler 64, ja keskeytys 250 kohdalla. Käytetään TIMER2

// CS22 CS21 CS20 
// 0    1    1    Clock / 32
// 1    0    0    Clock / 64

int radioPin=11;              
const int ledPin=13;
const int potPin=A7;
const int moottoriPin=12;
const int valoPin=10;

uint8_t perasin=32;
uint8_t moottori=0;
uint8_t valot=0,valonappi=1;
uint8_t alkutunnus=0xCC;
uint32_t lahetettava=0xFFFFFFFF;
uint8_t keskeytysLaskuri=0, tauko=0;
uint8_t o[17];

static uint8_t symbols[] =
{
    0xd,  0xe,  0x13, 0x15, 0x16, 0x19, 0x1a, 0x1c, 
    0x23, 0x25, 0x26, 0x29, 0x2a, 0x2c, 0x32, 0x34
};

void setup() {
//  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(potPin, INPUT);
  pinMode(moottoriPin, INPUT);
  pinMode(valoPin, INPUT_PULLUP);
  pinMode(radioPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(radioPin, LOW);
  
  TCCR2A=2; // CTC mode, eli katon voi määrätä OCR2 rekisterillä
//  TCCR2B=3; // prescaler=32
  TCCR2B=4; // prescaler=64

  OCR2A=250; // 2A Katto kun 250 pulssia 
  TIMSK2 |= _BV(OCIE2A); // keskeytyksen  aiheuttaa 2A rekisterin
}

void loop() {
  if (lahetettava==0xFFFFFFFF) {
    perasin=analogRead(potPin)/5;
    if (perasin>60) perasin-=60; else perasin=0;
    if (perasin>63) perasin=63;
    moottori=!digitalRead(moottoriPin);
    if (digitalRead(valoPin)==0) {
      if (valonappi==1) {
        valot=!valot;
        valonappi=0;
      }
    } else valonappi=1;
    
    o[3]=perasin%2;
    o[5]=(perasin>>1)%2;
    o[6]=(perasin>>2)%2;
    o[7]=(perasin>>3)%2;
    o[9]=(perasin>>4)%2;
    o[10]=(perasin>>5)%2;
    o[11]=moottori;
    o[12]=valot;
    o[13]=moottori; // reserved
    o[14]=moottori; // reserved
    o[15]=0; // aina 0
  
    o[1]=(o[3]+o[5] +o[7] +o[9] +o[11]+o[13]+o[15])%2;
    o[2]=(o[3]+o[6] +o[7] +o[10]+o[11]+o[14]+o[15])%2;
    o[4]=(o[5]+o[6] +o[7] +o[12]+o[13]+o[14]+o[15])%2;
    o[8]=(o[9]+o[10]+o[11]+o[12]+o[13]+o[14]+o[15])%2;
    o[16]=(o[1]+o[2]+o[3]+o[4]+o[5]+o[6]+o[7]+o[8]+o[9]+o[10]+o[11]+o[12]+o[13]+o[14]+o[15])%2;

    lahetettava=0;
    for (int i=0;i<4;i++) { // jokainen 4bit ryhmästä tehdään 6bit ryhmä jossa tasainen 0-1 jakauma. Viesti yhteensä 24bit
      lahetettava<<=6;
      lahetettava+=symbols[o[i*4+1]+o[i*4+2]*2+o[i*4+3]*4+o[i*4+4]*8];
    }
//    Serial.println(lahetettava);
  }
}

SIGNAL(TIMER2_COMPA_vect){
  if (tauko<20) {
    tauko++;
    digitalWrite(radioPin,tauko%2);
    return;                   // ensin lähetetään 20bit 10101010....
  }
  if (keskeytysLaskuri<8) {              // tämän jälkeen 8bit tunnus 0xCC=1100 1100
    digitalWrite(radioPin,alkutunnus&1);
    alkutunnus=alkutunnus>>1;  
  } else {
    digitalWrite(radioPin,lahetettava&1);  // sitten 24bit koodi. eli yhteensä 52bit. bitti0 lähetetään ensimmäiseksi 
    lahetettava=lahetettava>>1;    
  }
  keskeytysLaskuri++;
  if (keskeytysLaskuri==32) {
    keskeytysLaskuri=0;
    tauko=0;
    lahetettava=0xFFFFFFFF;
    alkutunnus=0xCC;
  }  
}
