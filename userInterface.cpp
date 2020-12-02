/** @file userInterface.cpp
 *    This file contains the implementation of the user interface for the router controller.
 *    It also contains the class functions for the encoder, and the task to update the user interface.
 *
 *  @author Jordan Kochavi
 *  @author Jenny Verheul
 *  @author Chandler Jones
 *  @date 2020-Nov-10
 */

#include "userInterface.h"                                              // Include the motor driver header file created for this lab
#include "encoder.h"                                                    // Include encoder library
#include "taskshare.h"                                                  // Include task sharing library
#include "Wire.h"                                                       // Include I2C connection library
#include "Adafruit_GFX.h"                                               // Include Adafruit general graphics library
#include "Adafruit_SSD1306.h"                                           // Include Adafruit_SSD1306 library
#include "FreeMono9pt7b.h"                                              // Include custom font
#include "taskqueue.h"                                                  // Include taskqueue library
#define Encoder_press 11                                                // Define press hardware pin on the encoder
#define Encoder_A     3                                                 // Define the hardware pins used for the encoder 
#define Encoder_B     4                                                 // On all Nucleo and Arduino dev boards, digital pins 2 & 3 support hardware interrupts
Encoder myEncoder (Encoder_A, Encoder_B, Encoder_press);                // Instantiate encoder object with desired pins (class created for this lab)
//virtualEncoder myVirtualEncoder (0);  // Instantiate virtual encoder object

Share <int> speed_SP;                                                   // Create share for storing speed set point
Share <int> maxMotorSpeed;                                              // Create share for storing maxMotorSpeed
String RES_TEXT;                                                        // Create global variable for resolution text
bool motorEncoderRun = false;                                           // Global flag to run motor encoder ISR

extern Queue <int> actualMotorSpeed;                                    // Points to Queue created by motor control tasks
/** @brief   ISR that triggers when the encoder is spun.
 *  @details This ISR updates the encoder's internal count. Count
 *           is also stored as a global variable.
 */
void A_pin_ISR ()
{   
    int count = myEncoder.update_spin();                              // Update the encoder 
    //Serial << "Encoder count: " << count << "        " << endl;       // Display to the serial monitor
}

/** @brief   ISR that triggers when the encoder is pressed.
 *  @details This ISR updates the encoder's press status. 
 */
void press_ISR ()
{
    if (myEncoder.update_press())                               // If the encoder has been pressed...
    {                                                           //
        //Serial << endl << "Press!" << endl;                     //      Then, tell the serial monitor
    }
}

/** @brief   Function to construct screenButton object.  
 *  @details This function instantiates a pressable button on the screen. There are two types of
 *           buttons: regular ones and extended ones. They are both rounded and rectangular,
 *           but the regular button type is used for 3 or 4 letter labels, such as, "View",
 *           or "Set". The extended ones display a label and a number, so they are wider. 
 *           The parameter @c label is the text label that is printed on the button. The 
 *           paramters @c X and @c Y are the X and Y coordinates on the screen where the button
 *           will be displayed. Lastly, a @c ButtonType can be specified. It is set to REGULAR
 *           by default, but passing EXTENDED will create an extended button. REGULAR and 
 *           EXTENDED are just defines that are translated to 1 or 0 by the compiler. 
 */
screenButton::screenButton(String label, uint8_t X, uint8_t Y, uint8_t ButtonType=REGULAR)
{
    refresh = false;
    text = label;                       // Set the button's text attribute to the passed label
    x_coord = X;                        // Set the X coordinate 
    y_coord = Y;                        // Set the Y coordinate
    if (ButtonType == EXTENDED)         // If an extended button type has been declared, then...
    {                                   //
        width = 105;                    //      Define the pixel width of extended button
        height = 15;                    //      Define the pixel height of extended button
        rect_rad = 3;                   //      Radius of rounded edges on button
        buttonType = 0;                 //      Set the button type 0 -> Extended
    }                                   //
    else if (ButtonType == REGULAR)     // Else if a regular button type has been declared, then...
    {                                   //
        buttonType = 1;                 //      Set the button type 1 -> Regular
        rect_rad = 5;                   //      Radius of rounded edges on button
        width = 50;                     //      Define the pixel width of regular button
        height = 15;                    //      Define the pixel height of regular button
    }               
}

