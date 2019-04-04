// 2009-02-13 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#ifndef Ports_h
#define Ports_h

// JMB
//#include <SoftwareSerial.h>
//#include <util/delay.h>



//extern SoftwareSerial Serial(const int,const int);
///JMB

/// @file
/// Ports library definitions.

#if ARDUINO >= 100
#include <Arduino.h> // Arduino 1.0
#else
#include <WProgram.h> // Arduino 0022
#endif
#include <stdint.h>
#include <avr/pgmspace.h>
//#include <util/delay.h>

// tweak this to switch ATtiny84 etc to new Arduino 1.0+ conventions
// see http://arduino.cc/forum/index.php/topic,51984.msg371307.html#msg371307
// and http://forum.jeelabs.net/node/1567
#if ARDUINO >= 100
#define WRITE_RESULT size_t
#else
#define WRITE_RESULT void
#endif

/// Interface for JeeNode Ports - see the wiki docs for
/// [JeeNodes](http://jeelabs.net/projects/hardware/wiki/JeeNode) and
/// [pinouts](http://jeelabs.net/projects/hardware/wiki/Pinouts).
/// The Ports class is a thin wrapper around the Arduino's digitalRead(),
/// digitalWrite(), analogRead(), etc. functions. It was designed to simplify
/// the use of the four standard port headers on JeeNodes.
class Port {
protected:
	/// The port number is a small integer mathing the hardware port used.
    /// Port 0 is special, it designates the I2C hardware pins on a JeeNode.
    uint8_t portNum;

#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__)
	/// @return Arduino digital pin number of a Port's D pin (uint8_t).
    inline uint8_t digiPin() const
        { return 0; }
	/// @return Arduino digital pin number of a Port's A pin (uint8_t).
    inline uint8_t digiPin2() const
        { return 2; }
	/// @return Arduino digital pin number of the I pin on all Ports (uint8_t).
    static uint8_t digiPin3()
        { return 1; }
    /// @return Arduino analog pin number of a Port's A pin (uint8_t).
    inline uint8_t anaPin() const
        { return 0; }
#elif defined(__AVR_ATtiny84__)
	/// @return Arduino digital pin number of a Port's D pin (uint8_t).
    inline uint8_t digiPin() const
        { return 12 - 2 * portNum; }
	/// @return Arduino digital pin number of a Port's A pin (uint8_t).
    inline uint8_t digiPin2() const
        { return 11 - 2 * portNum; }
	/// @return Arduino digital pin number of the I pin on all Ports (uint8_t).
    static uint8_t digiPin3()
        { return 3; }
    /// @return Arduino analog pin number of a Port's A pin (uint8_t).
    inline uint8_t anaPin() const
        { return 11 - 2 * portNum; }
#else
	/// @return Arduino digital pin number of a Port's D pin (uint8_t).
    inline uint8_t digiPin() const
        { return portNum ? portNum + 3 : 18; }
	/// @return Arduino digital pin number of a Port's A pin (uint8_t).
    inline uint8_t digiPin2() const
        { return portNum ? portNum + 13 : 19; }
	/// @return Arduino digital pin number of the I pin on all Ports (uint8_t).
    static uint8_t digiPin3()
        { return 3; }
    /// @return Arduino analog pin number of a Port's A pin (uint8_t).
    inline uint8_t anaPin() const
        { return portNum - 1; }
#endif

public:
	///Contructor for a Port.
    inline Port (uint8_t num) : portNum (num) {}

    // DIO pin

    /// Set the pin mode of a Port's D pin. The mode() function member sets the
    /// I/O data direction of the DIO pin associated with a specific port.
    /// @param value INPUT or OUTPUT.
    inline void mode(uint8_t value) const
        { pinMode(digiPin(), value); }
    /// Reads the value of a Port's D pin.
    /// @return High or Low.
    inline uint8_t digiRead() const
        { return digitalRead(digiPin()); }
	/// Write High or Low to a Port's D pin.
    /// @param value High or Low.
    inline void digiWrite(uint8_t value) const
        { return digitalWrite(digiPin(), value); }
    /// Writes a PWM value to a Port's D pin.
    inline void anaWrite(uint8_t val) const
        { analogWrite(digiPin(), val); }
    /// Applies the Arduino pulseIn() function on a Port's D pin.
    inline uint32_t pulse(uint8_t state, uint32_t timeout =1000000L) const
        { return pulseIn(digiPin(), state, timeout); }
    
    // AIO pin

    /// Set the pin mode of a Port's A pin. The mode2() function member sets
    /// the I/O data direction of the AIO pin associated with a specific port.
    /// @param value INPUT or OUTPUT.
    inline void mode2(uint8_t value) const
        { pinMode(digiPin2(), value); }
    /// Reads an analog value from a Port's A pin.
    /// @return int [0..1023]
    inline uint16_t anaRead() const
        { return analogRead(anaPin()); }        
	/// Reads the value of a Port's A pin.
    /// @return High or Low.
    inline uint8_t digiRead2() const
        { return digitalRead(digiPin2()); }
    /// Write High or Low to a Port's A pin.
    /// @param value High or Low.
    inline void digiWrite2(uint8_t value) const
        { return digitalWrite(digiPin2(), value); }
	/// Applies the Arduino pulseIn() function on a Port's A pin.
    /// @see http://arduino.cc/en/Reference/pulseIn for more details.
    inline uint32_t pulse2(uint8_t state, uint32_t timeout =1000000L) const
        { return pulseIn(digiPin2(), state, timeout); }
        
