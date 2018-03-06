#include "enc28j60.h"
#include "enc28j60_defs.h"

//#include "system.h"


#include <avr/interrupt.h>
#include <util/delay.h>

#include "display.h"
#include "dmx.h"

// buffer boundaries applied to internal 8K ram
// the entire available packet buffer space is allocated

#define RXSTART_INIT        0x0000  // start of RX buffer, (must be zero, Rev. B4 Errata point 5)
#define RXSTOP_INIT         0x1FFF  // end of RX buffer, room for 2 packets


//#define TXSTART_INIT        0x0C00  // start of TX buffer, room for 1 packet
//#define TXSTOP_INIT         0x11FF  // end of TX buffer

#define SSPIN PIN4_bm

static void enableChip (void) {
    cli();
    PORTD.OUTCLR = SSPIN; //TODO: Check
}

static void disableChip (void) {
    PORTD.OUTSET = SSPIN;
    sei();
}

static void xferSPI (uint8_t data) {
    SPID.DATA = data;

    while (!(SPID.STATUS & SPI_IF_bm));
}

static uint8_t readOp (uint8_t op, uint8_t address) {
    enableChip();
    xferSPI(op | (address & ADDR_MASK));
    xferSPI(0x00);
    if (address & 0x80)
        xferSPI(0x00);
    uint8_t result = SPID.DATA;
    disableChip();
    return result;
}

static void writeOp (uint8_t op, uint8_t address, uint8_t data) {
    enableChip();
    xferSPI(op | (address & ADDR_MASK));
    xferSPI(data);
    disableChip();
}

static void readBuf(uint16_t len, uint8_t* data) {
    uint8_t nextByte;

    enableChip();
    if (len != 0) {    
        xferSPI(ENC28J60_READ_BUF_MEM);
          
        SPID.DATA = 0x00; 
        while (--len) {
            while (!(SPID.STATUS & SPI_IF_bm));

            nextByte = SPID.DATA;
            SPID.DATA = 0x00;
            *data++ = nextByte;     
        }
        while (!(SPID.STATUS & SPI_IF_bm));
        *data++ = SPID.DATA;    
    }
    disableChip(); 
}

static void writeBuf(uint16_t len, const uint8_t* data) {
    enableChip();
    if (len != 0) {
        xferSPI(ENC28J60_WRITE_BUF_MEM);
           
        SPID.DATA = *data++;    
        while (--len) {
            uint8_t nextByte = *data++;

          while (!(SPID.STATUS & SPI_IF_bm));
            SPID.DATA = nextByte;
      };  
        while (!(SPID.STATUS & SPI_IF_bm));
    }
    disableChip();
}

static uint8_t Enc28j60Bank;

static void SetBank (uint8_t address) {
    if ((address & BANK_MASK) != Enc28j60Bank) {
        writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_BSEL1|ECON1_BSEL0);
        Enc28j60Bank = address & BANK_MASK;
        writeOp(ENC28J60_BIT_FIELD_SET, ECON1, Enc28j60Bank>>5);
    }
}

static uint8_t readRegByte (uint8_t address) {
    SetBank(address);
    return readOp(ENC28J60_READ_CTRL_REG, address);
}

static uint16_t readReg(uint8_t address) {
    return readRegByte(address) + (readRegByte(address+1) << 8);
}

static void writeRegByte (uint8_t address, uint8_t data) {
    SetBank(address);
    writeOp(ENC28J60_WRITE_CTRL_REG, address, data);
}

static void writeReg(uint8_t address, uint16_t data) {
    writeRegByte(address, data);
    writeRegByte(address + 1, data >> 8);
}

static uint16_t readPhyByte (uint8_t address) {
    writeRegByte(MIREGADR, address);
    writeRegByte(MICMD, MICMD_MIIRD);
    while (readRegByte(MISTAT) & MISTAT_BUSY)
        ;
    writeRegByte(MICMD, 0x00);
    return readRegByte(MIRD+1);
}

static void writePhy (uint8_t address, uint16_t data) {
    writeRegByte(MIREGADR, address);
    writeReg(MIWR, data);

    while (readRegByte(MISTAT) & MISTAT_BUSY);
}



uint8_t isMyBitch(void) {
    return (SPID.CTRL & SPI_MASTER_bm) ? 0 : 1;
}


/**
 * 
 * 
 */