/** @brief   A screenButton function that displays the button at a display object.
 *  @details This function displays a screenButton object in a particular mode.
 *           There are three possible display modes: REGULAR, PRESSED, or ERASE.
 *           This mode is defined with the parameter @c action. By default, it is set
 *           to regular, but will accept PRESSED, or ERASE here. PRESSED and ERASE are
 *           translated to 1 and 2 by the compiler. On the screen, a REGULAR type 
 *           is just the button label with nothing around it. When pressed, the button
 *           inverts its colors, so the text is now black, surrounded by a filled white 
 *           rectangle. An ERASE action sets all colors to black, so it appears that it 
 *           had been erased from the screen.
 *  @param   display The display object that the buttons are tied to.
 *  @param   action  How we want the button to be displayed: regular, pressed, or erased.
 *                   You can specify this action by entering 0,1,2, or you can literally 
 *                   type in "PRESSED" or "ERASE". The default action is regular. 
 *                   0 -> Regular, 1 -> Pressed, 2 -> Erase
 */
void screenButton::displayRegular(Adafruit_SSD1306* display, uint8_t action=0)
{
    bool textColor = WHITE; bool fillColor = BLACK;     // Set the default text color to WHITE and fill color to BLACK
    if (action == 0)                                    // If a REGULAR display type is passed...
    {                                                   //  
        textColor = WHITE; fillColor = BLACK;           //      Then, set the text color to WHITE and fill color to BLACK
    }                                                   // 
    else if (action == PRESSED)                         // Else if a PRESSED display type is passed...       
    {                                                   //  
        textColor = BLACK; fillColor = WHITE;           //      Then, set the text color to BLACK and fill color to WHITE
    }                                                   //
    else if (action == ERASE)                           // Else if an ERASE display type is passed...
    {                                                   //
        textColor = BLACK; fillColor = BLACK;           //      Then, set both the text color and fill color to BLACK
    }                                                   //
    display->setFont(&FreeMono9pt7b);                   // Set the display font to our chosen font and size
    display->setTextColor(textColor);                   // Set the text color
    if (buttonType == EXTENDED)                                                             // If the button type is extended...
    {                                                                                       //
        display->fillRoundRect(x_coord-2,y_coord-height+3,width,height,rect_rad,fillColor); //      Then, draw a filled rectangle
        display->drawRoundRect(x_coord-2,y_coord-height+3,width,height,rect_rad,fillColor); //      Draw a round rectangle
        display->setCursor(x_coord,y_coord);                                                //      Set the screen cursor to X and Y 
        display->println(text);                                                             //      Print the label on the screen
    }                                                                                       //
    else if (buttonType == REGULAR)                                                         // Else if the button type is regular...
    {                                                                                       //
        display->fillRoundRect(x_coord,y_coord,width,height,rect_rad,fillColor);            //      Then, draw a slightly different filled rectangle
        display->drawRoundRect(x_coord,y_coord,width,height,rect_rad,fillColor);            //      Draw a slightly different round rectangle
        display->setCursor(x_coord+5,y_coord+11);                                           //      Set the cursor slightly differently
        display->println(text);                                                             //      Print the label on the screen
    }                                                                                       //
    display->display();                                                                     // Equivalent to hitting the "write" command...
}

/** @brief   Function that displays a white outline over a button.
 *  @details This function displays a white outline over a screnButton object, indicating
 *           that it can be pressed. This allows the user to toggle through different 
 *           options and choices on the screen, and select a desired option with the encoder.
 */
