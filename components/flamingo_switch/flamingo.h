#pragma once

#include "esphome.h"
#include "esphome/core/component.h"

using namespace esphome;

#define FLAMINGO_NUM_REPEATS 4
#define FLAMINGO_PULSE_DURATION 268

class FlamingoSwitch : public Component, public Switch {
 public:

  uint8_t mUnitId;
  uint16_t mAddress;
  uint8_t mTxPin;
  bool mIsGroup;

  FlamingoSwitch(uint8_t txPin, uint16_t address, uint8_t unitId)
  {
    mTxPin = txPin;
    mAddress = address;
    mUnitId = unitId;
    mIsGroup = (unitId == 0);
  }

  void setup() override {
    pinMode(mTxPin, OUTPUT);
  }

  void write_state(bool state) override
  {
    noInterrupts();
    if (mIsGroup)
      sendGroup(mAddress, state);
    else
      sendSwitch(mUnitId, mAddress, state);
  
    interrupts();
    publish_state(state);
  }

  void sendGroup(uint16_t address, bool state) {
	for (int i = FLAMINGO_NUM_REPEATS; i >= 0; i--) {
		sendStartPulse();
		sendAddress(address);

		// Group bit
		sendBit(true);

		// Switch on | off
		sendBit(state);
		
		// No unit. Is this actually ignored?..
		sendUnit(0);

		sendStopPulse();
	}
  }

  void sendSwitch(uint8_t unit, uint16_t address, bool state) {
    for (int i = FLAMINGO_NUM_REPEATS; i >= 0; i--) {
      sendStartPulse();
      sendAddress(address);

      // No group bit
      sendBit(false);

      // Switch on | off
      sendBit(state);

      sendUnit(unit);

      sendStopPulse();
    }
  }

  void sendStartPulse(){
	digitalWrite(mTxPin, HIGH);
	delayMicroseconds(FLAMINGO_PULSE_DURATION);
	digitalWrite(mTxPin, LOW);
	delayMicroseconds(FLAMINGO_PULSE_DURATION * 10 + (FLAMINGO_PULSE_DURATION >> 1)); // Actually 10.5T instead of 10.44T. Close enough.
  }

  void sendAddress(uint16_t addr) {
    for (short i=25; i>=0; i--) {
      sendBit((addr >> i) & 1);
    }
  }

  void sendUnit(uint8_t unit) {
    for (short i=3; i>=0; i--) {
      sendBit(unit & 1<<i);
    }
  }

  void sendStopPulse() {
    digitalWrite(mTxPin, HIGH);
    delayMicroseconds(FLAMINGO_PULSE_DURATION);
    digitalWrite(mTxPin, LOW);
    delayMicroseconds(FLAMINGO_PULSE_DURATION * 40);
  }

  void sendBit(bool isBitOne) {
    if (isBitOne) {
      // Send '1'
      digitalWrite(mTxPin, HIGH);
      delayMicroseconds(FLAMINGO_PULSE_DURATION);
      digitalWrite(mTxPin, LOW);
      delayMicroseconds(FLAMINGO_PULSE_DURATION * 5);
      digitalWrite(mTxPin, HIGH);
      delayMicroseconds(FLAMINGO_PULSE_DURATION);
      digitalWrite(mTxPin, LOW);
      delayMicroseconds(FLAMINGO_PULSE_DURATION);
    } else {
      // Send '0'
      digitalWrite(mTxPin, HIGH);
      delayMicroseconds(FLAMINGO_PULSE_DURATION);
      digitalWrite(mTxPin, LOW);
      delayMicroseconds(FLAMINGO_PULSE_DURATION);
      digitalWrite(mTxPin, HIGH);
      delayMicroseconds(FLAMINGO_PULSE_DURATION);
      digitalWrite(mTxPin, LOW);
      delayMicroseconds(FLAMINGO_PULSE_DURATION * 5);
    }
  }

};