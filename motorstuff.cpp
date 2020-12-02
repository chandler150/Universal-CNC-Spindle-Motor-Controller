/** @file motorstuff.cpp
 *    This file contains the implementation of the motor driver class and task.
 *    It also contains the class functions for the encoder, and the task to 
 *    obtain user input.
 *
 *  @author Jordan Kochavi
 *  @author Jenny Verheul
 *  @author Chandler Jones
 *  @date 2020-Nov-10
 */

#include "userInterface.h"                                              // Include user interface files
#include "motorstuff.h"                                                 // Include corresponding header file
#include "taskshare.h"                                                  // Include task sharing library
#include "taskqueue.h"                                                  // Include taskqueue library

#define motorEncoderPinA 7
#define motorEncoderPinB 8
#define motorPWMpin      A3
#define motorDIRpin      2

Queue <int> actualMotorSpeed (30,"Buffer");                             // Create Queue to store current speed calculations
extern Share <int> speed_SP;                                            // Point to Queue created by user interface tasks
extern Share <int> maxMotorSpeed;
motorEncoder myMotorEncoder(motorEncoderPinA, motorEncoderPinB);

/** @brief   Function called to instantiate a MotorDriver object.
 *  @details This function requires two parameters to instantiate a MotorDriver object.
 *           The first parameter is the hardware pin for the PWM signal output.
 *           The second parameter is the hardware pin for the direction.
 *           For ease of access, these parameters are included as #defines at the top of this file.
 */
MotorDriver::MotorDriver (uint8_t PWM_GPIO, uint8_t direction_GPIO)
{  
    PWM_pin = PWM_GPIO;                       // Save the parameter, which will evaporate when the constructor exits
    direction_pin = direction_GPIO;           // Save the parameter, which will evaporate when the constructor exits
    pinMode(direction_pin, OUTPUT);           // Configure the direction pin for output.
}

/** @brief   Function file to output a duty cycle and direction to the motor driver.
 *  @details This function outputs the user-specified PWM duty cycle and direction
 *           to the motor using analogWrite() and digitalWrite(). The direction pin is 
 *           essentially a boolean. Writing a 0 will drive the motor in the forward
 *           direction, and writing a 1 will drive the motor in reverse. However,
 *           this function takes the direction as an int23_t instead of a boolean
 *           because function, parseIntWithEcho() returns user-input numbers as
 *           int32_t. 
 */
void MotorDriver::run (int32_t DUTYCYCLE, int32_t DIRECTION)
{
    analogWrite(PWM_pin, DUTYCYCLE);            // Output the PWM signal to the PWM output pin
    if (DIRECTION)                              // If the direction is 0...
    {                                           // 
        digitalWrite(direction_pin, HIGH);      //      Then, output a logic 1 to the direction output pin
    }                                           //
    else                                        // Else...
    {                                           //
        digitalWrite(direction_pin, LOW);       //      Output a logiuc 0 to the direction output pin
    }                                           //
}

/** @brief   An interrupt service routine for calculating the speed of the motor.
 *  @details This ISR is triggered by the rising edge of one of the encoder signals.
 *           Each encoder signal outputs a square wave, and each square wave is 90 degrees out of phase.
 *           We determine the direction that the motor is spinning by checking which wave is leading.
 *           For example, let's say the motor is spinning clockwise. In clockwise rotation, signal A
 *           leads signal B. The instant A toggles HIGH on its rising edge, this ISR is run. 
 *           Since A leads B, if we were to read signal B at precisely the instant that A toggles HIGH,
 *           then B should be low, because B toggles HIGH after A does in clockwise rotation.
 *           Therefore, if we read signal B using digitalRead(), then a returned value of 0 would mean that
 *           the motor is in clockwise (forward) rotation, and if it returns a 1, the motor is in counterclockwise
 *           rotation (reverse). After determining the direction of the motor, the ISR updates the encoder to calculate
 *           its speed. The timestamp at the current interrupt is also necessary for this calculation. The Arduino function,
 *           micros() returns the number of microseconds elapsed since the program started. This value is stored in an unsigned
 *           32-bit integer, which means it has the capacity to overflow. In this case, it resets to 0, and we would not want
 *           to calculate the speed of the motor in this case. 
 */
void motorISR ()
{    
    uint32_t current_time_stamp = micros();            // Get the current time stamp in microseconds
    bool direction = digitalRead(motorEncoderPinB);           // Determine the motor's direction by checking signal B
    myMotorEncoder.update(current_time_stamp, direction);   // Update the encoder to calculate the speed of the motor
    actualMotorSpeed.put(myMotorEncoder.motorSpeed);
}

/** @brief   Task which interacts with a user. 
 *  @details This task demonstrates how to use a FreeRTOS task for interacting
 *           with some user while other more important things are going on.
 *  @param   p_params A pointer to function parameters which we don't use.
 */
void task_MotorStuff (void* p_params)
{
    (void)p_params;                                                             // Does nothing but shut up a compiler warning
    // Initialise the xLastWakeTime variable with the current time.
    // It will be used to run the task at precise intervals
    TickType_t xLastWakeTime = xTaskGetTickCount();  
    // Set the timeout for reading from the serial port to the maximum
    // possible value, essentially forever for a real-time control program
    attachInterrupt(digitalPinToInterrupt(motorEncoderPinA), motorISR, RISING); // Attach interrupt for change of A
    int currentSpeedSP;
    int maxSpeed;

    maxMotorSpeed.get(maxSpeed);
    MotorDriver myMotorDriver(motorPWMpin, motorDIRpin);                        // Instantiate MotorDriver object with desired pins
    myMotorDriver.run(0,0);
    Serial.setTimeout (0xFFFFFFFF);
    // The task's infinite loop goes here
    for (;;)
    {
        speed_SP.get(currentSpeedSP);
        currentSpeedSP = currentSpeedSP*255/325;
        myMotorDriver.run(currentSpeedSP,1);

        // This type of delay waits until the given number of RTOS ticks have
        // elapsed since the task previously began running. This prevents 
        // inaccuracy due to not accounting for how long the task took to run
        vTaskDelayUntil (&xLastWakeTime, update_period);
    }
}