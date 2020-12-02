/** @file motorstuff.h
 *    blarg
 *  @author Jordan Kochavi
 *  @date 2020-Nov-10
 */

#ifndef MOTOR_H
#define MOTOR_H
#include <Arduino.h>
#include <PrintStream.h>
#if (defined STM32L4xx || defined STM32F4xx)
    #include <STM32FreeRTOS.h>
#endif

/// Class Definitions
/** @brief   Defines the class for the MotorDriver.
 *  @details This class has 2 protected attributes and two public functions.
 */
class MotorDriver {
    protected:
        /// PWM Pin
        uint8_t PWM_pin;
        /// Direction pin
        uint8_t direction_pin;
    public:
        MotorDriver (uint8_t PWM_GPIO, uint8_t direction_GPIO);     // Format for instantiating a motor driver object
        void run (int32_t DUTYCYCLE, int32_t DIRECTION);            // Function format for getting the signal status
};

/// Task functions
void task_MotorStuff (void* params);                                        // The user interface task function

#endif // MOTOR_H