void screenButton::displayHover(Adafruit_SSD1306* display)                                  //
{                                                                                           //
    display->setFont(&FreeMono9pt7b);                                                       // Set display font to custom font
    if (buttonType == REGULAR)                                                              // If the button type is regular...
    {                                                                                       //
        display->fillRoundRect(x_coord,y_coord,width,height,rect_rad,BLACK);                //      Then, draw a slightly different filled rectangle
        display->drawRoundRect(x_coord,y_coord,width,height,rect_rad,WHITE);                //      Then, draw a round, white rectangle 
        display->setCursor(x_coord+5,y_coord+11);                                           //      Set the cursor slightly differently
        display->println(text);                                                             //      Print the label on the screen
    }                                                                                       //
    else if (buttonType == EXTENDED)                                                        // Else if the button type is extended...
    {                                                                                       //      
        display->fillRoundRect(x_coord-2,y_coord-height+3,width,height,rect_rad,BLACK);     //      Then, draw a filled rectangle
        display->setCursor(x_coord,y_coord);                                                //      Set the screen cursor to X and Y 
        display->println(text);                                                             //      Print the label on the screen
        display->drawRoundRect(x_coord-2,y_coord-height+3,width,height,rect_rad,WHITE);     //      Then, draw a slightly different rectangle
    }                                                                                       //  
    display->display();                                                                     // Equivalent to hitting the "write" command...
}

/** @brief   Function that updates the appearance of a button.
 *  @details This function updates the appearance of a screen button based on its state.
 *           There are four possible display states that a button could be in: unpressed,
 *           pressed, off, or hovered. This function is called for every button whenever
 *           the screen is refreshed. To maximize the speed of the screen, it only makes 
 *           sense to update the appearance of a button whenever it is on the screen. 
 *           First, it checks if its flag @c refresh has been raised. If it hasn't,
 *           then the button does not need to be updated.
 *  @param   display The display object that the button is tied to.
 */
void screenButton::update(Adafruit_SSD1306* display)
{
    if (refresh)                              // If refresh has been raised...
    {                                         // 
        if (state == UNPRESSED)               // If the button should be unpressed...  
        {                                     //    
            displayRegular(display);          // Display the button as unpressed     
        }                                     //
        else if (state == PRESSED)            // Else if the button should be pressed...
        {                                     //
            displayRegular(display,PRESSED);  // Display the button as pressed
        }                                     //
        else if (state == OFF)                // Else if the button should be off...   
        {                                     //
            displayRegular(display,ERASE);    // Then display the button as off 
        }                                     // 
        else if (state == HOVER)              // Else if the button should be hovered over...
        {                                     //
            displayHover(display);            // Then display the button as hovered
        }                                     //
        refresh = false;                      // Lower the button's refresh flag
    }
}

/** @brief   Function that constructs an interface object.
 *  @details This function creates an instance of our interface and sets default values.
 */
routerInterface::routerInterface(bool init)         
{                                                                                           
    selected = false;                                                                       // Default to false
    SET = new screenButton("Set",0,0);                                                      // Create SET button    
    VIEW = new screenButton("View",78,0);                                                   // Create VIEW button   
    RES = new screenButton("RES.....1",2,35,EXTENDED);                                      // Create RES button   
    SPEED = new screenButton("RPM:" + String(0),2,60,EXTENDED);                             // Create SPEED button 
    MES = new screenButton("RPM:",2,35,EXTENDED);                                           // Create MES button
    display = new Adafruit_SSD1306(128,64);                                                 // Create new display object
    static_disp_done = false;                                                               // Default to false
    page_state = 0;                                                                         // Default to zero
    button_state = 0;                                                                       // Default to zero
    SET->state = HOVER;      SET->refresh = true;                                           // Default to hover
    VIEW->state = UNPRESSED; VIEW->refresh = true;                                          // Default to unpressed
    display->begin(SSD1306_SWITCHCAPVCC, 0x3C);                                             // Init display at I2C address
    display->clearDisplay();                                                                // Clear display
    display->setTextColor(WHITE);                                                           // Default to white
    display->setRotation(0);                                                                // Default orientation
    display->setTextWrap(false);                                                            // Turn text wrapping off
    // In setting a screen dimension of 0 and text size of 0, the screen's own documentation will tell it that we are printing
    // custom fonts and shapes. We use custom fonts because it allows us to make full use out of the OLED's resolution. The screen's
    // default size and dimension values are hard to read and are very pixelated at small sizes. 
    display->dim(0);
    display->setTextSize(0);
    display->display();                                                           
    // In setting a screen dimension of 0 and text size of 0, the screen's own documentation will tell it that we are printing
    // custom fonts and shapes. We use custom fonts because it allows us to make full use out of the OLED's resolution. The screen's
    // default size and dimension values are hard to read and are very pixelated at small sizes. 
    Serial << "Display Initialized..." << endl;                                             // Tell serial monitor that display has been initialized
}

