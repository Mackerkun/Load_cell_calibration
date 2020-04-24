#include <Arduino.h>


uint8_t GAIN = 0;  // Per il GAIN
long offset;  // Offset da sottrarre per la taratura
long val = 0; // Var per il valore letto - offset

void setup() {
  Serial.begin(115200); // Posta seriale
  pinMode(A1, INPUT); // Comunicazione con HX711 - Pin per il segnale Data
}

void loop() {
  if (Serial.available()) { // Controllo di dati sulla seriale
    int g = Serial.parseInt();  // Lettura del valore del gain
    int op = Serial.parseInt(); // Lettura della funzione da fare
    if(Serial.read() == '\n'){
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
          break;
        case 1:
          compare_gain(); // Funzione per effettuare tre misure con 3 GAIN diversi
          break;
        case 2:
          calibration();  // Funzione per la calibrazione
          break;
      }
    }
  }
}
// Funzione per la lettura dei dati della cella
long reading() {
  Serial.println(analogRead(A1));
}
void calibration() {
  Serial.println(analogRead(A1));
  Serial.println(analogRead(A1));
}
// Funzione per la comparazione dei GAIN
void compare_gain() {
  Serial.println(analogRead(A1));
  Serial.println(analogRead(A1));

}
