/** @file encoder.cpp
 *    This file contains the implementation of the motor driver class and task.
 *    It also contains the class functions for the encoder, and the task to 
 *    obtain user input.
 *
 *  @author Jordan Kochavi
 *  @author Jenny Verheul
 *  @author Chandler Jones
 *  @date 2020-Nov-10
 */

#include "encoder.h"

/** @brief   Function that creates an encoder object.
 *  @details This function creates an encoder object at specified hardware pins.
 *           The hardware pins for the encoder must be passed as arguments, @c A, @c B, and @c Press
 */
Encoder::Encoder (uint8_t A, uint8_t B, uint8_t Press)                      // Function format for instantiating real encoder object
{                                                                           // 
    A_pin = A; B_pin = B; Button_pin = Press;                               // Initialize hardware pin attributes with parameters
    count = 0.0;                                                            // Initialize to 0
    resolution = 1;                                                         // Initialize to 1
    max_count = 1;                                                          // Initialize to 1
    current_reading = 0;                                                    // Initialize to 0
    previous_reading = 0;                                                   // Initialize to 0
    pressed = false;                                                        // Initialize to false
}

/** @brief   Function that debounces the hardware pin for the encoder button.
 *  @details The encoder can be sensitive to presses, especially if the user is also 
 *           twising and torquing it. This function debounces the signal that the encoder's 
 *           button is connected to to make sure that it is actually being pressed.
 */
bool Encoder::debounce (uint8_t signal_pin, uint8_t limit)
{
    uint8_t debounce_count = 0;             // Start a counter for the debouncer
    for (uint8_t k = 0; k < limit; k++)     // From k = 0 to k = limit
    {                                       //
        if (digitalRead(signal_pin))        // If the signal reading is high...
        {                                   //
            debounce_count += 1;            //      Then add one to the counter
        }                                   //
    }                                       //
    if (debounce_count > limit-2)           // If the debouncer records enough counts...
    {                                       //  
        return true;                        //      Then, return true
    }                                       //  
    else                                    // Otherwise...
    {                                       //       
        return false;                       //      Return false
    }
}

/** @brief   Function that interprets the A and B encoder signals as rotation.
 *  @details This function reads the A and B encoder pins and calculates whether
 *           it is being spun in the clockwise are counterclockwise direction.
 *           The calculations below are inspired by the "grey code" method used 
 *           on Sparkfun's website for the COM-09117 encoder knob. 
 *           URL: https://www.sparkfun.com/products/9117
 *           Download the file hyperlinked as "Example Arduino Project"
 *           The example file uses binary numbers and bit shifting.
 */
int Encoder::update_spin (void)
{
    current_reading = digitalRead(B_pin)*2 + digitalRead(A_pin);
    uint8_t sum = previous_reading*4 + current_reading;
    if((sum == 13 || sum == 4 || sum == 2 || sum == 11) && count < max_count)
    {
        count += resolution; 
    }
    if((sum == 14 || sum == 7 || sum == 1 || sum == 8) && count >= resolution)
    {
        count -= resolution; 
    }
    previous_reading = current_reading; 
    return count;
}

/** @brief   Function that interprets the press of the encoder.
 *  @details When called in an ISR, this function debounces the press
 *           hardware pin, with a length of 20 (can be changed here).
 *           If the signal is debounced, then it returns true.
 */
bool Encoder::update_press (void)
{
    if (debounce(Button_pin, 50))   // If signal debounced...
    {                               //
        pressed = true;             //      Then, return true
    }                               //
    else                            // Else...
    {                               //
        pressed = false;            //      Return false
    }                               // 
    return pressed;                 // Return the press status
}

/** @brief   Function for constructing a virtual encoder object.
 *  @details This function creates a virtual encoder object. This class is helpful
 *           when testing the screen on its own without a real encoder. The virtual encoder
 *           works through the serial monitor and receives input from the user using 
 *           @c parseIntWithEcho(). 
 */
virtualEncoder::virtualEncoder (bool init)
{
    count = 0;
    resolution = 1;
    max_count = 1;
    pressed = false;
}

/** @brief   Read an integer from a serial device, echoing input and blocking.
 *  @details This function reads an integer which is typed by a user into a
 *           serial device. It uses the Arduino function @c readBytes(), which
 *           blocks the task which calls this function until a character is
 *           read. When any character is received, it is echoed through the
 *           serial port so the user can see what was typed. Only decimal
 *           integers are supported; negative integers beginning with a single
 *           @c - sign or positive ones with a @c + will work. 
 * 
 *           @b NOTE: The serial device must have its timeout set to a very
 *           long time, or this function will malfunction. A recommended call:
 *           @code
 *           Serial.setTimeout (0xFFFFFFFF);
 *           @endcode
 *           Assuming that the serial port named @c Serial is being used.
 *  @param   stream The serial device such as @c Serial used to communicate
 */
int32_t virtualEncoder::parseIntWithEcho (Stream& stream)
{
    const uint8_t MAX_INT_DIGITS = 24;        // More than a 64-bit integer has
    char ch_in = 0;                           // One character from the buffer
    char in_buf[MAX_INT_DIGITS];              // Character buffer for input
    uint8_t count = 0;                        // Counts characters received

    // Read until return is received or too many characters have been read.
    // The readBytes function blocks while waiting for characters
    while (true)
    {
        stream.readBytes (&ch_in, 1);         // Read (and wait for) characters
        in_buf[count++] = ch_in;
        stream.print (ch_in);                 // Echo the character
        if (ch_in == '\b' && count)           // If a backspace, back up one
        {                                     // character and try again
            count -= 2;
        }
        if (ch_in == '\n' || count >= (MAX_INT_DIGITS - 1))
        {
            in_buf[count] = '\0';             // String must have a \0 at end
            return atoi (in_buf);
        }
    }
}