/** @brief   Function that is called in precise intervals by the user interface task
 *           to refresh the display.
 *  @details This function updates the display using a state machine. There are 5 possible
 *           display states. The user can either be: choosing whether to adjust the resolution or speed,
 *           viewing the current measured speed, adjusting the resolution, adjusting the speed, or neutral.
 *           First, this function updates all button objects. Not every button is displayed on the screen at
 *           any given time, so many of these update() calls will return 0; only the buttons that are not "off",
 *           are refreshed. Next, the function checks if the encoder has been pressed. Because the user can
 *           press the encoder to provide input to the interface, the screen needs to refresh accordingly based
 *           on when the encoder was pressed. Handling the pressing of the encoder is nearly a state machine in itself.
 *           Finally, the function runs the code that corresponds to our display state. The state variable is stored in 
 *           the variable @c button_state . 
 *  @param   encoder The encoder object that we're using.
 */
void routerInterface::refresh(Encoder &encoder)
{     
    SET->update(display);                                      // Refresh the appearance of the SET button if needed
    VIEW->update(display);                                     // Refresh the appearance of the VIEW button if needed
    RES->update(display);                                      // Refresh the appearance of the RES button if needed
    SPEED->update(display);                                    // Refresh the appearance of the SPEED button if needed
    MES->update(display);                                      // Refresh the appearance of the MES button if needed
    if (encoder.pressed)             {managePress(encoder);}   // If the encoder is pressed, then run the code that deals with that
    else                                                       // Otherwise... proceed to update the display as we regularly do...
    {                                                          //   
        if      (button_state == 1)  {manageSet(encoder);  }   //      If state = 1... then run the SET state code
        else if (button_state == 2)  {manageView(encoder); }   //      Else if state = 2... then run the VIEW state code
        else if (button_state == 3)  {manageRes(encoder);  }   //      Else if state = 3... then run the RES state code
        else if (button_state == 4)  {manageSpeed(encoder);}   //      Else if state = 4... then run the SPEED state code
        else                         {manageSpin(encoder); }   //      Else... then run the neutral state code
    }
}

/** @brief   Function that updates the display based on when the encoder was pressed.
 *  @details Whenever the encoder is pressed, it raises a boolean flag telling the interface that 
 *           it was pressed, and an action needs to be taken. The interface lowers this flag after
 *           it has been dealt with. The response of the interface to a button press depends on 
 *           what display state we are currently in. This function checks which state we're in,
 *           and then updates the objects currently on-display accordingly, based on the press.
 *  @param   encoder The encoder object that we're using.
 */
