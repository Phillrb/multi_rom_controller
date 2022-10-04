
// ATTiny as a Multi-ROM controller
// Phillip Riscombe-Burton 2022
//
// - Changes target EPROM bank on button press or hold of ACTION_PIN
// - Controls up to 16 banks with A3 enabled: 0b0000 to 0b1111
// - Controls up to 8 banks with A3 disabled: 0b000 to 0b111
// - Reset target with RESET_TARGET_PIN (active LOW)
// - Uses ATTiny85/45/25/13 EEPROM to persist current bank
//
// To program ATTiny with Arduino Nano:
//
// INITIAL SETUP OF NANO
// File >> Examples >> ArduinoISP 
// Programmer >> Arduino as ISP (Old Bootloader)
// Upload sketch
//
// PROGRAM ATTiny WITH NANO:
// Sketch >> Uplaod Using Programmer
//
// Alternatively program ATTiny with TL866 or similar;
// (the only option when A3 is enabled)
// Sketch >> Export compiled Binary
//
//        ATTINY85/45/25/13
//          _  _
//  (A3*) -| \/ |- VCC
// ACTION -|    |- A2
//  RESET -|    |- A1
//    GND -|____|- A0
//
// *Controlling A3 will prevent reprogramming of ATTiny.
// TL866 programmer or similar must be used to reset the fuse bits.

#include <EEPROM.h>

// Programming options
#define ENABLE_A3 false
#define HOLD_FOR_BANKSWITCH true

const int A0_PIN = 0; // 0 = physical pin 5
const int A1_PIN = 1; // 1 = physical pin 6
const int A2_PIN = 2; // 2 = physical pin 7
#if ENABLE_A3
const int A3_PIN = 5; // 5 = physical pin 1
#endif

const int ACTION_PIN = 3; // 3 = physical pin 2
const int RESET_TARGET_PIN = 4; // 4 = physical pin 3

const int HOLD_RESET_MS = 100; // 0.1 sec

// Debounce
int actionState = HIGH;
int lastActionState = HIGH;
unsigned long lastDebounceTime = 0;  // last time action was carried out


// debounce time (extended for hold logic if required)
#if HOLD_FOR_BANKSWITCH

const unsigned long debounceDelay = 4000; // hold time
const int BANKSWITCH_STATE = LOW; // hold down for bankswitch
bool isActionButtonActive = false;
bool isBankSwitchActive = false;

#else

const unsigned long debounceDelay = 50;   // debounce time
const int BANKSWITCH_STATE = HIGH; // release for bankswitch

#endif


// ATTiny85 has 512 bytes internal EEPROM (0x000 to 0x1FF)
byte ADDR_EEPROM_LOC = 0x000; // Bank number stored at this location
byte ADDR = 0; // local var used throughout

// Get specific bit from byte (starts at bit 0)
#define GET_BIT(var,pos) ((var) & (1<<(pos)))

void setup() {
  
    // Control address pins
    pinMode(A0_PIN, OUTPUT);
    pinMode(A1_PIN, OUTPUT);
    pinMode(A2_PIN, OUTPUT);
#if ENABLE_A3
    pinMode(A3_PIN, OUTPUT);
#endif

    // Allow reading from action pin
    pinMode(ACTION_PIN, INPUT_PULLUP);

    // Load the current bank address
    loadCurrentBank();
}

void loop() {

  // Action pin state
  int reading = digitalRead(ACTION_PIN);
  
#if HOLD_FOR_BANKSWITCH

  // is Action button held long enough to take action?
  if (reading == BANKSWITCH_STATE) {
    
    if(isActionButtonActive == false) {
      isActionButtonActive = true;
      lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay && (isBankSwitchActive == false)) {
      isBankSwitchActive = true;
      // Load the next bank address
      loadNextBank();
    }
  } else if(isActionButtonActive) {
    
      if(isBankSwitchActive == true) {
        isBankSwitchActive = false;
      } 
      isActionButtonActive = false;
  }
  
#else
  
  // Was Action button pressed and released?
  if (reading != lastActionState) {
    lastDebounceTime = millis();
  }

  // Avoid bounce period
  if ((millis() - lastDebounceTime) > debounceDelay) {

    // Was there a change?
    if (reading != actionState) {

      // record new state
      actionState = reading;

      // only take action if the new state is BANKSWITCH_STATE
      if (actionState == BANKSWITCH_STATE) {

        // Load the next bank address
        loadNextBank();
      }
    }
  }
  
  // Record as last state
  lastActionState = reading;
  
#endif

}

// Read address from EEPROM
void loadCurrentBank() {
  ADDR = getAddrFromEeprom();
  loadBank(ADDR);
}

// Go to next address in EEPROM
void loadNextBank() {
  ADDR = incAddr();
  loadBank(ADDR);
}

// Write address out to pins and reset target
void loadBank(byte addr) {
  updateBankSwitch(addr);
  resetTarget();        
}

// Read current bank switch address from eeprom 
byte getAddrFromEeprom() {
  return EEPROM.read(ADDR_EEPROM_LOC);
}

// Write bank switch address to eeprom 
void updateAddrInEeprom(byte addr) {
  EEPROM.write(ADDR_EEPROM_LOC, addr);
}

// Go to next bank switch address and save in eeprom
// Address shadowing is OK
byte incAddr() {
  ADDR = getAddrFromEeprom();
  ADDR++;
  updateAddrInEeprom(ADDR);
  return ADDR;
}

// Output current address to pins
void updateBankSwitch(byte addr) {
  digitalWrite(A0_PIN, GET_BIT(addr, 0));
  digitalWrite(A1_PIN, GET_BIT(addr, 1));
  digitalWrite(A2_PIN, GET_BIT(addr, 2));
#if ENABLE_A3
  digitalWrite(A3_PIN, GET_BIT(addr, 3));
#endif
}

// Pulse the reset line low for a set time period
void resetTarget() {
  pinMode(RESET_TARGET_PIN, OUTPUT);
  digitalWrite(RESET_TARGET_PIN, LOW);
  delay(HOLD_RESET_MS);
  pinMode(RESET_TARGET_PIN, INPUT);
}
