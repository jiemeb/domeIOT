/// @file
/// Ports library definitions.
// 2009-02-13 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include "Ports.h"
#include <avr/sleep.h>
#include <util/atomic.h>

// #define DEBUG_DHT 1 // add code to send info over the serial port of non-zero

// ATtiny84 has BODS and BODSE for ATtiny84, revision B, and newer, even though
// the iotnx4.h header doesn't list it, so we *can* disable brown-out detection!
// See the ATtiny24/44/84 datasheet reference, section 7.2.1, page 34.
#if (defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny44__)) && \
    !defined(BODSE) && !defined(BODS)
#define BODSE 2
#define BODS  7
#endif

// flag bits sent to the receiver
#define MODE_CHANGE 0x80    // a pin mode was changed
#define DIG_CHANGE  0x40    // a digital output was changed
#define PWM_CHANGE  0x30    // an analog (pwm) value was changed on port 2..3
#define ANA_MASK    0x0F    // an analog read was requested on port 1..4

/// @class PortI2C
/// @details
/// The PortI2C class is a special version of class Port implementing the I2C /
/// Two-Wire Interface (TWI) protocol. Allows using any port as I2C bus master.
/// When used for I2C, DIO is used as SDA and AIO as SCL.
/// Unlike the Wire library for the Arduino, which is a more advanced solution
/// for the hardware I2C lines of an ATmega, the PortI2C class is implemented
/// entirely in software using "bit-banging". Another difference is that
/// PortI2C does not use interrupts and will keep the microcontroller occupied
/// while it is performing I/O transfers.
/// @see DeviceI2C

/// @class DeviceI2C
/// @details
/// Since I2C is a bus, there are actually two classes involved. A PortI2C
/// object manages a port as I2C master for one or more objects of class
/// DeviceI2C, each representing a separate device. You can have multiple ports
/// as I2C bus (running at a different speed perhaps), each talking to multiple
/// I2C devices. Devices sharing the same bus must each have a unique ID in the
/// range 0 .. 127.
/// @see PortI2C

/// @fn uint32_t Port::pulse(uint8_t state, uint32_t timeout =1000000L) const
/// @details
/// Measure the length of a pulse in microseconds on the DIO (pulse) or
/// AIO (pulse2) line. The optional timeout value specifies how many
/// microseconds to wait for a pulse - of none is received, 0 is returned.
/// @param state Polarity of the pulse to wait for - HIGH (1) or LOW (0).
/// @param timeout Max number of microseconds to wait.
///                Default is 1,000,000, i.e. 1 second.
/// @see http://arduino.cc/en/Reference/pulseIn for more details.

/// @fn void Port::shift(uint8_t bitOrder, uint8_t value) const
/// @details
/// This can be used to send out a pulse sequence of bits or to read such
/// a pulse sequence in. The AIO line is cycled while the value bits are
/// "shifted" and written out to (shift, shiftWrite) or read in from
/// (shiftRead) the DIO pin.
/// @param bitOrder How to shift bits in or out: either LSBFIRST (0) or
///                 MSBFIRST (1), where LSB stands for Least Significant
///                 Bit and MSB for Most Significant Bit.
/// @param value The value to shift out, with as many lower bits as needed.
/// This argument is a byte for shift() and a word for the more general
/// shiftWrite() function.
/// @see http://arduino.cc/en/Tutorial/ShiftOut

/// @fn static void Sleepy::powerDown ();
/// Take the ATmega into the deepest possible power down state. Getting out of
/// this state requires setting up the watchdog beforehand, or making sure that
/// suitable interrupts will occur once powered down.
/// Disables the Brown Out Detector (BOD), the A/D converter (ADC), and other
/// peripheral functions such as TWI, SPI, and UART before sleeping, and
/// restores their previous state when back up.