void routerInterface::managePress(Encoder &encoder)     
{
    if (!button_state)                                                      // If we're in the neutral state...
    {                                                                       //
        if (SET->state == HOVER)                                            // If the SET button was pressed...
        {                                                                   //
            SET->state = PRESSED; SET->refresh = true;                      // Update SET as pressed
            RES->state = UNPRESSED; RES->refresh = true;                    // Update RES as unpressed
            SPEED->state = UNPRESSED; SPEED->refresh = true;                // Update SPEED as unpressed
            button_state = 1;                                               // Switch to button state 1
            encoder.max_count = 3*encoder.resolution;                       // Set the max count to 3x resolution
        }                                                                   //
        else if (VIEW->state == HOVER)                                      // If the VIEW button was pressed...
        {                                                                   //
            VIEW->state = PRESSED; VIEW->refresh = true;                    // Update VIEW as pressed
            int currentSP; speed_SP.get(currentSP);                         // Retrieve the current speed set point from Share
            SPEED->text = "SP:"+String(currentSP);                          // Update the speed text with current speed sp
            SPEED->state = UNPRESSED; SPEED->refresh = true;                // Update state appearance as unpressed
            MES->state = UNPRESSED; MES->refresh = true;                    // Update MESS appearance as unpressed
            motorEncoderRun = true;
            button_state = 2;                                               // Switch to button state 2
        }                                                                   //
    }                                                                       //
    else if (button_state == 1)                                             // Else if we're in the SET state...
    {                                                                       //
        if (VIEW->state == HOVER)                                           // If the VIEW button was pressed...
        {                                                                   // 
            RES->state = OFF; RES->refresh = true;                          // Update RES appearance as off
            SPEED->state = OFF; SPEED->refresh = true;                      // Update SPEED appearance as off
            SET->state = UNPRESSED; SET->refresh = true;                    // Update SET appearance as unpressed
            button_state = 0;                                               // Return to neutral state
            encoder.max_count = encoder.resolution;                         // Set the max encoder count to 1x resolution
        }                                                                   //
        else if (RES->state == HOVER)                                       // If the RES button was pressed...
        {                                                                   //
            RES->state = PRESSED; RES->refresh = true;                      // Then update the apearance of the button as pressed
            button_state = 3;                                               // Switch to button state 3
            // Whenever the user wants to adjust the resolution, it would be annoying to start their options at 1 every time.
            // In other words, if the user previously selected a resolution of 1000, it would be annoying and confusing if a 1
            // showed up the next time they went to adjust the resolution. Therefore, we need to restore the encoder count 
            // appropriately so the user can pick up where they left off in the toggling process. Recall that the potential options
            // for the resolution are 1, 10, 100, and 1000. If the resolution is 1, then toggling from 1-1000 will have the encoder
            // count increment from 0,1,2,3. 0 = log(1), 1 = log(10), 2 = log(100), and 3 = log(1000). So by taking the log value 
            // of the current resolution value will put the encoder count in the right place. But we also need to then multiply it
            // by the current resolution. Lets say that the user previously set the resolution to 100. When they go to set the 
            // the resolution next time, we want the encoder count to be 200, because 100 is the 3rd possible option (0, 100, 200).
            // 200 = log(100)*100 = 2*100 = 200. 
            encoder.count = log10(encoder.resolution)*encoder.resolution;   // Set the encoder count based on the log of the resolution
            //Serial << "Resolution: " << encoder.resolution << endl;         // Display to the serial for debugging
            //Serial << "Encoder Count: " << encoder.count << endl;           // Display to the serial for debugging
            encoder.max_count = 3*encoder.resolution;                       // Set the max encoder count to 3x resolution
        }                                                                   //
        else if (SPEED->state == HOVER)                                     // If the SPEED button was pressed...
        {                                                                   //
            SPEED->state = PRESSED; SPEED->refresh = true;                  // Update SPEED appearance as pressed
            button_state = 4;                                               // Switch to button state 4
            maxMotorSpeed.get(encoder.max_count);                           // Store the maxMotorSpeed as the maximum count
            speed_SP.get(encoder.count);                                    // Store the current set point as the current count
        }                                                                   //
        else                                                                // Otherwise...
        {                                                                   //  
            RES->state = OFF; RES->refresh = true;                          // Turn RES appearance off
            SPEED->state = OFF; SPEED->refresh = true;                      // Turn SPEED appearance off
            SET->state = UNPRESSED; SET->refresh = true;                    // Turn SET appearance off
            button_state = 0;                                               // Return to neutral state
            encoder.max_count = encoder.resolution;                         // Set max count to 1x resolution
        }                                                                   //
    }                                                                       //
    else if (button_state == 2)                                             // Else if we're in the VIEW state...
    {                                                                       // 
        if (SET->state == HOVER)                                            // If the SET button was pressed...
        {                                                                   //
            VIEW->state = UNPRESSED; VIEW->refresh = true;                  // Update the VIEW appearance as unpressed 
        }                                                                   //
        else                                                                // Otherwise...
        {                                                                   //
            VIEW->state = HOVER; VIEW->refresh = true;                      // Update VIEW appearance as hovered over
            SPEED->state = OFF; SPEED->refresh = true;                      // Update SPEED appearance as off
            MES->state = OFF; MES->refresh = true;                          // Update MES appearance as off
        }                                                                   // 
        button_state = 0;                                                   // Return to neutral state
        encoder.max_count = encoder.resolution;                             // Set max count to 1x resolution
    }                                                                       //
    else if (button_state == 3)                                             // Else if we're in the RES state
    {                                                                       // 
        RES->state = HOVER; RES->refresh = true;                            // Update the RES appearance as hovered over
        button_state = 1;                                                   // Switch to button state 1
        encoder.resolution = pow(10,encoder.count/encoder.resolution);      // Set the encoder resolution based on current count
        encoder.count = 2*(int)encoder.resolution;                          // Set count to 2x resolution
        encoder.max_count = 3*(int)encoder.resolution;                      // Set max count to 3x resolution
        //Serial << "Resolution: " << encoder.resolution << endl;             // Print to serial for debugging
        //Serial << "Encoder Count: " << encoder.count << endl;               // Print to serial for debugging
    }                                                                       //
    else if (button_state == 4)                                             // Else if we're in the SPEED state...
    {                                                                       //
        SPEED->state = HOVER; SPEED->refresh = true;                        // Update SPEED appearance to hovered over
        button_state = 1;                                                   // Switch to state 1
        speed_SP.put(encoder.count);                                          // Update the speed setpoint with current encoder count
        //Serial << "Speed SP: " << encoder.count << endl;                    // Print to serial for debugging
        encoder.count = 3*encoder.resolution;                               // Set count to 3x resolution
        encoder.max_count = 3*encoder.resolution;                           // Set max count to 3x resolution
    }                                                                       //
    //Serial << "Button State: " << button_state << endl;                     // Print to serial for debugging
    encoder.pressed = false;                                                // Lower the encoder pressed flag
}

