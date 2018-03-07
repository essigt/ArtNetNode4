#include "udp.h"

#include "dmx.h"
#include "enc28j60.h"


#define EHT_PROTOCOLL_UDP   17

#define ART_DMX             0x5000


static uint8_t eth_type_is_arp_and_my_ip(uint16_t len) {
    return len >= 41 && gPB[ETH_TYPE_H_P] == ETHTYPE_ARP_H_V &&
           gPB[ETH_TYPE_L_P] == ETHTYPE_ARP_L_V &&
           memcmp(gPB + ETH_ARP_DST_IP_P, EtherCard::myip, IP_LEN) == 0;
}

void packageLoop(uint16_t len) {
    if (len == 0)
        return;

    struct {
        uint8_t destAddr[6];
        uint8_t srcAddr[6];
        uint16_t length;    //TODO: LenH LenL?

        uint8_t *data;
    } macPackage;

    memcpy(&macPackage, buffer, sizeof(macPackage));
    macPackage.data = ((uint8_t *)buffer) + 14; //Offset of data[]

    struct {
        uint8_t stuff;
        uint8_t typeOfService;
        uint8_t lengthH;
        uint8_t lengthL;
        uint16_t identfication;
        uint16_t flagAndFragmentOffset;
        uint8_t TTL;
        uint8_t Protocoll;
        uint16_t headerChecksum;
        uint8_t srcIP[4];
        uint8_t destIP[4];
        uint8_t *data;
    } ethPackage;

    memcpy(&ethPackage, macPackage.data, sizeof(ethPackage));
    ethPackage.data = macPackage.data + 20; //Offset of data[]

    if (ethPackage.Protocoll == EHT_PROTOCOLL_UDP) {
        struct {
            uint8_t srcPortH;
            uint8_t srcPortL;
            uint8_t destPortH;
            uint8_t destPortL;
            uint8_t lengthH;
            uint8_t lengthL;
            uint16_t checksum;
            uint8_t *data;
        } udpPackage;

        memcpy(&udpPackage, ethPackage.data, sizeof(udpPackage));
        udpPackage.data = ethPackage.data + 8; //Offset of *data

        if (memcmp("Art-Net", udpPackage.data, 7) == 0) {
            uint16_t opCode = udpPackage.data[8] | udpPackage.data[9] << 8;

            if (opCode == ART_DMX) {
                uint8_t sequence = udpPackage.data[12];
                uint16_t universe = udpPackage.data[14] | udpPackage.data[15] << 8;
                uint16_t dmxDataLength = udpPackage.data[17] | udpPackage.data[16] << 8; //TODO: Check endian

                if (dmxDataLength > 512) {
                    //TODO: Log error
                    dmxDataLength = 512;
                }

                //Copy data to DMX Buffer
                if (universe < 4)
                {
                    memcpy(dmxBuffer[universe], udpPackage.data + 18, dmxDataLength);
                    universeReceivedTMP(universe);
                }
            }
        }
    }
}