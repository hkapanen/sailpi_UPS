// Sketch for controlling MightyBoost (http://lowpowerlab.com/mightyboost/) 
// for basic UPS-like functionality for Raspberry Pi computer
// Copyright Harri Kapanen

#include <LowPower.h>  // https://github.com/LowPowerLab/LowPower

//#define SERIAL_DEBUG   // Uncomment to enable serial debug messages

// GPIO mapping
#define PINI_BATT_MON        A7   // Battery level input
#define PINI_RUNNING         A0   // Running signal from RasPi
#define PINO_SHUTDOWN         6   // Shutdown signal to RasPi
#define PINO_POWER_RASPI      4   // 5V on/off switch to RasPi
#define PINO_BOOSTER          7   // LiPo-5V Booster chip control

// FSM states
#define S_POWEROFF   1
#define S_BOOTUP     2
#define S_POWERON    3
#define S_LOSTPOWER  4
#define S_SHUTDOWN   5

// Voltage thresholds
// Full LiPo is around 4.15v and empty 3.3v
#define GOTPOWER_THRESHOLD   4.4
#define LOSTPOWER_THRESHOLD  4.2
#define RUNNING_THRESHOLD    800 // mW

// Sleep periods
#define POWERCHECK_PERIOD   SLEEP_2S
#define SHUTDOWN_DELAY      SLEEP_4S

int state = S_POWEROFF;

// BATT_MON has 10k+4.7k voltage divider => 1.47 multiplier
// 3.3V is 1024 => 0.00322266 coefficient
float volts_coeff = 0.00322266 * 1.47;
float volts = 5;

boolean RASPI_RUNNING() {
  return analogRead(PINI_RUNNING) > RUNNING_THRESHOLD;
}

#ifdef SERIAL_DEBUG
  #define DEBUG(input)   {delay (1); Serial.print(input); delay(1);}
  #define DEBUGln(input) {delay (1); Serial.println(input); delay(1);}
#else
  #define DEBUG(input);
  #define DEBUGln(input);
#endif


void setup() {
  pinMode(PINO_BOOSTER, OUTPUT);
  pinMode(PINO_SHUTDOWN, OUTPUT);
  pinMode(PINI_RUNNING, INPUT);
  pinMode(PINO_POWER_RASPI, OUTPUT);
  pinMode(PINI_BATT_MON, INPUT);
  digitalWrite(PINO_BOOSTER, LOW);  // all down on initial power on
  digitalWrite(PINO_SHUTDOWN, LOW);
  digitalWrite(PINO_POWER_RASPI, LOW);

  #ifdef SERIAL_DEBUG
    Serial.begin(115200);
    DEBUGln("\nSerial debug enabled!");
  #endif
}

void loop() {
  volts = analogRead(PINI_BATT_MON) * volts_coeff;
  
  switch(state)
  {
    case S_POWEROFF:
      DEBUG("Power off... ");
      
      if (volts > GOTPOWER_THRESHOLD) {
        digitalWrite(PINO_BOOSTER, HIGH);
        digitalWrite(PINO_POWER_RASPI, HIGH);
        state = S_BOOTUP;
      }
      break;

    case S_BOOTUP:
      DEBUG("Got power - booting up! ");

      if (RASPI_RUNNING()) {
        state = S_POWERON;
      }
      // todo recovery if power lost and no bootup
      break;

    case S_POWERON:
      DEBUG("RasPi running! ");

      if (volts < LOSTPOWER_THRESHOLD) {
        state = S_LOSTPOWER;
      }
      break;

    case S_LOSTPOWER:  // Let's doublecheck if we really lost the power...
      DEBUG("Power lost? ");
            
      LowPower.powerDown(POWERCHECK_PERIOD, ADC_OFF, BOD_ON);
      
      if (volts < LOSTPOWER_THRESHOLD) {
        DEBUG("yes ");
        digitalWrite(PINO_SHUTDOWN, HIGH);
        state = S_SHUTDOWN;
      } else {
        DEBUG("not really ");
        state = S_POWERON;
      }
      break;

    case S_SHUTDOWN:
      DEBUG("Lost power, shutting RasPi down! ");
            
      if (!RASPI_RUNNING()) {
        LowPower.powerDown(SHUTDOWN_DELAY, ADC_OFF, BOD_ON);
        digitalWrite(PINO_POWER_RASPI, LOW);  // 5V output off
        digitalWrite(PINO_BOOSTER, LOW);      // Booster off
        digitalWrite(PINO_SHUTDOWN, LOW);
        state = S_POWEROFF;
      }
      break;
    }

  DEBUG("Volts: ");
  DEBUG(volts);
  DEBUG("  RasPi: ");
  DEBUGln(RASPI_RUNNING());

  LowPower.powerDown(POWERCHECK_PERIOD, ADC_OFF, BOD_ON);
}