/** @brief   State of the user interface FSM when nothing has been selected yet.
 *  @details When nothing has been selected, the user can only choose between two
 *           options that are displayed on the screen, SET and VIEW. When nothing is
 *           selected, the maximum encoder count value is constrained to 1x resolution, 
 *           so the encoder count can either be 0, or the resolution. This state
 *           updates both buttons with which one is being hovered over while the user
 *           uses the encoder to toggle through the two options.
 *  @param   encoder The encoder object that we're using.
 */
void routerInterface::manageSpin(Encoder &encoder)
{
    if ((int)encoder.count == 0)                        // If encoder count = 0, then...
    {                                                   //
        SET->state = HOVER;      SET->refresh = true;   //      Set the SET button as hovered over
        VIEW->state = UNPRESSED; VIEW->refresh = true;  //      Set the VIEW button as unpressed
    }                                                   //
    else if ((int)encoder.count == encoder.resolution)  // Else if encoder count = 1x encoder resolution, then...
    {                                                   //
        SET->state = UNPRESSED;  SET->refresh = true;   //      Set the SET button as unpressed
        VIEW->state = HOVER;     VIEW->refresh = true;  //      Set the VIEW button as hovered over
    }
}

/** @brief   State of the user interface FSM when the resolution is 
 *           being adjusted.
 *  @details This state does not update the actual resolution of the encoder. 
 *           Rather, it updates the text on the screen with a resolution option.
 *           The user toggles through four possible encoder resolutions: 1, 10, 100, 1000, 
 *           using the encoder, and the resolution is updated upon deselecting the RES button.
 *  @param   encoder The encoder object that we're using.
 */