/// Shift a number of bites in to read them.
/// @param bitOrder How to shift bits in or out: either LSBFIRST (0) or
///                 MSBFIRST (1), where LSB stands for Least Significant
///                 Bit and MSB for Most Significant Bit.
/// @param count The number of bits to shift in or out. Must be in the
/// range 1 .. 16, the default is 8.
/// @see shift()
uint16_t Port::shiftRead(uint8_t bitOrder, uint8_t count) const {
    uint16_t value = 0, mask = bit(LSBFIRST ? 0 : count - 1);
    for (uint8_t i = 0; i < count; ++i) {
        digiWrite2(1);
        delayMicroseconds(5);
        if (digiRead())
            value |= mask;
        if (bitOrder == LSBFIRST)
            mask <<= 1;
        else
            mask >>= 1;
        digiWrite2(0);
        delayMicroseconds(5);
    }
    return value;
}

/// The shiftWrite() call is similar but more general than the shift() call
/// in that it allows an adjustable number of bits to be sent, not just 8.
/// @param bitOrder How to shift bits in or out: either LSBFIRST (0) or
///                 MSBFIRST (1), where LSB stands for Least Significant
///                 Bit and MSB for Most Significant Bit.
/// @param value The value to shift out, with as many lower bits as needed.
/// This argument is a byte for shift() and a word for the more general
/// shiftWrite() function.
/// @param count The number of bits to shift in or out. Must be in the
/// range 1 .. 16, the default is 8.
/// @see shift()
void Port::shiftWrite(uint8_t bitOrder, uint16_t value, uint8_t count) const {
    uint16_t mask = bit(LSBFIRST ? 0 : count - 1);
    for (uint8_t i = 0; i < count; ++i) {
        digiWrite((value & mask) != 0);
        if (bitOrder == LSBFIRST)
            mask <<= 1;
        else
            mask >>= 1;
        digiWrite2(1);
        digiWrite2(0);
    }
}


byte MilliTimer::poll(word ms) {
    byte ready = 0;
    if (armed) {
        word remain = next - millis();
        // since remain is unsigned, it will overflow to large values when
        // the timeout is reached, so this test works as long as poll() is
        // called no later than 5535 millisecs after the timer has expired
        if (remain <= 60000)
            return 0;
        // return a value between 1 and 255, being msecs+1 past expiration
        // note: the actual return value is only reliable if poll() is
        // called no later than 255 millisecs after the timer has expired
        ready = -remain;
    }
    set(ms);
    return ready;
}

word MilliTimer::remaining() const {
    word remain = armed ? next - millis() : 0;
    return remain <= 60000 ? remain : 0;
}

void MilliTimer::set(word ms) {
    armed = ms != 0;
    if (armed)
        next = millis() + ms - 1;
}





// ISR(WDT_vect) { Sleepy::watchdogEvent(); }

static volatile byte watchdogCounter;

void Sleepy::watchdogInterrupts (char mode) {
    // correct for the fact that WDP3 is *not* in bit position 3!
    if (mode & bit(3))
        mode ^= bit(3) | bit(WDP3);
    // pre-calculate the WDTCSR value, can't do it inside the timed sequence
    // we only generate interrupts, no reset
    byte wdtcsr = mode >= 0 ? bit(WDIE) | mode : 0;
    MCUSR &= ~(1<<WDRF);
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
#ifndef WDTCSR
#define WDTCSR WDTCR
#endif
        WDTCSR |= (1<<WDCE) | (1<<WDE); // timed sequence
        WDTCSR = wdtcsr;
    }
}

/// @see http://www.nongnu.org/avr-libc/user-manual/group__avr__sleep.html
void Sleepy::powerDown () {
    byte adcsraSave = ADCSRA;
    ADCSRA &= ~ bit(ADEN); // disable the ADC
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        sleep_enable();
        // sleep_bod_disable(); // can't use this - not in my avr-libc version!
#ifdef BODSE
        MCUCR = MCUCR | bit(BODSE) | bit(BODS); // timed sequence
        MCUCR = (MCUCR & ~ bit(BODSE)) | bit(BODS);
#endif
    }
    sleep_cpu();
    sleep_disable();
    // re-enable what we disabled
    ADCSRA = adcsraSave;
}

