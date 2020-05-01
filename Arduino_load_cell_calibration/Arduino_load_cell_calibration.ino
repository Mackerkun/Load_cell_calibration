//**************************************************************************************//
//***********************      Arduino sketch for MDAS project      ********************//
//************************         Fiore, Stasolla, Tinti          *********************//
//**************************************************************************************//
//**************************************************************************************//

#include <EEPROM.h> 

// Pin for HX711
const int DOUT_PIN = 6; // DATA OUT Pin
const int PD_SCK = 7; // CLOCK Pin

uint8_t GAIN = 0; // Variable for GAIN read
long offset;  // Variable for offset read for first equlibration
long value = 0; // Variable for result read
bool flag = 1;  // Flag for writing the calibration values ​​in EEPROM
bool flag_for_setting_all_offset = 1; // Flag for initial balancing
long g_128 = 0; // Offset with GAIN 128
long g_64 = 0;  // Offset with GAIN 64
long g_32 = 0;  // Offset with GAIN 32
long read_32 = 0; // read with with GAIN 32
long read_64 = 0; // read with with GAIN 64
long read_128 = 0;  // read with with GAIN 128

void setup() {
  Serial.begin(115200); // Serial port @115200
  pinMode(PD_SCK, OUTPUT);  // Setting pin for HX711
  pinMode(DOUT_PIN, INPUT); // Setting pin for HX711
}

void loop() {
  if (Serial.available()) { // Check data on serial port
    digitalWrite(PD_SCK, LOW);  // Set LOW Clock PIN of HX711
    int g = Serial.parseInt();  // Parsing for GAIN value
    int op = Serial.parseInt(); // Parsing for function value
    if (Serial.read() == '\n') {  // Verify termination character
      switch (g) {
        case 0:
          GAIN = 2; // GAIN 32
          break;
        case 1:
          GAIN = 3; // GAIN 64
          break;
        case 2: // GAIN 128
          GAIN = 1;
          break;
      }
      // Switch for function
      switch (op) {
        case 0:
          reading();  // Function for reading data
          Serial.println(value);
          break;
        case 1:
          compare_gain(); // Function for GAIN comparison
          Serial.print(String(read_32) + ("\t") + String(read_64) + ("\t") + String(read_128) + ("\n"));
          break;
        case 2:
          flag_for_setting_all_offset = 1;  // Flag for activate the calibration
          calibration();  // Function calibration
          flag_for_setting_all_offset = 0;
          if (GAIN == 1) {
            g_128 = offset;
          } else if (GAIN == 2) {
            g_32 = offset;
          } else {
            g_64 = offset;
          };
          Serial.print(String(offset) + ("\t") + String(value) + ("\n"));
          break;
        case 3:
          get_parameters(); // Function for return the calibration values
          break;
        case 4:
          flag = 1;
          Serial.println("Setting parameters");
          set_parameters(); // Function for saving value in EEPROM
          break;
        case 5:
          set_all_offset(); // Function for first equilibration
          Serial.print(String(g_32) + ("\t") + String(g_64) + ("\t") + String(g_128) + ("\n"));
          break;
      }
    }
  }
}

// Function for return the calibration values
void get_parameters() {
  int address = 0;  // Function for return the calibration values
  double slope1;    // Slope value for GAIN 32
  double intercept1;    //  Intercept for GAIN 32
  double slope2;    //  Slope value for GAIN 64
  double intercept2;    //  Intercept for GAIN 64
  double slope3;    //  Slope value for GAIN 128
  double intercept3;    //  Intercept for GAIN 128
  EEPROM.get(address, slope1);  // Putting in EEPROM starting from 0
  address += sizeof(slope1);    // Setting the new address
  EEPROM.get(address, intercept1);
  address += sizeof(intercept1);
  EEPROM.get(address, slope2);
  address += sizeof(slope2);
  EEPROM.get(address, intercept2);
  address += sizeof(intercept2);
  EEPROM.get(address, slope3);
  address += sizeof(slope3);
  EEPROM.get(address, intercept3);
  address += sizeof(intercept3);
  Serial.print(String(slope1) + ("\t") + String(intercept1) + ("\t") + String(slope2) + ("\t") + String(intercept2) + ("\t") + String(slope3) + ("\t") + String(intercept3) + ("\n"));
  if (address == 512) { // If address == 512 start over 
    address = 0;
  }
}