void routerInterface::manageRes(Encoder &encoder)
{
    if      ((int)encoder.count == 0)                     {RES->text = "RES.....1";} // If count is 0,                 res = 1
    else if ((int)encoder.count == encoder.resolution)    {RES->text = "RES....10";} // Else if count = 1x resolution, res = 10
    else if ((int)encoder.count == 2*encoder.resolution)  {RES->text = "RES...100";} // Else if count = 2x resolution, res = 100
    else if ((int)encoder.count == 3*encoder.resolution)  {RES->text = "RES..1000";} // Else if count = 3x resolution, res = 1000
    RES->refresh = true;
}

/** @brief   State of the user interface FSM after the user has pressed the
 *           "Set" button. 
 *  @details When the user selects the "Set" button, they now have several options.
 *           They can either press the Set or View button again to unselect it, or they
 *           can select the "Res" or "Speed" button to change the resolution of the encoder
 *           or change the speed set point. Because we're using the same encoder to toggle 
 *           through options and to adjust the speed, we need to make this this code more general.
 *           Deciding which button is currently being hovered/selected is determined by multiples
 *           of the encoder's resolution. This lets us store the encoder resolution and not worry
 *           about it being erased. As an example, the user sets the encoder resolution to 10, 
 *           meaning that they want to adjust the speed set point in increments of 10. When 
 *           they return to this state, as they twist the encoder to select another option 
 *           on the screen, the encoder's count will continue to increase or decrease by 10. 
 *           In this state, the SET button will always be pressed. Therefore, its appearance
 *           on the display will never need to be updated, as it is static.
 *  @param   encoder The encoder object that we're using.
 */
void routerInterface::manageSet(Encoder &encoder)   
{
    if ((int)encoder.count == 0)                                // If the encoder count is 0, then...
    {                                                           //
        VIEW->state = UNPRESSED;  VIEW->refresh = true;         //      Set the VIEW button as unpressed
        RES->state = UNPRESSED;   RES->refresh = true;          //      Set the RES button as unpressed
        SPEED->state = UNPRESSED; SPEED->refresh = true;        //      Set the SPEED button as unpressed
    }                                                           //
    else if ((int)encoder.count == (int)encoder.resolution)     // Else if the encoder count = 1x encoder resolution...
    {                                                           //
        VIEW->state = HOVER;      VIEW->refresh = true;         //      Set the VIEW button as hovered over
        RES->state = UNPRESSED;   RES->refresh = true;          //      Set the RES button as unpressed
        SPEED->state = UNPRESSED; SPEED->refresh = true;        //      Set the SPEED button as unpressed
    }                                                           //
    else if ((int)encoder.count == 2*(int)encoder.resolution)   // Else if the encoder count = 2x encoder resolution...
    {                                                           //
        VIEW->state = UNPRESSED;  VIEW->refresh = true;         //      Set the VIEW button as unpressed
        RES->state = HOVER;       RES->refresh = true;          //      Set the RES button as hovered over
        SPEED->state = UNPRESSED; SPEED->refresh = true;        //      Set the SPEED button as unpressed
    }                                                           //
    else if ((int)encoder.count == 3*(int)encoder.resolution)   // Else if the encoder count = 3x encoder resolution...
    {                                                           //
        VIEW->state = UNPRESSED;  VIEW->refresh = true;         //      Set the VIEW button as unpressed
        RES->state = UNPRESSED;   RES->refresh = true;          //      Set the RES button as unpressed
        SPEED->state = HOVER;     SPEED->refresh = true;        //      Set the SPEED button as hovered over
    }
}