byte Sleepy::loseSomeTime (word msecs) {
    byte ok = 1;
    word msleft = msecs;
    // only slow down for periods longer than the watchdog granularity
    while (msleft >= 16) {
        char wdp = 0; // wdp 0..9 corresponds to roughly 16..8192 ms
        // calc wdp as log2(msleft/16), i.e. loop & inc while next value is ok
        for (word m = msleft; m >= 32; m >>= 1)
            if (++wdp >= 9)
                break;
        watchdogCounter = 0;
        watchdogInterrupts(wdp);
        powerDown();
        watchdogInterrupts(-1); // off
        // when interrupted, our best guess is that half the time has passed
        word halfms = 8 << wdp;
        msleft -= halfms;
        if (watchdogCounter == 0) {
            ok = 0; // lost some time, but got interrupted
            break;
        }
        msleft -= halfms;
    }
    // adjust the milli ticks, since we will have missed several
#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__) || defined (__AVR_ATtiny44__) || defined (__AVR_ATtiny45__)
    extern volatile unsigned long millis_timer_millis;
    millis_timer_millis += msecs - msleft;
#else
    extern volatile unsigned long timer0_millis;
    timer0_millis += msecs - msleft;
#endif
    return ok; // true if we lost approx the time planned
}

void Sleepy::watchdogEvent() {
    ++watchdogCounter;
}

Scheduler::Scheduler (byte size) : remaining (~0), maxTasks (size) {
    byte bytes = size * sizeof *tasks;
    tasks = (word*) malloc(bytes);
    memset(tasks, 0xFF, bytes);
}

Scheduler::Scheduler (word* buf, byte size) : tasks (buf), remaining(~0), maxTasks (size) {
    byte bytes = size * sizeof *tasks;
    memset(tasks, 0xFF, bytes);
}

char Scheduler::poll() {
    // all times in the tasks array are relative to the "remaining" value
    // i.e. only remaining counts down while waiting for the next timeout
    if (remaining == 0) {
        word lowest = ~0;
        for (byte i = 0; i < maxTasks; ++i) {
            if (tasks[i] == 0) {
                tasks[i] = ~0;
                return i;
            }
            if (tasks[i] < lowest)
                lowest = tasks[i];
        }
        if (lowest != ~0U) {
            for (byte i = 0; i < maxTasks; ++i) {
                if(tasks[i] != ~0U) {
                    tasks[i] -= lowest;
                }
            }
        } else {
            // must turn off timer or it might overflow if its poll-method
            // is not called within 5535 ms, i.e. if no tasks are scheduled
            ms100.set(0);
        }
        remaining = lowest;
    } else if (remaining == ~0U) //remaining == ~0 means nothing running
        return -2;
    else if (ms100.poll(100))
        --remaining;
    return -1;
}

char Scheduler::pollWaiting() {
    if(remaining == ~0U)  // Nothing running!
        return -2;
    // first wait until the remaining time we need to wait is less than 0.1s
    while (remaining > 0) {
        word step = remaining > 600 ? 600 : remaining;
        if (!Sleepy::loseSomeTime(100 * step)) // uses least amount of power
            return -1;
        remaining -= step;
    }
    // now lose some more time until that 0.1s mark
    if (!Sleepy::loseSomeTime(ms100.remaining()))
        return -1;
    // lastly, just ignore the 0..15 ms still left to go until the 0.1s mark
    return poll();
}

void Scheduler::timer(byte task, word tenths) {
    // if new timer will go off sooner than the rest, then adjust all entries
    if (tenths < remaining) {
        word diff = remaining - tenths;
        for (byte i = 0; i < maxTasks; ++i)
            if (tasks[i] != ~0U)
                tasks[i] += diff;
        remaining = tenths;
    }
    tasks[task] = tenths - remaining;
}

void Scheduler::cancel(byte task) {
    tasks[task] = ~0;
}