uint8_t setupEthernet(void) {

    //////////////////////
    //1. Setup Pin   //
    //////////////////////
    PORTD.DIRSET = SSPIN;	 //D4 = CS
	PORTD.OUTSET = SSPIN;

    //Other SPI PINs
    //PORTD.PIN4CTRL = PORT_OPC_PULLUP_gc;
    PORTD.DIRSET = /*PIN4_bm |*/ PIN5_bm | PIN7_bm; //SCK and MOSI = OUT
	PORTD.OUTSET = PIN5_bm | PIN7_bm;


    //FROM http://lb9mg.no/2017/03/31/xmega-high-performance-spi-with-dma/
    PORTD.DIRCLR = PIN6_bm; /*MISO*/
    PORTD.OUTSET = SSPIN; /*CS high*/
    //PORTD.PIN5CTRL = PORT_INVEN_bm; /*CLOCK HAS TO BE INVERTED!!*/

    //////////////////////
    //2. Setup SPI on   //
    //////////////////////


    //SPI_CLK2X_bm => Double Spedd when in master mode?(P.228)

    SPID.CTRL = (SPID.CTRL & ~SPI_MODE_gm) |
			((SPI_MODE0_bm) & SPI_MODE_gm);

    SPID.CTRL = SPI_MASTER_bm ; //Prescaler default=00 -> Clk/4
    SPID.INTCTRL = 0x00; //Interruptlevel


    SPID.CTRL |= SPI_ENABLE_bm;

    //PORTD.DIRSET =  SPI_MOSI_bm | SPI_SCK_bm;

    PORTD.DIRSET = SSPIN;	 //D4 = CS
	PORTD.OUTSET = SSPIN;


    writeOp(0x01, 0x00, 0x02);

    //return;

    //////////////////////
    //3. Setup ENC28J60 //
    //////////////////////
    static uint8_t macaddr[] = { 0x70,0x69,0x69,0x2D,0x30,0x31 };

    disableChip();

    writeOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    
    //delay(2); // errata B7/2
    _delay_ms(2);
    while (! (readOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY));


    writeReg(ERXST, RXSTART_INIT);
    writeReg(ERXRDPT, RXSTART_INIT);
    writeReg(ERXND, RXSTOP_INIT);
    //writeReg(ETXST, TXSTART_INIT);
    //writeReg(ETXND, TXSTOP_INIT);

    writeRegByte(ERXFCON, ERXFCON_UCEN /*|ERXFCON_CRCEN|ERXFCON_PMEN*/|ERXFCON_BCEN); //Filter Config TODO: Check
    writeReg(EPMM0, 0x303f);
    writeReg(EPMCS, 0xf7f9);

    //Manual 6.5
    writeRegByte(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS); //Enable to receive frames, TXPAUS and RXPAUS for Flowcontroll    
    writeRegByte(MACON2, 0x00);
    
    writeOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);//TODO: Full Duplex?
    //TODO: MACON4.DEFER Bit for IEEE compilance?
    
    writeReg(MAIPG, 0x0C12);
    writeRegByte(MABBIPG, 0x12);// 0x12=used by HalfDurplex 0x15=used by fullduplex
    writeReg(MAMXFL, MAX_FRAMELEN);
    
    writeRegByte(MAADR5, macaddr[0]);
    writeRegByte(MAADR4, macaddr[1]);
    writeRegByte(MAADR3, macaddr[2]);
    writeRegByte(MAADR2, macaddr[3]);
    writeRegByte(MAADR1, macaddr[4]);
    writeRegByte(MAADR0, macaddr[5]);
    
    writePhy(PHCON2, PHCON2_HDLDIS); //Loopback disable bit
    
    SetBank(ECON1);
    writeOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE); // Set Receive Interrutp Enabled
    writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN); //Enable Reception


    PORTA.OUTSET = 0x02;

    uint8_t rev = readRegByte(EREVID);

    PORTA.OUTCLR = 0x02;
    return rev;
}



uint8_t buffer[MAX_FRAMELEN];

