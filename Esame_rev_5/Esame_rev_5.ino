#include <EEPROM.h>

// Pin dell'amplificatore HX711
const int DOUT_PIN = 6;  // Data out pin
const int PD_SCK = 7;  // Clock pin

uint8_t GAIN = 0; // Per impostare il GAIN
long offset;  // Offset da sottrarre per l'ecquilibratura iniziale
long val = 0; // Variabile per il valore letto - offset

void setup() {
  Serial.begin(115200); // Porta seriale
  pinMode(PD_SCK, OUTPUT);  // Comunicazione con HX711 - Pin per il segnale di Clock
  pinMode(DOUT_PIN, INPUT); // Comunicazione con HX711 - Pin per il segnale Data
  // Inizializzazione dei registri EEPROM
  EEPROM.write(1, 0); // Inizializzo il registro del segno
  EEPROM.write(2, 1); // Inizializzo il registro del valore che conterrà lo slope a 1
  EEPROM.write(3, 0); // Inizializzo il registro per il segno dell'intercetto
  EEPROM.write(4, 0); // Inizializzo il registro per il valore dell'intercetto
  EEPROM.write(5, 0);
  EEPROM.write(6, 1);
  EEPROM.write(7, 0);
  EEPROM.write(8, 0);
  EEPROM.write(9, 0);
  EEPROM.write(10, 1);
  EEPROM.write(11, 0);
  EEPROM.write(12, 0);

}

