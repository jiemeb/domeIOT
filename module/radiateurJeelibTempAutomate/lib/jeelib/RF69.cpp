#include <stdint.h>
#include <RF69.h>
#include <RF69_avr.h>

#define REG_FIFO            0x00
#define REG_OPMODE          0x01
#define REG_FRFMSB          0x07
#define REG_AFCFEI          0x1E
#define REG_AFCMSB          0x1F
#define REG_AFCLSB          0x20
#define REG_FEIMSB          0x21
#define REG_FEILSB          0x22
#define REG_RSSICONFIG      0x23
#define REG_RSSIVALUE       0x24
#define REG_DIOMAPPING1     0x25
#define REG_IRQFLAGS1       0x27
#define REG_IRQFLAGS2       0x28
#define REG_SYNCCONFIG      0x2E
#define REG_SYNCVALUE1      0x2F
#define REG_SYNCVALUE2      0x30
#define REG_SYNCVALUE3      0x31
#define REG_SYNCVALUE4      0x32
#define REG_SYNCVALUE5      0x33
#define REG_SYNCGROUP       0x33
#define REG_NODEADRS        0x39
#define REG_PACKETCONFIG2   0x3D
#define REG_AESKEY1         0x3E

#define MODE_SLEEP          0x00
#define MODE_STANDBY        0x04
#define MODE_RECEIVER       0x10
#define MODE_TRANSMITTER    0x0C

#define IRQ1_MODEREADY      0x80
#define IRQ1_RXREADY        0x40

#define IRQ2_FIFOFULL       0x80
#define IRQ2_FIFONOTEMPTY   0x40
#define IRQ2_FIFOOVERRUN    0x10
#define IRQ2_PACKETSENT     0x08
#define IRQ2_PAYLOADREADY   0x04

#define FeiDone             0x40
#define RssiStart           0x01
#define RssiDone            0x02

#define fourByteSync        0x98
#define fiveByteSync        0xA0

#define RF_MAX   72

// transceiver states, these determine what to do with each interrupt
enum { TXCRC1, TXCRC2, TXTAIL, TXDONE, TXIDLE, TXRECV };

namespace RF69 {
    uint32_t frf;
    uint8_t  group;
    uint8_t  node;
    uint16_t crc;
    uint8_t  rssi;
    int16_t  afc;
    int16_t  fei;
    uint16_t interruptCount;
}

static volatile uint8_t rxfill;     // number of data bytes in rf12_buf
static volatile int8_t rxstate;     // current transceiver state

static ROM_UINT8 configRegs_compat [] ROM_DATA = {
 // 0x01, 0x04, // OpMode = standby
 // 0x02, 0x00, // DataModul = packet mode, fsk
  0x03, 0x02, // BitRateMsb, data rate = 49,261 khz
  0x04, 0x8A, // BitRateLsb, divider = 32 MHz / 650
  0x05, 0x05, // FdevMsb = 90 KHz
  0x06, 0xC3, // FdevLsb = 90 KHz
  // 0x07, 0xD9, // FrfMsb, freq = 868.000 MHz
  // 0x08, 0x00, // FrfMib, divider = 14221312
  // 0x09, 0x00, // FrfLsb, step = 61.03515625
//  0x0B, 0x20, // AfcCtrl, afclowbetaon
  0x19, 0x49, // RxBw ...
  0x1A, 0x8B,   // Channel filter BW
  0x1E, 0x2F, //M17 0x2C, // FeiStart, AfcAutoclearOn, AfcAutoOn
  0x25, 0x80, // DioMapping1 = SyncAddress (Rx)
  0x29, 0xE4, // 0xC4, // RssiThresh ...

  0x2E, 0xA0, // SyncConfig = sync on, sync size = 5
  0x2F, 0xAA, // SyncValue1 = 0xAA
  0x30, 0xAA, // SyncValue2 = 0xAA
  0x31, 0xAA, // SyncValue3 = 0xAA
  0x32, 0x2D, // SyncValue4 = 0x2D
    // 0x33, 0xD4, // SyncValue5 = 212, Group
  0x37, 0x00, // PacketConfig1 = fixed, no crc, filt off
  0x38, 0x00, //j0x50, // PayloadLength = 0, unlimited
  0x3C, 0x8F, // FifoTresh, not empty, level 15
  0x3D, 0x10, // PacketConfig2, interpkt = 1, autorxrestart off
  0x6F, 0x30, //j0x20, // TestDagc ...
  0
};

uint8_t RF69::control(uint8_t cmd, uint8_t val) {
    PreventInterrupt irq0;
    return spiTransfer(cmd, val);
}

