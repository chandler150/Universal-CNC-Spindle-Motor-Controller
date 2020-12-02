/** @file userInterface.h
 *    blarg
 *  @author Jordan Kochavi
 *  @date 2020-Nov-10
 */

#ifndef UI_H
#define UI_H
#include "Adafruit_SSD1306.h"                  // Include Adafruit_SSD1306 library to drive the display
#include "encoder.h"                           // Include encoder library
#include <Arduino.h>                           // Include Arduino library  
#include <PrintStream.h>                       // Include PrintStream libary
#if (defined STM32L4xx || defined STM32F4xx)   // Include FreeRTOS
    #include <STM32FreeRTOS.h>  
#endif

// These #defines make it easier to call different functions. A lot of the time,
// it can be harder to remember which number corresponds to what action when passing
// function arguments, so redefining these as words makes it easier.
#define REGULAR   1                             // Define REGULAR as 1
#define EXTENDED  0                             // Define EXTENDED as 1
#define UNPRESSED 0                             // Define UNPRESSED as 1
#define PRESSED   1                             // Define PRESSED as 1
#define OFF       2                             // Define OFF as 2
#define ERASE     2                             // Define ERASE as 2
#define HOVER     3                             // Define HOVER as 3

/** @brief   Class definition for screen button.
 *  @details It would be too repetative and complicated to manage all screen coordinates, messages, and button formats
 *           within a single class or function. This way, we can create as many buttons and options as we wants, and 
 *           customize each one with different labels and sizes. This class encapsulates functions for displaying
 *           buttons in different states, like being hovered over or pressed/selected. 
 */
class screenButton {
    protected:                                                              
        uint8_t rect_rad;                                                       // Radius of rounded edges on button
        uint8_t width;                                                          // Overall width of button
        uint8_t height;                                                         // Overall height of button
        bool buttonType;                                                        // Button type (regular or extended)
public:                                                                     
        bool refresh;                                                           // Flag to refresh the button's appearance
        uint8_t state;                                                          // Stores what state of appearance the button is
        String text;                                                            // Button label to be printed on screen
        uint8_t x_coord;                                                        // X coordinate of button start
        uint8_t y_coord;                                                        // Y coordinate of button start
        screenButton(String label, uint8_t X, uint8_t Y, uint8_t ButtonType);   // Function format for creating button object
        void displayHover(Adafruit_SSD1306* display);                           // Function format for display a hover over the button
        void displayRegular(Adafruit_SSD1306* display, uint8_t action);         // Function format for displaying the button
        void update(Adafruit_SSD1306* display);                                 // Function formate to update the appearance of a button
};

/** @brief   Class definition for router interface.
 *  @details Although we need to create a display object, it is helpful to have
 *           a separate object to store attributes unique to this particular user interface.
 *           This includes display flags such as @c selected, @c static_disp_done, and @c setting speed.
 *           Having all of this encapsulated in a class allows us to implement multiple types of interfaces for 
 *           the same display object. Let's say we want one "front end" interface for changing the speed RPM 
 *           set point and reading the current measured sped, but we want a second, unique, "back end" interface
 *           for adjusting PID constants and debugging errors with the controller. Then we can make a second interface
 *           object and switch off between the two. This would also be useful if we wanted to incorporate another I2C screen.
 */
class routerInterface {
    protected:                                                              
        uint8_t page_state;         // Page state as an 8-bit unsigned integer
        uint8_t button_state;       // Button state as an 8-bit unsigned integer
        bool static_disp_done;      // Flag for initializing static display elements
        bool selected;              // Flag for storing if an option has been selected
        bool settingSpeed;          // Flag for halting other option selections if adjusting motor speed
        screenButton* SET;          // Create pointer for SET button
        screenButton* VIEW;         // Create pointer for VIEW button
        screenButton* RES;          // Create pointer for RES button
        screenButton* SPEED;        // Create pointer for SPEED button
        screenButton* MES;          // Create point for MES button
        Adafruit_SSD1306* display;  // Create pointer for display
    public:                                                                 
        int currentSP;
        routerInterface(bool init);          // Function format for constructing interface object
        void refresh(Encoder &encoder);      // Function format for refreshing the screen
        void managePress(Encoder &encoder);  // Function format for all press-related code  
        void manageSet(Encoder &encoder);    // Function format for SET state
        void manageView(Encoder &encoder);   // Function format for VIEW state
        void manageSpin(Encoder &encoder);   // Function format for neutral state
        void manageRes(Encoder &encoder);    // Function format for RES state
        void manageSpeed(Encoder &encoder);  // Function format for SPEED state
};

/// Task functions
void task_UI (void* params);                                                // The user interface task function
/// Other variables
const TickType_t update_period = 10;                                        // RTOS ticks (ms) per task run
#endif // UI_H