void loop() {
  if (Serial.available()) { // Controllo la presenza di dati sulla porta seriale
    digitalWrite(PD_SCK, LOW); // Setto il pin del clock basso per fare le configurazioni (riferimento da datasheet)
    int g = Serial.parseInt();  // Lettura del valore del gain
    int op = Serial.parseInt(); // Lettura della funzione da fare
    if (Serial.read() == '\n') { // Verifico che il terzo carattere sia il terminatore di riga
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
      // Switch per decidere la funzione
      switch (op) {
        case 0:
          reading();  // Funzione per leggere i dati
          Serial.println(val);
          break;
        case 1:
          compare_gain(); // Funzione per effettuare tre misure con 3 GAIN diversi
          break;
        case 2:
          calibration();  // Funzione per la calibrazione
          break;
        case 3:
          get_parameters(); // Funzione per la restituzione dei valori in EEPROM
          break;
        case 4:
          set_parameters(); // Funzione per l salvataggio dei dati in EEPROM
          break;
      }
    }
  }
}
// Funzione per la lettura dei dati della cella
long reading() {
  long value = 0; // Variabile per il valore letto
  uint8_t data[3] = { 0 };  // Struttura per salvare i dati letti
  uint8_t filler = 0x00;
  while (digitalRead(DOUT_PIN)) { // Finchè il pin è alto aspetto che il valore sia disponibile
    delay(100);
  }

  // 24 colpi di clock a cui vanno sommato quelli per definire il GAIN
  data[2] = shiftIn(DOUT_PIN, PD_SCK, MSBFIRST);
  data[1] = shiftIn(DOUT_PIN, PD_SCK, MSBFIRST);
  data[0] = shiftIn(DOUT_PIN, PD_SCK, MSBFIRST);

  // seleziono il canale e il gain factor
  for (unsigned int i = 0; i < GAIN; i++) {
    digitalWrite(PD_SCK, HIGH);
    digitalWrite(PD_SCK, LOW);
  }

  // Replica per padding a 32 bit
  if (data[2] & 0x80) {
    filler = 0xFF;
  } else {
    filler = 0x00;
  }

  // intero a 32 bit, ottenuto dallo shift dei bit
  value = ( static_cast<unsigned long>(filler) << 24
            | static_cast<unsigned long>(data[2]) << 16
            | static_cast<unsigned long>(data[1]) << 8
            | static_cast<unsigned long>(data[0]) );

  // Effettuo l'equilibratura sottraendo l'offset
  val = value - offset;
  return (val);
}
void calibration() {
  offset = 0; // Inizializzo la variabile che contiene l'offset
  offset = reading(); // Leggo per popolare la variabile offset
  reading();  //effettuo la seconda lettura per verificare la condizione
  while (abs(val) > 200) { // Per ottenenre una migliore equilibratura,
    // ciclo finchè la differenza non sia minore di 200
    offset = reading();
    reading();
  }
  Serial.println(offset);
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
void get_parameters() {
  if (EEPROM.read(1) == 1) {
    Serial.println((EEPROM.read(2) * -1));
  } else {
    Serial.println(EEPROM.read(2));
  }
  if (EEPROM.read(3) == 1) {
    Serial.println((EEPROM.read(4) * -1));
  } else {
    Serial.println(EEPROM.read(4));
  }
  if (EEPROM.read(5) == 1) {
    Serial.println((EEPROM.read(6) * -1));
  } else {
    Serial.println(EEPROM.read(6));
  }
  if (EEPROM.read(7) == 1) {
    Serial.println((EEPROM.read(8) * -1));
  } else {
    Serial.println(EEPROM.read(8));
  }
  if (EEPROM.read(9) == 1) {
    Serial.println((EEPROM.read(10) * -1));
  } else {
    Serial.println(EEPROM.read(10));
  }
  if (EEPROM.read(11) == 1) {
    Serial.println((EEPROM.read(12) * -1));
  } else {
    Serial.println(EEPROM.read(12));
  }
}
void set_parameters() {
  if (Serial.available()) { // Controllo la presenza di dati sulla porta seriale
    int g = Serial.parseInt();  // Lettura del valore del gain
    float slope = Serial.parseFloat(); // Lettura della pendenza della retta
    float intercept = Serial.parseFloat(); // Lettura del valore di
    if (Serial.read() == '\n') {
      switch (g) {
        case 0: // 32 sul canale B
          if (slope < 0) {
            EEPROM.write(1, 1); // Scrivo nel primo registro un flag per il segno negativo
            EEPROM.write(2, abs(slope));
          } else {
            EEPROM.write(1, 0);
            EEPROM.write(2, slope); // Scrivo
          }
          if (intercept < 0) {
            EEPROM.write(3, 1); // Scrivo nel primo registro un flag per il segno negativo
            EEPROM.write(4, abs(intercept));
          } else {
            EEPROM.write(3, 0);
            EEPROM.write(4, intercept); // Scrivo
          }
          Serial.println("Success");
          break;
        case 1: // 64 canale A
          if (slope < 0) {
            EEPROM.write(5, 1); // Scrivo nel primo registro un flag per il segno negativo
            EEPROM.write(6, abs(slope));
          } else {
            EEPROM.write(5, 0);
            EEPROM.write(6, slope); // Scrivo
          }
          if (intercept < 0) {
            EEPROM.write(7, 1); // Scrivo nel primo registro un flag per il segno negativo
            EEPROM.write(8, abs(intercept));
          } else {
            EEPROM.write(7, 0);
            EEPROM.write(8, intercept); // Scrivo
          }
          Serial.println("Success");
          break;
        case 2: // 128 canale A
          if (slope < 0) {
            EEPROM.write(9, 1); // Scrivo nel primo registro un flag per il segno negativo
            EEPROM.write(10, abs(slope));
          } else {
            EEPROM.write(9, 0);
            EEPROM.write(10, slope); // Scrivo
          }
          if (intercept < 0) {
            EEPROM.write(11, 1); // Scrivo nel primo registro un flag per il segno negativo
            EEPROM.write(12, abs(intercept)); // Salvo in valore assoluto
          } else {
            EEPROM.write(11, 0);
            EEPROM.write(12, intercept); // Altrimenti scrivo il valore
          }
          Serial.println("Success");
          break;
      }
    }
  }
}