static void writeReg (uint8_t addr, uint8_t value) {
    RF69::control(addr | 0x80, value);
}

static uint8_t readReg (uint8_t addr) {
    return RF69::control(addr, 0);
}

static void flushFifo () {
    while (readReg(REG_IRQFLAGS2) & (IRQ2_FIFONOTEMPTY | IRQ2_FIFOOVERRUN))
        readReg(REG_FIFO);
}

static void setMode (uint8_t mode) {
    writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | mode);
    while ((readReg(REG_IRQFLAGS1) & IRQ1_MODEREADY) == 0)
         ;
}

static void initRadio (ROM_UINT8* init) {
    spiInit();
    // What is all this doing?
    do
        writeReg(REG_SYNCVALUE1, 0xAA);
    while (readReg(REG_SYNCVALUE1) != 0xAA);
    do
        writeReg(REG_SYNCVALUE1, 0x55);
    while (readReg(REG_SYNCVALUE1) != 0x55);
    for (;;) {
        uint8_t cmd = ROM_READ_UINT8(init);
        if (cmd == 0) break;
        writeReg(cmd, ROM_READ_UINT8(init+1));
        init += 2;
    }
}

void RF69::setFrequency (uint32_t freq) {
    // Frequency steps are in units of (32,000,000 >> 19) = 61.03515625 Hz
    // use multiples of 64 to avoid multi-precision arithmetic, i.e. 3906.25 Hz
    // due to this, the lower 6 bits of the calculated factor will always be 0
    // this is still 4 ppm, i.e. well below the radio's 32 MHz crystal accuracy
    // 868.0 MHz = 0xD90000, 868.3 MHz = 0xD91300, 915.0 MHz = 0xE4C000  
    frf = ((freq << 2) / (32000000L >> 11)) << 6;
}

bool RF69::canSend () {
    if (rxstate == TXRECV && rxfill == 0) {
        rxstate = TXIDLE;
        setMode(MODE_STANDBY);
        return true;
    }
    return false;
}

bool RF69::sending () {
    return rxstate < TXIDLE; // What happens here? return a value less than whatever TXIDLE is valued?
}

void RF69::sleep (bool off) {
    setMode(off ? MODE_SLEEP : MODE_STANDBY);
    rxstate = TXIDLE;
}

// References to the RF12 driver above this line will generate compiler errors!
#include <RF69_compat.h>
#include <RF12.h>

void RF69::configure_compat () {
    initRadio(configRegs_compat);
    if(group == 0) {
        writeReg(REG_SYNCCONFIG, fourByteSync);
    } else {
        writeReg(REG_SYNCCONFIG, fiveByteSync);
        writeReg(REG_SYNCGROUP, group);
    }   

    writeReg(REG_FRFMSB, frf >> 16);
    writeReg(REG_FRFMSB+1, frf >> 8);
    writeReg(REG_FRFMSB+2, frf);

    rxstate = TXIDLE;
}

uint8_t* recvBuf;

uint16_t RF69::recvDone_compat (uint8_t* buf) {
    switch (rxstate) {
    case TXIDLE:
        rxfill = rf12_len = 0;
//        crc = _crc16_update(~0, group);
 	crc = ~0 ;			// pa de checksum sur le GROUP
        recvBuf = buf;
        rxstate = TXRECV;
        flushFifo();
//        writeReg(REG_IRQFLAGS2, 0x10); // Clear FIFO with FifoOverrun
        setMode(MODE_RECEIVER);
        break;
    case TXRECV:
        //if (rxfill >= rf12_len + 5 || rxfill >= RF_MAX) {
        if (rxfill > rf12_len  || rxfill >= RF_MAX) {
            rxstate = TXIDLE;
            setMode(MODE_STANDBY);
//    writeReg(REG_DIOMAPPING1, 0x80); // SyncAddress
 
            if (rf12_len > RF12_MAXDATA) {
                crc = 1; // force bad crc for invalid packet                
//                writeReg(REG_IRQFLAGS2, 0x10); // Clear FIFO with FifoOverrun
            }
            if (!(rf12_hdr & RF12_HDR_DST) || node == 31 ||
                    (rf12_hdr & RF12_HDR_MASK) == node)
                return crc;
        }
    }
    return ~0;
}

