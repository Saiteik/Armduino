#include "MSP.h"
#include "MSP_OSD.h"

#define DEBUG                 0       // 0 = false 1 = true
#define LED_PIN               13
#define ARM_PIN               10
#define PWM_HIGH              2000
#define PWM_CENTER            1500
#define PWM_LOW               1000
#define VOLT_PIN              1
#define VCC_SCALE             1010    // + decreases vbat / - increases vbat

const float ValueR1 = 22000.0;        //  22K   Resistor
const float ValueR2 = 5100.0;         //  5.1K  Resistor
const float arduinoVCC = 3.3;
const float voltageScale = arduinoVCC / VCC_SCALE;
const int disarm_flag = 0x00000002;
const int arm_flag = 0x00000003;
const int next_interval_MSP = 100;

uint8_t
    sampleCount = 100,
    batteryCellCount;

uint32_t
    vbat,
    rawVbat,
    inputStart,
    pulseWidth,
    armInputRx,
    sampleVoltageCount,
    flightModeFlags = 0x00000002;

float averageVoltage = 0.0;

volatile unsigned long previousMillis_MSP = 0;
volatile unsigned long activityDetectedMillis_MSP = 0;
volatile unsigned long general_counter = next_interval_MSP;

MSP msp;
msp_status_DJI_t status_DJI = { 0 };
msp_battery_state_t battery_state = { 0 };

void setup() {
  if (DEBUG) {
    Serial.begin(115200);
  }
  Serial1.begin(115200);

  msp.begin(Serial1);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(ARM_PIN, INPUT_PULLUP);

  status_DJI.cycleTime = 0x0080;
  status_DJI.i2cErrorCounter = 0;
  status_DJI.sensor = 0x23;
  status_DJI.configProfileIndex = 0;
  status_DJI.averageSystemLoadPercent = 7;
  status_DJI.accCalibrationAxisFlags = 0;
  status_DJI.DJI_ARMING_DISABLE_FLAGS_COUNT = 20;
  status_DJI.djiPackArmingDisabledFlags = (1 << 24);

  if (DEBUG) {
    Serial.println("Initialized DJI arm module.");
  }
}

void readVoltage() {
  rawVbat = 0;
  for (int i = 0; i < sampleCount; i++) rawVbat += analogRead(VOLT_PIN);
  rawVbat = rawVbat / sampleCount;
  vbat = (rawVbat * voltageScale) * (1 + (ValueR1 / ValueR2)) * 10;
}

uint8_t getCellCount(uint8_t voltage) {
  uint8_t batteryCellCount = 0;
  if (voltage < 43 && voltage > 0) batteryCellCount = 1;
  else if (voltage < 85) batteryCellCount = 2;
  else if (voltage < 127) batteryCellCount = 3;
  else if (voltage < 169) batteryCellCount = 4;
  else if (voltage < 211) batteryCellCount = 5;
  else if (voltage < 255) batteryCellCount = 6;

  return batteryCellCount;
}

void send_msp_to_airunit() {
  uint8_t cellCount = getCellCount(vbat);

  //MSP_STATUS
  status_DJI.flightModeFlags = flightModeFlags;
  status_DJI.armingFlags = 0x0303;
  msp.send(MSP_STATUS_EX, &status_DJI, sizeof(status_DJI));
  status_DJI.armingFlags = 0x0000;
  msp.send(MSP_STATUS, &status_DJI, sizeof(status_DJI));
    
  if (DEBUG) {
    Serial.println(vbat);
  }

  //MSP_BATTERY_STATE
  battery_state.batteryVoltage = vbat / 10;
  battery_state.batteryCellCount = cellCount;
  battery_state.legacyBatteryVoltage = vbat;
  msp.send(MSP_BATTERY_STATE, &battery_state, sizeof(battery_state));
}

void set_flight_mode_flags() {
  volatile int pwmValue = readChannel(ARM_PIN, PWM_LOW, PWM_HIGH, 0);
  if ((flightModeFlags == disarm_flag) && pwmValue >= PWM_CENTER) {
    flightModeFlags = arm_flag;  // Arm and record
    
    if (DEBUG) {
      Serial.println("ARMED");
    }
    
    digitalWrite(LED_PIN, LOW);
  } else if ((flightModeFlags == arm_flag) && pwmValue < PWM_CENTER && general_counter % 3000 == 0) {
    flightModeFlags = disarm_flag;  // Disarm and stop recording after 3 second delay
    
    if (DEBUG) {
      Serial.println("DISARMED");
    }
    
    digitalWrite(LED_PIN, HIGH);
  }
}

int readChannel(int channelInput, int minLimit, int maxLimit, int defaultValue) {
  int ch = pulseIn(channelInput, HIGH, 30000);
  if (ch < 100) return defaultValue;
  return map(ch, PWM_LOW, PWM_HIGH, minLimit, maxLimit);
}

void loop() {
  uint32_t currentMillis_MSP = millis();

  if ((uint32_t)(currentMillis_MSP - previousMillis_MSP) >= next_interval_MSP) {
    previousMillis_MSP = currentMillis_MSP;
    
    readVoltage();
    set_flight_mode_flags();
    send_msp_to_airunit();
    
    general_counter += next_interval_MSP;
  }
}
