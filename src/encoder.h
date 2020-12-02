/** @file encoder.h
 *    blarg
 *  @author Jordan Kochavi
 *  @date 2020-Nov-10
 */

#ifndef ENCODER_H
#define ENCODER_H
#include <Arduino.h>
#include <PrintStream.h>
#if (defined STM32L4xx || defined STM32F4xx)
    #include <STM32FreeRTOS.h>
#endif
#include "taskshare.h"                                                  // Include task sharing library
#include "taskqueue.h"                                                  // Include taskqueue library

/** @brief   Defines the class for an Encoder.
 *  @details 
 */
class Encoder {
    protected:                             
        uint8_t A_pin;                                              // Encoder signal A pin
        uint8_t B_pin;                                              // Encoder signal B pin
        uint8_t Button_pin;                                         // Encoder button pin
        uint8_t current_reading;                                    // Variable used for determining rotation
        uint8_t previous_reading;                                   // Variable used for determining rotation
        bool debounce (uint8_t signal_pin, uint8_t limit);          // Function for debouncing press signal
    public:                                                         // 
        bool pressed;                                               // If the encoder button was pressed or not
        int count;                                                  // Encoder count
        int max_count;                                              // Max encoder count
        int resolution;                                             // Add/subtract by 1,10,100,1000
        Encoder (uint8_t A_pin, uint8_t B_pin, uint8_t Button_pin); // Format for instantiating a debouncer object
        int update_spin (void);                                     // Function for updating count
        bool update_press (void);                                   // Function for updating press
};                                                                  

/** @brief   Defines the class for a virtual encoder.
 *  @details This virtual encoder can be used through the serial monitor.
 *           The user can press 1 to turn the 'knob' to the right, 
 *           press 2 turn the 'knob' to the left, and press
 *           0 to press the button. This lets us test the screen interface
 *           on a breadboard, away from the encoder when its not accessible,
 *           or hasn't been soldered onto the main board yet.
 */
class virtualEncoder {
    protected:     
        int32_t parseIntWithEcho (Stream &stream);                              // Function to process serial input                                                             // No protected attributes as of now
    public:                                                                     //
        int count;                                                              // Encoder count storage
        bool pressed;                                                           // If the encoder was pressed or not
        int max_count;                                                          // Max count   
        int resolution;                                                         // Resolution, 1, 10, 100, or 1000
        virtualEncoder (bool init);                                             // Function to declare the object
        void getInput (void);                                                   // Function to receive user input
        
};       

/** @brief   Defines the class for an Encoder.
 *  @details This class has several protected attributes, which are mostly used in calculating
 *           the speed of the motor. When instantiating an Encoder object, the user must specify
 *           which hardware pins to use. The user also has the ability to change the frequency
 *           at which the motor speed is updated to the user. The user must also specify the 
 *           resolution of their encoder, i.e. how many ticks per revolution it will trigger.
 */
class motorEncoder {
    protected:                             
        uint8_t A_pin;                                              // Encoder signal A pin
        uint8_t B_pin;                                              // Encoder signal B pin
        int count;                                                  // Encoder count
        uint32_t timestamp_previous_interrupt;                      // Time stamp at last interrupt
        volatile float speed_RPM;                                   // Float for measured speed
        int count_previous_interrupt;                               // Value of count at last interrupt
        bool direction;                                             // 0 -> Forward, 1 -> Reverse
        uint8_t counts_until_update;                                // Counting up to next speed calculation
    public:
        int motorSpeed;
        uint8_t counts_per_rev;                                     // How many encoder counts per revolution
        uint8_t update_frequency;                                   // How frequently we want to update
        motorEncoder (uint8_t A_pin, uint8_t B_pin);                // Format for instantiating a debouncer object
        void update (uint32_t timestamp, bool dir);                 // Function format for getting the signal status
};

#endif // ENCODER_H