void RF69::sendStart_compat (uint8_t hdr, const void* ptr, uint8_t len) {
    rf12_len_buf = len;
    rf12_len = len+4; // Node + len +CRC
    for (int i = 0; i < len; ++i)
        rf12_data[i] = ((const uint8_t*) ptr)[i];
    rf12_hdr = hdr & RF12_HDR_DST ? hdr : (hdr & ~RF12_HDR_MASK) + node;  
    crc = _crc16_update(~0, group);
    rxstate = - (2 + rf12_len_buf); // preamble and SYN1/SYN2 are sent by hardware
    flushFifo();
    setMode(MODE_TRANSMITTER);
    writeReg(REG_DIOMAPPING1, 0x00); // PacketSent
    
    // use busy polling until the last byte fits into the buffer
    // this makes sure it all happens on time, and that sendWait can sleep
    while (rxstate < TXDONE)
        if ((readReg(REG_IRQFLAGS2) & IRQ2_FIFOFULL) == 0) { // FIFO is only 64 bytes! 
            uint8_t out = 0xAA; // I'm lost here too, why not have it with the writeReg
            if (rxstate < 0) {
                out = recvBuf[3 + rf12_len_buf + rxstate];
                crc = _crc16_update(crc, out);
            } else {
                switch (rxstate) {
                    case TXCRC1: out = crc; break;
                    case TXCRC2: out = crc >> 8; break;
                }
            }
            writeReg(REG_FIFO, out); // Presume this outputs the 0xAA postamble to finish packet?
            ++rxstate;
        }
}

void RF69::interrupt_compat () {
    interruptCount++;
    IRQ_ENABLE; // allow nested interrupts from here on
        // Interrupt will remain asserted until FIFO empty or exit RX mode

        if (rxstate == TXRECV) {
            uint8_t f = false;
            rssi = readReg(REG_RSSIVALUE);
            fei  = readReg(REG_FEIMSB);
            fei  = (fei << 8) + readReg(REG_FEILSB);
            afc  = readReg(REG_AFCMSB);
            afc  = (afc << 8) + readReg(REG_AFCLSB);

            crc = ~0;
            /*
            for (uint16_t i = 0; i < 1024; i++) {
                if (readReg(REG_IRQFLAGS2) & (IRQ2_FIFONOTEMPTY|IRQ2_FIFOOVERRUN)) {
                    f = true;
                    break;
                }
            }
            // What happens if we empty the FIFO before packet reception complete
            // or if the FIFO is bigger than we eat - should be 66 max? - 
            // Rolling window!
            // What happens with we see sync but no payload?
    if (f) { */
            for (;;) { // busy loop, to get each data byte as soon as it comes in 
                if (readReg(REG_IRQFLAGS2) & (IRQ2_FIFONOTEMPTY|IRQ2_FIFOOVERRUN)) {
                    if (rxfill == 0 && group != 0) { 
                        recvBuf[rxfill++] = group;
                       // crc = _crc16_update(crc, group);
                    }

                    uint8_t in = readReg(REG_FIFO);
                    recvBuf[rxfill++] = in;
                    crc = _crc16_update(crc, in);              
                    //if (rxfill >= rf12_len + 5 || rxfill >= RF_MAX)
                    if (rxfill > rf12_len  || rxfill >= RF_MAX)
                        break;
                }
            }
//           }
           writeReg(REG_IRQFLAGS2, 0x10); // Clear FIFO with FifoOverrun
           writeReg(REG_AFCFEI, 0x2F);    // Clear AFC, start FEI 
// Make sure FIFO is empty - might deassert IRQ0
//          if (readReg(REG_IRQFLAGS2) & (IRQ2_FIFONOTEMPTY)) {
//          uint8_t in = readReg(REG_FIFO);
//          }
    // Make sure IRQ0 is deasserted
//    setMode(MODE_STANDBY);
//    setMode(MODE_RECEIVER);
//    writeReg(REG_DIOMAPPING1, 0x80); // SyncAddress  
    } else if (readReg(REG_IRQFLAGS2) & IRQ2_PACKETSENT) {
        // rxstate will be TXDONE at this point
        rxstate = TXIDLE;
        writeReg(REG_IRQFLAGS2, 0x10); // Clear FIFO with FifoOverrun 
        writeReg(REG_AFCFEI, 0x2F);    // Clear AFC, start FEI 
// We have just had an interrupt, mode standby should deassert IRQ0
// If we are in standby that is!
// Make sure FIFO is empty - might deassert IRQ0
//        if (readReg(REG_IRQFLAGS2) & (IRQ2_FIFONOTEMPTY|IRQ2_FIFOOVERRUN)) {
//            uint8_t in = readReg(REG_FIFO);
//        }
        setMode(MODE_STANDBY);
        writeReg(REG_IRQFLAGS2, 0x10); // Clear FIFO with FifoOverrun
        writeReg(REG_DIOMAPPING1, 0x80); // SyncAddress
        }
}