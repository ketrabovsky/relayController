#ifndef __RELAY_H
#define __RELAY_H

#include <stdint.h>

int relayModuleInit(uint8_t r1, uint8_t r2);
void switch1Relay(void);
void switch2Relay(void);
void setRelay1Pin(uint8_t pin);
void setRelay2Pin(uint8_t pin);
void setRelay1State(uint8_t state);
void setRelay2State(uint8_t state);
int getRelay1State(void);
int getRelay1State(void);

#endif
