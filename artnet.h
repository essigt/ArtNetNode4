#ifndef _ARTNET_H
#define _ARTNET_H   1

#include <avr/io.h>
#include "net.h"

// Port to listen on
#define UDP_PORT_ARTNET       6454  /* (0x1936) */
// Port to reply on
#define UDP_PORT_ARTNET_REPLY (UDP_PORT_ARTNET + 1)




class ArtNet {

public:
    static void udpArtNet(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len);

    static uint8_t getUniverseFPS(uint8_t universe);

    static void universeReceivedTMP(uint8_t universe);

    static void doTimerStuff1Hz(void);

private:
    static void sendArtPollReplay(void);
};


extern ArtNet artnet;



#endif