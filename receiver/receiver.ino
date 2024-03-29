// versio 1.1 (1000 baudia)
// Kauko-ohjattavan veneen vastaanotinyksikkö
// lahettimen pulssi: prescaler 64, 250 pulssia = pulssi 16000 välein. 
// vastaanottimen pulssi 64 kertaa nopeampi, eli 250. Prescaler 1 ja katto 250
// millisekunnissa on yhteensä 16000 kellopulssia, eli 64 keskeytystä.
// 21.1.2020: lisätty vilkkuvalot

// board: Arduino pro mini

// lahettimeta tulee 19.2 käskyä sekunnissa, yhteensä 52bit koodi koko ajan peräkkäin.


// ver 9/2021: keskeytys tulee 8x125=1000 kellopulssin välein. Neljä kertaa hitaammin kuin aiemmin.

const uint8_t radioPin=2;
const uint8_t valo1Pin=3;
const uint8_t valo2Pin=4;
const uint8_t ledPin=13;
const uint8_t perasinPin=9;
const uint8_t moottoriPin=10;

uint16_t servopulssit=0,valopulssit=0,valoVaihe=1;
uint16_t perasin=25;
uint8_t moottori=28; 
uint8_t valot=0;
uint8_t sample,avrSample=0,edellinenSample=0,ramp=0,bitti=0,radioPreScaler=0,bittilkm,parsdata,virhe,bitti8=0;
boolean tulossa=false;
uint32_t tullutData=0xFFFFFFFF, data,data2;
uint8_t o[17];

static uint8_t symbols[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0,    1,    0xff,
  0xff, 0xff, 0xff, 2,    0xff, 3,    4,    0xff, 0xff, 5,    6,    0xff, 7,    0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 8,    0xff, 9,    10,   0xff, 0xff, 11,   12,   0xff, 13,   0xff, 0xff, 0xff,
  0xff, 0xff, 14,   0xff, 15,   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

unsigned long time;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(perasinPin, OUTPUT);
  pinMode(moottoriPin, OUTPUT);
  pinMode(valo1Pin, OUTPUT);
  pinMode(valo2Pin, OUTPUT);
  pinMode(radioPin, INPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(moottoriPin, LOW);
  digitalWrite(perasinPin, LOW);
  digitalWrite(valo1Pin, LOW);
  digitalWrite(valo2Pin, LOW);
//  Serial.begin(9600);
  digitalWrite(valo1Pin, HIGH);
  delay(1000);
  digitalWrite(valo1Pin, LOW);
  digitalWrite(valo2Pin, HIGH);
  delay(1000);
  digitalWrite(valo2Pin, LOW);

//timer1
  TCCR1A=0; // _BV(WGM21); // CTC mode, eli katon voi määrätä OCR2 rekisterillä, prescaler 1
  TCCR1B=2+8; // alkuperäinen oli 1+8; 2=8x prescaler
  OCR1A=125; // alkuperäinen oli 250
  TIMSK1=2; 
}

void loop() {
  if (tullutData!=0xFFFFFFFF) {
    data2=tullutData;
    tullutData=0xFFFFFFFF;
//    Serial.println(data2);

    for (int i=3;i>=0;i--) {  // data ryhmitelty 6bit ryhmiksi jossa 1-0 tasaisesti. Dataa yhdessä blokissa 4bit
      parsdata=symbols[data2&0x3F];   // havaitsee myös virheitä, jos 6bit koodi ei ole muunnostaulukossa
      if (parsdata==0xFF) break;
      data2>>=6;
      for (int j=1;j<=4;j++) {
        o[i*4+j]=parsdata&1;
        parsdata>>=1;
      }
    }
//        for (int i=16;i>0;i--) Serial.print(o[i]);
//        Serial.println();
        
    if (parsdata!=0xFF) {
      virhe=0;
      virhe+=(o[1]+o[3]+o[5]+o[7] +o[9] +o[11]+o[13]+o[15])%2;   // hamming koodi. korjaa 1bit muunnoksen ja havaitsee 2bit muunnoksen
      virhe+=2*((o[2]+o[3]+o[6]+o[7] +o[10]+o[11]+o[14]+o[15])%2);
      virhe+=4*((o[4]+o[5]+o[6] +o[7] +o[12]+o[13]+o[14]+o[15])%2);
      virhe+=8*((o[8]+o[9]+o[10]+o[11]+o[12]+o[13]+o[14]+o[15])%2);
      if (virhe>0) o[virhe]=1-o[virhe];
    
      virhe=(o[16]+o[1]+o[2]+o[3]+o[4]+o[5]+o[6]+o[7]+o[8]+o[9]+o[10]+o[11]+o[12]+o[13]+o[14]+o[15])%2;
  
      if (virhe==0) {
        perasin=15+(o[3]+2*o[5]+4*o[6]+8*o[7]+16*o[9]+32*o[10])/3; // Aiemmin 64
//        Serial.println(perasin);
        moottori=28+2*o[11]; //Aiemmin 114
        valot=o[12];
        digitalWrite(ledPin,valot);
        time=millis();
      }
    } else virhe=1;
//    if (virhe==1) digitalWrite(ledPin, HIGH);delay(1); digitalWrite(ledPin, LOW);

//    if (virhe==1) {
//      Serial.println("virhe");
//    } else {
//      for (int i=16;i>0;i--) Serial.print(o[i]);
//      Serial.println();
//    }
  }

  if (millis()-time>1000) moottori=28;
}

SIGNAL(TIMER1_COMPA_vect){
  sample=(PIND&4)>>2;  // lukee pin2;

  // servo koodi
  if (servopulssit==0) PORTB|=0x06;  //   digitalWrite(perasinPin,1), digitalWrite(moottoriPin,1);
  if (servopulssit==perasin) PORTB&=0x0FD; //    digitalWrite(perasinPin,0);
  if (servopulssit==moottori) PORTB&=0x0FB; //    digitalWrite(moottoriPin,0);
  servopulssit++;
  if (servopulssit>320) servopulssit=0; //20ms = 20*64 pulssia // aiemmin 1280
  valopulssit++;
  if (valopulssit==2000) { // Aiemmin 8000
    valopulssit=0;
    if (valot==0) {
      PORTD&=0xE7;
    } else if (valoVaihe==1) {
      PORTD|=0x08;
      PORTD&=0xEF;
    } else {
      PORTD&=0xF7;
      PORTD|=0x10;
    }
    valoVaihe=1-valoVaihe;
  }

  radioPreScaler++;            // sample otetaan vain 2 pulssin välein
  if (radioPreScaler==2) {  // Aiemmin 8
    radioPreScaler=0;
    avrSample+=sample;
    if (sample!=edellinenSample) {
      if (ramp<80) ramp+=11;
        else ramp+=29;
      edellinenSample=sample;
    } else ramp=ramp+20;
  
    if (ramp>=160) {            // käytännössä ohjelma odottaa kunnes tulee 0xC08B, sitten lukee seuraavat 24bit
      bitti8>>=1;
      bitti=0;
      if (avrSample>4) {
        bitti=1;
        bitti8|=0x80;
      }
      ramp-=160;
      avrSample=0;
      
      if (tulossa) {
        data>>=1;
        if (bitti==1) data|=0x800000;
        bittilkm++;
        if (bittilkm==24) {
          tulossa=false;
          tullutData=data;
        }
      } else if (bitti8==0xCC) {
        tulossa=true;
        bittilkm=0;
        data=0;
      }
    }
  }
}