    // IRQ pin (INT1, shared across all ports)

    /// Set the pin mode of the I pin on all Ports. The mode3() function member
    /// sets the I/O direction of the IRQ pin associated with a specific port.
    /// Note that this is the same pin on all ports.
    /// @param value INPUT or OUTPUT.
    static void mode3(uint8_t value)
        { pinMode(digiPin3(), value); }
    /// Reads the value of the I pin on all Ports.
    /// @return High or Low.
    static uint8_t digiRead3()
        { return digitalRead(digiPin3()); }
    /// Writes the value of the I pin on all Ports.
    /// @param value High or Low.
    static void digiWrite3(uint8_t value)
        { return digitalWrite(digiPin3(), value); }
    /// Writes a PWM value to the I pin of all Ports.
    static void anaWrite3(uint8_t val)
        { analogWrite(digiPin3(), val); }
    
    // both pins: data on DIO, clock on AIO

    /// Does Arduino shiftOut() with data on D and clock on A pin of the Port.
    inline void shift(uint8_t bitOrder, uint8_t value) const
        { shiftOut(digiPin(), digiPin2(), bitOrder, value); }
    uint16_t shiftRead(uint8_t bitOrder, uint8_t count =8) const;
    void shiftWrite(uint8_t bitOrder, uint16_t value, uint8_t count =8) const;
};



/// The millisecond timer can be used for timeouts up to 60000 milliseconds.
/// Setting the timeout to zero disables the timer.
///
/// * for periodic use, poll the timer object with "if (timer.poll(123)) ..."
/// * for one-shot use, call "timer.set(123)" and poll as "if (timer.poll())"

class MilliTimer {
    word next;
    byte armed;
public:
    MilliTimer () : armed (0) {}
    
    /// poll until the timer fires
    /// @param ms Periodic repeat rate of the time, omit for a one-shot timer.
    byte poll(word ms =0);
    /// Return the number of milliseconds before the timer will fire
    word remaining() const;
    /// Returns true if the timer is not armed
    byte idle() const { return !armed; }
    /// set the one-shot timeout value
    /// @param ms Timeout value. Timer stops once the timer has fired.
    void set(word ms);
};

/// Low-power utility code using the Watchdog Timer (WDT). Requires a WDT
/// interrupt handler, e.g. EMPTY_INTERRUPT(WDT_vect);
class Sleepy {
public:
    /// start the watchdog timer (or disable it if mode < 0)
    /// @param mode Enable watchdog trigger after "16 << mode" milliseconds 
    ///             (mode 0..9), or disable it (mode < 0).
    /// @note If you use this function, you MUST included a definition of a WDT
    /// interrupt handler in your code. The simplest is to include this line:
    ///
    ///     ISR(WDT_vect) { Sleepy::watchdogEvent(); }
    ///
    /// This will get called when the watchdog fires.
    static void watchdogInterrupts (char mode);
    
    /// enter low-power mode, wake up with watchdog, INT0/1, or pin-change
    static void powerDown ();
    
    /// Spend some time in low-power mode, the timing is only approximate.
    /// @param msecs Number of milliseconds to sleep, in range 0..65535.
    /// @returns 1 if all went normally, or 0 if some other interrupt occurred
    /// @note If you use this function, you MUST included a definition of a WDT
    /// interrupt handler in your code. The simplest is to include this line:
    ///
    ///     ISR(WDT_vect) { Sleepy::watchdogEvent(); }
    ///
    /// This will get called when the watchdog fires.
    static byte loseSomeTime (word msecs);

    /// This must be called from your watchdog interrupt code.
    static void watchdogEvent();
};

/// simple task scheduler for times up to 6000 seconds
class Scheduler {
    word* tasks;
    word remaining;
    byte maxTasks;
    MilliTimer ms100;
public:
    /// initialize for a specified maximum number of tasks
    Scheduler (byte max);
    Scheduler (word* buf, byte max);

    /// Return next task to run, -1 if there are none ready to run, but there
    /// are tasks waiting, or -2 if there are no tasks waiting (i.e. all idle)
    char poll();
    /// same as poll, but wait for event in power-down mode.
    /// Uses Sleepy::loseSomeTime() - see comments there re requiring the
    /// watchdog timer. 
    char pollWaiting();
    
    /// set a task timer, in tenths of seconds
    void timer(byte task, word tenths);
    /// cancel a task timer
    void cancel(byte task);
    
    /// return true if a task timer is not running
    byte idle(byte task) { return tasks[task] == ~0U; }
};

#endif
