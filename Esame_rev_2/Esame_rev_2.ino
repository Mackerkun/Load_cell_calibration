#include <Arduino.h>

// Pin di HX711
const int DOUT_PIN = 16; // Data out pin
const int PD_SCK = 17; // Clock pin

uint8_t GAIN = 0;  // Per il GAIN
long offset;  // Offset da sottrarre per la taratura
long val = 0; // Var per il valore letto - offset

void setup() {
  Serial.begin(115200); // Posta seriale
  pinMode(PD_SCK, OUTPUT);  // Comunicazione con HX711 - Pin per il segnale di Clock
  pinMode(DOUT_PIN, INPUT); // Comunicazione con HX711 - Pin per il segnale Data
}

void loop() {
  if (Serial.available()) { // Controllo di dati sulla seriale
    // Arrivano i dati per effettuare la misurazione
    analogWrite(PD_SCK, LOW);  // Setto il pin del clock basso per fare le configurazioni (riferimento da datasheet)
    int g = Serial.parseInt(); // Lettura del valore del gain
    int op = Serial.parseInt(); // Lettura della funzione da fare
    Serial.flush();
    switch (g) {
      case 0:
        GAIN = 2; // GAIN a 32 sul canale B
        break;
      case 1:
        GAIN = 3; // GAIN a 64 canale A
        break;
      case 2: // GAIN a 128 canale A
        GAIN = 1;
        break;
    }
    // chiama alla funzione
    switch (op) {
      case 0:
        // Funzione per leggere i dati
        reading();
        Serial.println(val);
        break;
      case 1:
        // Funzione per effettuare tre misure con 3 GAIN diversi
        compare_gain();
        break;
      case 2:
        // Funzione per la calibrazione
        calibration();
        break;
    }
  }
}
// Funzione per la lettura dei dati della cella
long reading() {
  long value = 0; // Var per il valore letto
  uint8_t data[3] = { 0 };  // Struttura per salvare i dati letti
  uint8_t filler = 0x00;  // Per MostSignificantBit?
  while (analogRead(DOUT_PIN)) { // Finchè il pin è alto aspetto che il valore sia disponibile
    delay(100);
  }
  // 24 colpi di clock a cui va sommato il GAIN
  data[2] = shiftIn(DOUT_PIN, PD_SCK, MSBFIRST);
  data[1] = shiftIn(DOUT_PIN, PD_SCK, MSBFIRST);
  data[0] = shiftIn(DOUT_PIN, PD_SCK, MSBFIRST);

  // set the channel and the gain factor for the next reading using the clock pin
  for (unsigned int i = 0; i < GAIN; i++) {
    analogWrite(PD_SCK, HIGH);
    analogWrite(PD_SCK, LOW);
  }

  // Replicate the most significant bit to pad out a 32-bit signed integer
  if (data[2] & 0x80) {
    filler = 0xFF;
  } else {
    filler = 0x00;
  }

  // Construct a 32-bit signed integer
  value = ( static_cast<unsigned long>(filler) << 24
            | static_cast<unsigned long>(data[2]) << 16
            | static_cast<unsigned long>(data[1]) << 8
            | static_cast<unsigned long>(data[0]) );

  // Sottraggo l'offset, nel caso non sia tarato sarà 0
  val = value - offset;
  return (val);
}
void calibration() {
  offset = 0;
  offset = reading(); // leggo il valore di offset per poi sottrarlo nel reading ed ottenere il valore corretto dalla funzione reading stessa
  reading();
  while ((val - offset) < 0 && (val - offset) < 160) {
    offset = reading(); // leggo il valore di offset per poi sottrarlo nel reading ed ottenere il valore corretto dalla funzione reading stessa
    reading();
  }
  Serial.println(offset);
  reading();
  Serial.println(val);
}
// Funzione per la comparazione dei GAIN
void compare_gain() {
  // GAIN a 128
  GAIN = 1;
  long g_128 = reading();
  Serial.println(val);
  delay(100);
  GAIN = 2;
  long g_32 = reading();
  Serial.println(val);
  delay(100);
  GAIN = 3;
  long g_64 = reading();
  Serial.println(val);
}