uint16_t packetReceive(void) {
    static uint16_t gNextPacketPtr = RXSTART_INIT;
    static uint8_t     unreleasedPacket = 0;
    uint16_t len = 0;

    if (unreleasedPacket) {
        if (gNextPacketPtr == 0) 
            writeReg(ERXRDPT, RXSTOP_INIT);
        else
            writeReg(ERXRDPT, gNextPacketPtr - 1);
        unreleasedPacket = 0;
    }

    if (readRegByte(EPKTCNT) > 0) {
        writeReg(ERDPT, gNextPacketPtr);

        struct {
            uint16_t nextPacket;
            uint16_t byteCount;
            uint16_t status;
        } header;

        readBuf(sizeof header, (uint8_t*) &header);

        gNextPacketPtr  = header.nextPacket;
        len = header.byteCount - 4; //remove the CRC count
        if (len>MAX_FRAMELEN-1)
            len=MAX_FRAMELEN-1;
        if ((header.status & 0x80)==0)
            len = 0;
        else
            readBuf(len, buffer);
        buffer[len] = 0;
        unreleasedPacket = 1;

        writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
    }
    return len;
}





void packageLoop(uint16_t len) {
  struct {
            uint8_t destAddr[6];
            uint8_t srcAddr[6];
            uint16_t length;
            
            uint8_t* data;
  } macPackage;
  
  memcpy(&macPackage, buffer, sizeof(macPackage));
  macPackage.data = ((uint8_t*)buffer) + 14; //Offset of data[]

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
      uint8_t* data;
  } ethPackage;


  memcpy(&ethPackage, macPackage.data, sizeof(ethPackage));
  ethPackage.data = macPackage.data + 20; //Offset of data[]

  //Serial.println("Eth Header:");
  //Serial.print("Length: " ); Serial.println(ethPackage.lengthH << 8 | ethPackage.lengthL, DEC);
  //Serial.print("Protocoll: 0x" ); Serial.println(ethPackage.Protocoll, HEX);

  //Serial.print("SrcIP: " ); Serial.print(ethPackage.srcIP[0], HEX); Serial.print(ethPackage.srcIP[1], HEX);Serial.print(ethPackage.srcIP[2], HEX); Serial.println(ethPackage.srcIP[3], HEX);


  if(ethPackage.Protocoll == 17) { // 17 == UDP
    struct {
      uint8_t srcPortH;
      uint8_t srcPortL;
      uint8_t destPortH;
      uint8_t destPortL;
      uint8_t lengthH;
      uint8_t lengthL;
      uint16_t checksum;
      uint8_t* data;
    } udpPackage;
    
    memcpy(&udpPackage, ethPackage.data, sizeof(udpPackage));
    udpPackage.data = ethPackage.data + 8; //Offset of *data

    //Serial.println(" Package is UDP");
    //Serial.print("   src Port: "); Serial.println(udpPackage.srcPortH << 8 | udpPackage.srcPortL);
    //Serial.print("   destPort: "); Serial.println(udpPackage.destPortH << 8 | udpPackage.destPortL);
    //Serial.print("   length: "); Serial.println(udpPackage.lengthH << 8 | udpPackage.lengthL);

    
  

      if(memcmp("Art-Net", udpPackage.data, 7) == 0) { //TODO: Check last 0x00
        //Serial.println("Starts with Art-Net");
        uint16_t opCode = udpPackage.data[8] | udpPackage.data[9] << 8;

        if(opCode = 0x5000) { //ART_DMX
          uint8_t sequence = udpPackage.data[12];
          uint16_t universe = udpPackage.data[14] | udpPackage.data[15] << 8;
          uint16_t dmxDataLength = udpPackage.data[17] | udpPackage.data[16] << 8; //TODO: Check endian

          if(dmxDataLength > 512) {
              //TODO: Log error
              dmxDataLength = 512;
          }

          //Serial.print("Seq: ");  Serial.println(sequence); 
          //Serial.print("Universe: ");  Serial.println(universe); 
          //Serial.print("Data Length: ");  Serial.println(dmxDataLength); 

          /*Serial.print("Ch 1: ");  Serial.println(udpPackage.data[18]); 
          Serial.print("Ch 2: ");  Serial.println(udpPackage.data[19]); 
          Serial.print("Ch 3: ");  Serial.println(udpPackage.data[20]); 
          Serial.print("Ch 4: ");  Serial.println(udpPackage.data[21]); */

            

            //Copy data to DMX Buffer
          memcpy(dmxBuffer[0] , udpPackage.data + 18, dmxDataLength);
          
        }
        
      }

  }

}