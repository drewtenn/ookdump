#include <SPI.h>
#include <RH_RF69.h>

#define RF69_FREQ 433.42

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     7
  #define RFM69_RST     4
  #define LED           13
#endif

#if defined(ADAFRUIT_FEATHER_M0) // Feather M0 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     3
  #define RFM69_RST     4
  #define LED           13
  #define RFM69_DATA    10
  #define RFM69_CLK_INT 11
#endif

#if defined (__AVR_ATmega328P__)  // Feather 328P w/wing
  #define RFM69_INT     3  // 
  #define RFM69_CS      4  //
  #define RFM69_RST     2  // "A"
  #define LED           13
#endif

#if defined(ESP8266)    // ESP8266 feather w/wing
  #define RFM69_CS      2    // "E"
  #define RFM69_IRQ     15   // "B"
  #define RFM69_RST     16   // "D"
  #define LED           0
#endif

#if defined(ESP32)    // ESP32 feather w/wing
  #define RFM69_RST     13   // same as LED
  #define RFM69_CS      33   // "B"
  #define RFM69_INT     27   // "A"
  #define LED           13
#endif

/* Teensy 3.x w/wing
#define RFM69_RST     9   // "A"
#define RFM69_CS      10   // "B"
#define RFM69_IRQ     4    // "C"
#define RFM69_IRQN    digitalPinToInterrupt(RFM69_IRQ )
*/
 
/* WICED Feather w/wing 
#define RFM69_RST     PA4     // "A"
#define RFM69_CS      PB4     // "B"
#define RFM69_IRQ     PA15    // "C"
#define RFM69_IRQN    RFM69_IRQ
*/

#define OOK_THRESHOLD 45

RH_RF69 rf69(RFM69_CS, RFM69_INT);

void setup() 
{
  Serial.begin(115200);
  while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

  attachInterrupt(digitalPinToInterrupt(RFM69_CLK_INT), readData, RISING); // connected to DIO1
  pinMode(RFM69_DATA, INPUT); // connected to DIO2

  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println("Feather RFM69 RX Test!");
  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
 
  Serial.println("RFM69 radio init OK!");
  
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }
  
  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");

  rf69.setModemConfig(RH_RF69::OOK_Rb1_2Bw75);
  
  rf69.spiWrite(RH_RF69_REG_02_DATAMODUL, RH_RF69_DATAMODUL_DATAMODE_CONT_WITH_SYNC | RH_RF69_DATAMODUL_MODULATIONTYPE_OOK | RH_RF69_DATAMODUL_MODULATIONSHAPING_OOK_NONE);
  rf69.spiWrite(RH_RF69_REG_1B_OOKPEAK, rf69.spiRead(RH_RF69_REG_1B_OOKPEAK) & ~0xc0);
  rf69.spiWrite(RH_RF69_REG_1D_OOKFIX, OOK_THRESHOLD);

  rf69.setModeRx();
}

void loop() {  
}

unsigned long time = micros();
int state = LOW;

void readData() {
  int newState = digitalRead(10);

  if (newState != state) {
    unsigned long now = micros();    
    unsigned long duration = (now > time) ? now - time : (ULONG_MAX - time) + now; // in case it wraps in the middle of a signal
         
    Serial.printf("%s = %u\n", state == HIGH ? "HIGH" : "LOW", duration);
    
    state = newState;
    time = now;
  }
}