/** @brief   Function that receives input from the serial monitor.
 *  @details This function calls @c parseIntWithEcho() and updates
 *           the virtual encoder count. It displays instructions to 
 *           the user for what they should press. 
 */
void virtualEncoder::getInput ()
{
    Serial << "Enter 0 to press the button"                                 // Display instructions
           << endl
           << "Enter 1 to twist to the right"
           << endl
           << "Enter 2 to twist to the left" 
           << endl;
    int32_t userInput = parseIntWithEcho(Serial);                           // Wait to receive input from the user
    Serial << userInput << endl;
    if (userInput == 0)                                                     // If a 0 is entered...
    {                                                                       //
        pressed = true;                                                     //      Then, the encoder was pressed
        Serial << "Press!" << endl;                                         //      Tell the serial monitor
    }                                                                       //
    else if (userInput == 1 && count < max_count)                           // Else if 1 is entered and we're not over the max count...
    {                                                                       //
        count += resolution;                                                //      Then increase the count by the resolution
        Serial << "Encoder count: " << count << "        " << endl;         //      Tell the serial monitor current count   
    }                                                                       //
    else if (userInput == 2 && count >= resolution)                         // Else if a 2 is entered and we're not over the min count...
    {                                                                       //
        count -= resolution;                                                //      Then subtract the count by the resolution  
        Serial << "Encoder count: " << count << "        " << endl;         //      Tell the serial monitor current count
    }                                                                       //
}

/** @brief   Function called to instantiate an encoder object.
 *  @details This function requires two parameters to instantiate an encoder object.
 *           The first parameter is the hardware pin to read signal A, and the second
 *           parameter is the hardware pin to read signal B. 
 */
motorEncoder::motorEncoder (uint8_t A_GPIO, uint8_t B_GPIO)
{
    A_pin = A_GPIO;                             // Save the parameter, which will evaporate when the constructor exits
    B_pin = B_GPIO;                             // Save the parameter, which will evaporate when the constructor exits
    direction = 0;                              // Initialize direction to forward
    speed_RPM = 0;                              // Initialize speed to 0
    timestamp_previous_interrupt = 0;           // Initialize to 0
    count_previous_interrupt = 0;               // Initialize to 0
    count = 0;                                  // Initialize to 0
    counts_until_update = 0;                    // Initialize to 0
    count_previous_interrupt = 0;               // Initialize to 0
}

/** @brief   This function calculates the speed of the motor.
 *  @details After the direction of the motor is determined in the ISR,
 *           this function calculates the motor's speed. It keeps track of the 
 *           absolute position of the motor, in encoder ticks, with a protected attribute
 *           called "count". Each encoder tick in the forward direction adds 1 to count,
 *           and each encoder tick in the reverse direction subtracts 1 from count.
 *           Although this function is called every interrupt, it does not calculate
 *           the motor speed every interrupt. Rather, it calculates the motor speed every
 *           update_frequency interrupts. To do this, this function calculates the number of encoder
 *           counts between calculations, and the elapsed time between calculations. This uses protected
 *           attributes: timestamp_previous_interrupt, and count_previous_interrupt. The speed,
 *           in RPM, is calculated using the equation below. Units are noted in "[ ]"
 *      
 *           Speed [RPM] = (delta_count/delta_time)[counts/microsecond]*1000000[microseconds/second]
 *                         * 60 [seconds/min] / (counts_per_rev) [counts/revolution]
 */          
void motorEncoder::update (uint32_t timestamp, bool dir)
{
    direction = dir;                                                                    // Store the direction in its protected attribute
    if (direction)                                                                      // If motor is spinning in reverse...
    {                                                                                   //
        count --;                                                                       //      Then, subtract 1 from the encoder position
    }                                                                                   //
    else if (!direction)                                                                // Else if the motor is spinning forward...
    {                                                                                   //
        count ++;                                                                       //      Then, add 1 to the encoder position
    }                                                                                   //
    counts_until_update ++;                                                             // Increase the number of ticks between calculations
    if (counts_until_update > update_frequency)                                         // If it's time to calculate the motor speed...
    {                                                                                   //      Then, calculate it:
        if (timestamp_previous_interrupt < timestamp)                                   //      If micros() returned a not-overflowed value...
        {                                                                               //              Then, implement the equation:
            float time_between_interrupts = timestamp - timestamp_previous_interrupt;   //              Units of microseconds
            int counts_between_interrupts = abs(count - count_previous_interrupt);      //              Units of counts
            speed_RPM = counts_between_interrupts/time_between_interrupts;              //              Calculate speed in counts per microsecond
            speed_RPM *= 1000000*60/counts_per_rev;                                     //              Calculate speed in RPM
            motorSpeed = (int)speed_RPM;                                                //              Set the speed global variable for printing
        }                                                                               //
        timestamp_previous_interrupt = timestamp;                                       // Set a new "old" timestamp for next calculation
        count_previous_interrupt = count;                                               // Set a new "old" count value for next calculation
        counts_until_update = 0;                                                        // Begin counting up again to next time to calculate
    }
}