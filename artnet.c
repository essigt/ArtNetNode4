#include "artnet.h"
#include "dmx.h"


volatile uint8_t universeFPS[DMX_NUM_UNIVERSES];
volatile uint8_t universeFPSCounter[DMX_NUM_UNIVERSES] = {0};

uint8_t getUniverseFPS(uint8_t universe) {
    if(universe < DMX_NUM_UNIVERSES) {
        return universeFPS[universe];
    } else {
        return 0;
    }
}

void universeReceivedTMP(uint8_t universe) {
    if(universe < DMX_NUM_UNIVERSES) {
        universeFPSCounter[universe]++;
    }
}

void doTimerStuff1Hz(void) {
    PORTA.DIRTGL = 0x03;

    for(uint8_t i=0; i < 4; i++) {
        universeFPS[i] = universeFPSCounter[i];
        universeFPSCounter[i] = 0;
    }
}