/** @brief   State of the user interface FSM when the user is viewing the 
 *           current measured RPM.
 *  @details The user interface enters this state when the user selects
 *           "View" on the display. Here, the current measured RPM of the motor
 *           is displayed, along with the current set point. To display the current
 *           speed, it retreives the current speed from the queue, and updates the 
 *           text attribute of the MES button object with the current speed. 
 *           Since the speed set point will never change in this state, it does not 
 *           need to be updated here, as it is static. Finally, the state raises the 
 *           button's "refresh" flag, indicating that it needs to be updated on the screen.
 *  @param   encoder The encoder object that we're using.
 */
void routerInterface::manageView(Encoder &encoder)
{
    int currentSpeed;                               // Create local variable for current speed
    actualMotorSpeed.get(currentSpeed);             // Pull a value from the queue and store in the local variable
    MES->text = "RPM:"+String(currentSpeed);        // Update the text attribute of the button 
    MES->refresh = true;                            // Raise the refresh flag
}

/** @brief   State of the user interface FSM when the speed set point is 
 *           being adjusted.
 *  @details This function updates the text that displays the speed set 
 *           point. Note that it doesn't actually update the speed set point,
 *           it only updates the text on the screen. It does this by updating
 *           the text attribute of the SPEED button object, and then raising its
 *           "refresh" flag, indicating that the button needs to be updated on 
 *           the screen.
 *  @param   encoder The encoder object that we're using.
 */
void routerInterface::manageSpeed(Encoder &encoder)
{
    SPEED->text = "RPM:"+String((int)encoder.count); // Change the text attribute of the button
    SPEED->refresh = true;                           // Raise the refresh flag
}

/** @brief   Task which interacts with a user. 
 *  @details This task demonstrates how to use a FreeRTOS task for interacting
 *           with some user while other more important things are going on.
 *  @param   p_params A pointer to function parameters which we don't use.
 */
void task_UI (void* p_params)
{
    (void)p_params;                                                             // Does nothing but shut up a compiler warning
    pinMode(Encoder_A, INPUT_PULLUP);                                           // Configure A_pin for input
    pinMode(Encoder_B, INPUT_PULLUP);                                           // Configure B_pin for input
    pinMode(Encoder_press, INPUT_PULLUP);                                       // Configure the press pin for input
    attachInterrupt(digitalPinToInterrupt(Encoder_A), A_pin_ISR, CHANGE);       // Attach interrupt for change of A
    attachInterrupt(digitalPinToInterrupt(Encoder_press), press_ISR, CHANGE);   // Attach interrupt for change of press pin
    routerInterface myInterface(0);                                             // Create new interface object
    myEncoder.count = 0;                                                        // Initialize to 0
    int maxSpeed = 325;                                                         // Set max motor RPM to 30000, default
    maxMotorSpeed.put(maxSpeed);                                                // Store in shared task variable   
    speed_SP.put(0);                                                            // Set default speed set point to 0, default
    // Initialise the xLastWakeTime variable with the current time.
    // It will be used to run the task at precise intervals
    TickType_t xLastWakeTime = xTaskGetTickCount();  
    // Set the timeout for reading from the serial port to the maximum
    // possible value, essentially forever for a real-time control program
    Serial.setTimeout (0xFFFFFFFF);
    // The task's infinite loop goes here
    for (;;)
    {
        myInterface.refresh(myEncoder);                                         // Refresh the interface
        //myVirtualEncoder.getInput();                                            // Receive input from the virtual encoder
        // This type of delay waits until the given number of RTOS ticks have
        // elapsed since the task previously began running. This prevents 
        // inaccuracy due to not accounting for how long the task took to run
        vTaskDelayUntil (&xLastWakeTime, update_period);
    }
}