void set_parameters() {
  int address;
  while (flag == 1) {
    // Check data presence on serial port
    if (Serial.available()) {
      int g = Serial.parseInt();  // Reading gain value on serial port
      float slope = Serial.parseFloat(); // Reading slope value on serial port
      float intercept = Serial.parseFloat(); // Reading intercept value on serial port
      if (Serial.read() == '\n') {
        switch (g) {
          case 0: // gain 32
            address = 0;
            EEPROM.put(address, slope);
            address += sizeof(slope);
            EEPROM.put(address, intercept);
            flag = 0;
            Serial.println("Success");
            break;
          case 1: // gain 64
            address = 8;
            EEPROM.put(address, slope);
            address += sizeof(slope);
            EEPROM.put(address, intercept);
            flag = 0;
            Serial.println("Success");
            break;
          case 2: // gain 128
            address = 16;
            EEPROM.put(address, slope);
            address += sizeof(slope);
            EEPROM.put(address, intercept);
            flag = 0;
            Serial.println("Success");
            break;
        }
      }
    }
  }
}

// Function for reading data from load cell
long reading() {
  long read = 0; // Variable for read value
  uint8_t data[3] = { 0 };  // Structure for read data saving
  uint8_t filler = 0x00;
  while (digitalRead(DOUT_PIN)) { // If pin is high, wait for available value
    delay(100);
  }

  // 24 clock pulses with sum for GAIN definition
  data[2] = shiftIn(DOUT_PIN, PD_SCK, MSBFIRST);
  data[1] = shiftIn(DOUT_PIN, PD_SCK, MSBFIRST);
  data[0] = shiftIn(DOUT_PIN, PD_SCK, MSBFIRST);

  // Select channel and gain factor
  for (unsigned int i = 0; i < GAIN; i++) {
    digitalWrite(PD_SCK, HIGH);
    digitalWrite(PD_SCK, LOW);
  }

  // Padding 32 bit
  if (data[2] & 0x80) {
    filler = 0xFF;
  } else {
    filler = 0x00;
  }

  // 32 bit number, obtained by bit shift
  read = ( static_cast<unsigned long>(filler) << 24
            | static_cast<unsigned long>(data[2]) << 16
            | static_cast<unsigned long>(data[1]) << 8
            | static_cast<unsigned long>(data[0]) );

  if (flag_for_setting_all_offset == 1) {
    value = read - offset;
  } else {
    // Calibration with offset subtraction
    if (GAIN ==  1) {
      value = read - g_128;
    } else if (GAIN == 2) {
      value = read - g_32;
    } else {
      value = read - g_64;
    }
  }
  return (value);
}

void calibration() {
  offset = 0; // Initialize the variable containing the offset
  offset = reading(); // Reads to populate the offset variable
  reading();  // Take the second reading to check the condition
  while (abs(value) > 200) { // To get a better calibration,
    // cycle until the differences is less than 200
    calibration();
  }
}

// Function to set the GAIN offset
void set_all_offset() {
  // GAIN a 128
  GAIN = 1;
  calibration();
  g_128 = offset;
  delay(500);
  // GAIN a 64
  GAIN = 3;
  calibration();
  g_64 = offset;
  delay(500);
  // GAIN a 32
  GAIN = 2;
  calibration();
  g_32 = offset;
  // Disables the control on the flag concerning the setting of the offset
  flag_for_setting_all_offset = 0;
}

// Function for comparing GAIN
void compare_gain() {
  long read = 0;
  int i;
  // GAIN at 128
  GAIN = 1;
  for (i = 0; i < 10; i++) { //average over ten samples
    read = read + reading();
  }
  read_128 = read / 10;
  delay(500);

  read = 0;
  // GAIN at 64
  GAIN = 3;
  for (i = 0; i < 10; i++) { //average over ten samples
    read = read + reading();
  }
  read_64 = read / 10;
  delay(500);

  // GAIN at 32
  GAIN = 2;
  read = 0;
  for (i = 0; i < 10; i++) {//average over ten samples
    read = read + reading();
  }
  read_32 = read / 10;
}
