// versio 1.1 (1000 baudia)
// Kauko-ohjattavan veneen lähetinyksikkö
// koodin pituus: 20bit tauko, 16bit tunnus, 16bit koodi=70bit = käskyjen taajuus=2000/70=28.5 käskyä sekunnissa
// kellolaitekeskeytys 16mhz/2khz=8000 pulssin välein.
// prescaler 32, ja keskeytys 250 kohdalla. Käytetään TIMER2

// CS22 CS21 CS20 
// 0    1    1    Clock / 32


int radioPin=11;              
const int ledPin=13;
const int potPin=A7;
const int moottoriPin=12;
const int valoPin=10;

uint8_t perasin=32;
uint8_t moottori=0;
uint8_t valot=0,samaData=0;
uint8_t potikka=0;
uint16_t alkutunnus=0xC08B,edellinen;
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
  pinMode(valoPin, INPUT);
  pinMode(radioPin, INPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(radioPin, LOW);
  
  TCCR2A = _BV(WGM21); // CTC mode, eli katon voi määrätä OCR2 rekisterillä
  TCCR2B = _BV(CS21) | _BV(CS20); // prescaler=32
  OCR2A=250; // 2A Katto kun 250 pulssia 
  TIMSK2 |= _BV(OCIE2A); // keskeytyksen  aiheuttaa 2A rekisterin
}

void loop() {
  if (lahetettava==0xFFFFFFFF) {
    perasin=analogRead(potPin)/5;
    if (perasin>63) perasin-=64; else perasin=0;
    if (perasin>63) perasin=63;

    moottori=!digitalRead(moottoriPin);
    valot=digitalRead(valoPin);
    
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
//    for (int i=16;i>0;i--) Serial.print(o[i]);
//    Serial.println();
    
  }
}

SIGNAL(TIMER2_COMPA_vect){
  tauko++;
  digitalWrite(radioPin,tauko%2);
  if (tauko<20) return;                   // ensin lähetetään 20bit 10101010....
  if (keskeytysLaskuri<16) {              // tämän jälkeen 16bit tunnus 0xC08B=1100 0000 1000 1011, mikä lie tunnus
    digitalWrite(radioPin,alkutunnus&1);
    alkutunnus=alkutunnus>>1;  
  } else {
    digitalWrite(radioPin,lahetettava&1);  // sitten 24bit koodi. eli yhteensä 70bit. eli 
    lahetettava=lahetettava>>1;    
  }
  keskeytysLaskuri++;
  if (keskeytysLaskuri==40) {
    keskeytysLaskuri=0;
    tauko=0;
    lahetettava=0xFFFFFFFF;
    alkutunnus=0xC08B;
  }  
}
