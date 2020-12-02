/** @file main.cpp
 *    This file contains a program that controls the speed of a DC motor, 
 *    and measures its speed with an encoder. It outputs the speed of the motor
 *    to the serial monitor.
 *
 *  @author  Jordan Kochavi
 *  @author  Jenny Verheul
 *  @author  Chandler Jones
 * 
 *  @date    28 Oct 2020 Original file
 */

#include <Arduino.h>                          // Include Arduino library
#include <PrintStream.h>                      // Include PrintStream library
#if (defined STM32L4xx || defined STM32F4xx)  //
    #include <STM32FreeRTOS.h>                // Include FreeRTOS library
#endif                                        //
#include "taskshare.h"                        // Include task sharing library
#include "encoder.h"                          // Include the encoder files
#include "userInterface.h"                    // Incldue the user interface files
#include "motorstuff.h"                       // Include the motor control files

/** @brief   Arduino setup function which runs once at program startup.
 *  @details This function sets up a serial port for communication and creates
 *           the tasks which will be run.
 */
void setup () 
{
    Wire.begin();
    // Start the serial port, wait a short time, then say hello. Use the
    // non-RTOS delay() function because the RTOS hasn't been started yet
    Serial.begin (115200);                        // Begin serial at baud rate of 115200 kHz
    delay (2000);                                 // Delay to allow time to open serial monitor
    Serial << endl << endl << "Starting Program..." << endl;
    xTaskCreate (task_UI,                         // Create task for user interface
                 "User Interface",                // Name for printouts
                 1536,                            // Stack size
                 NULL,                            // Parameters for task fn.
                 1,                               // Priority
                 NULL);                           // Task handle
    xTaskCreate (task_MotorStuff,                 // Create task for motor stuff
                 "User Interface",                // Name for printouts
                 1536,                            // Stack size
                 NULL,                            // Parameters for task fn.
                 2,                               // Priority
                 NULL);                           // Task handle
    // If using an STM32, we need to call the scheduler startup function now;
    // if using an ESP32, it has already been called for us
    #if (defined STM32L4xx || defined STM32F4xx)
        vTaskStartScheduler ();
    #endif
}

/** @brief   Arduino's low-priority loop function, which we don't use.
 *  @details A non-RTOS Arduino program runs all of its continuously running
 *           code in this function after @c setup() has finished. When using
 *           FreeRTOS, @c loop() implements a low priority task on most
 *           microcontrollers, and crashes on some others, so we'll not use it.
 */
void loop () 
{
}