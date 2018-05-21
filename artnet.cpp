#include "artnet.h"
#include "dmx.h"
#include "EtherCard.h"

#include <string.h> //memcpy


volatile uint8_t universeFPS[DMX_NUM_UNIVERSES];
volatile uint8_t universeFPSCounter[DMX_NUM_UNIVERSES] = {0};

ArtNet artnet; //Global instance of ArtNet class

// Opcodes
#define ART_POLL        0x2000
#define ART_POLL_REPLAY 0x2100
#define ART_DMX         0x5000
// Buffers
#define MAX_BUFFER_ARTNET 530
// Packet
#define ART_NET_ID "Art-Net\0"



/**
 * Receive ArtNet Packages from Ethercard 
 * 
 * 
 **/
void ArtNet::udpArtNet(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len) {
	uint8_t result = strncmp(data, "Art-Net",7);

	uint16_t opCode = (data[9] << 8) | data[8];
	uint16_t protocolVersion = data[10] << 8 | data[11];

	uint16_t universe = data[14] | data[15] << 8 ;
	uint16_t length = data[16] << 8 | data[17];


    if(opCode == ART_POLL) {
        PORTA.OUTTGL = 0x02;
        sendArtPollReplay();
    } else if(opCode == ART_DMX) {
		if (universe < 4) { 
            memcpy(dmxBuffer[universe], data + 18, length);
            universeReceivedTMP(universe);
        }
	}
}

uint8_t ArtNet::getUniverseFPS(uint8_t universe) {
    if(universe < DMX_NUM_UNIVERSES) {
        return universeFPS[universe];
    } else {
        return 0;
    }
}

void ArtNet::universeReceivedTMP(uint8_t universe) {
    if(universe < DMX_NUM_UNIVERSES) {
        universeFPSCounter[universe]++;
    }
}

void ArtNet::doTimerStuff1Hz(void) {
    for(uint8_t i=0; i < 4; i++) {
        universeFPS[i] = universeFPSCounter[i];
        universeFPSCounter[i] = 0;
    }
}

void ArtNet::sendArtPollReplay(void) {
    //Build ArtPollReplay Package
    struct ArtPollReplay_t {
        uint8_t id[8];
        uint8_t opCode[2];
        uint8_t ipAddress[4];
        uint8_t port[2];
        uint8_t versInfoH;
        uint8_t versInfoL;
        uint8_t netSwitch;
        uint8_t subSwitch;
        uint8_t oemHi;
        uint8_t oem;
        uint8_t ubeaVersion;
        uint8_t status1;
        uint8_t estaManLo;
        uint8_t estaManHi;
        uint8_t shortName[18];
        uint8_t longName[64];
        uint8_t nodeReport[64];
        uint8_t numPortsHi;
        uint8_t numPortsLo;
        uint8_t portTypes[4];
        uint8_t goodInput[4];
        uint8_t goodOutput[4];
        uint8_t swIn[4];
        uint8_t swOut[4];
        uint8_t swVideo;
        uint8_t swMacro;
        uint8_t swRemote;
        uint8_t spare[3];
        uint8_t style;
        uint8_t mac[6];
        uint8_t bindIp[4];
        uint8_t bindIndex;
        uint8_t status2;
        uint8_t filler[26];
    } replay;

    memcpy(replay.id,ART_NET_ID, 8);
    replay.opCode[0] = ART_POLL_REPLAY;
    replay.opCode[1] = ART_POLL_REPLAY >> 8;
    
    memcpy(replay.ipAddress, ether.myip, IP_LEN);

    replay.port[0] = 0x36;
    replay.port[1] = 0x19;

    replay.versInfoH = 0;
    replay.versInfoL = 1;

    replay.netSwitch = 0;
    replay.subSwitch = 0;

    replay.oemHi = 0;
    replay.oem = 0;

    replay.ubeaVersion = 0;

    replay.status1 = 0xF0;

    replay.estaManLo = 'E';
    replay.estaManHi = 'T';

    memcpy(replay.shortName,"ArtNetNode 4      ", 18);
    memcpy(replay.longName,"ArtNetNode 4 LONG     ", 22);
    //memcpy(replay.nodeReport,"ArtNetNode 4 LONG     ", 64);

    replay.numPortsHi = 0;
    replay.numPortsLo = 4;

    //4x ArtNet Output + DMX512 Mode
    replay.portTypes[0] = 1 << 7;
    replay.portTypes[1] = 1 << 7;
    replay.portTypes[2] = 1 << 7;
    replay.portTypes[3] = 1 << 7;


    //replay.goodInput[] //TODO: Unused, transmit as zero

    //TODO: Relay check transmission status
    replay.goodOutput[0] = 1 << 7;
    replay.goodOutput[1] = 1 << 7;
    replay.goodOutput[2] = 1 << 7;
    replay.goodOutput[3] = 1 << 7;

    replay.style = 0x00; //Style 0x00 -> A DMX to7from Art-Net device

    memcpy(replay.mac, ether.mymac, ETH_LEN);
    memcpy(replay.bindIp, ether.myip, IP_LEN);
    
    replay.bindIndex = 1; //Root device

    replay.status2 = 1 << 3; //Supports ArtNed 3 + 4 Port adresses

    //TODO: Don't use static ip
    uint8_t hisip[4] = {0x02, 0x0, 0x0, 123};

    //static void sendUdp (char *data,uint8_t len,uint16_t sport, uint8_t *dip, uint16_t dport);      
     ether.sendUdp((char*)&replay, sizeof(replay), UDP_PORT_ARTNET_REPLY, hisip, UDP_PORT_ARTNET_REPLY ); //*/
}