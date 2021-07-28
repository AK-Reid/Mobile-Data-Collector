//Including gfx & touchscreen libraries
#include <Adafruit_GFX.h> 
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

//Defining the display
MCUFRIEND_kbv tft;

//Pressure threshholds 
#define MINPRESSURE 200
#define MAXPRESSURE 1000

// I2C_Anything from : http://www.gammon.com.au/i2c
//Important library, this handles the I2C data transfer between the Uno and Due. Written by Nick Gammon
//You need to link the "I2C_Anything.h" file to this file, by having it in the same Arduino project folder, and opening it as an attached tab in the IDE. 
#include "I2C_Anything.h"
const int slaveAddress = 9;

//This particular display has an uncommon pin layout, if it's not exactly as follows the touch functionality will fail.  
const int XP = 8, XM = A2, YP = A3, YM = 9; //ID=0x9341

//Results of the screen calibration, produced by the touchscreen calibrator code within the mcufriend library. 
const int TS_LEFT = 95, TS_RT = 914, TS_TOP = 77, TS_BOT = 892;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//Defining some of the simplier buttons wusing the Adafruit_gfx button library.
Adafruit_GFX_Button menu_btn, back_btn, confirm_btn, cancel_btn, ok_btn;

//Global variables and constants 
float lux;

int currentPage; 
int buttonNumber;
int buttonState;
int thickness = 4;
int curvature = 12;

unsigned long timeStamp = 0;
unsigned long currentTime = 0;
const long period = 500;

unsigned long timerStart = 0;
const long printInterval = 1000;

float volts;
float tst;
float lat;
float lon;
float alt;
String latValueString;
float average = 0;

int pixel_x, pixel_y;     //Global vars for Touch_getXY(), for the Adafruit_gfx buttons. 
bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        pixel_x = map(p.x, TS_LEFT, TS_RT, 0, 240); //Defining the parameters of the tft with the screen calibration results. 
        pixel_y = map(p.y, TS_TOP, TS_BOT, 0, 320);
    }
    return pressed;
}

//Defining some colors, most were not used, colors with the "COLOR_" prefix were buggy for an unknown reason, having banded horizontal lines when appllied as a button color for example.
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define COLOR_NAVY       0x000F
#define COLOR_DARK_GREEN 0x03E0
#define COLOR_DARK_CYAN  0x03EF
#define COLOR_OLIVE      0x7BE0
#define COLOR_LIGHT_GREY 0xC618
#define COLOR_DARK_GREY  0x7BEF
#define COLOR_MAROON     0x7800
#define COLOR_GEISHA     0x0BC8
#define COLOR_HOT_PINK   0xF81F

//Interval and time tracker for the average lux display
unsigned long previousMillis = 0;   
const long interval = 15000;  

//Interval and time tracker for the timing of the constant_gps_and_lux_data_loop
unsigned long previousLoopTime = 0;
const long loopTimingInterval = 1000;


//Initial setup
void setup(void)
{
    Serial.begin(9600);   //Opens serial port to begin the serial for communication between the boards, rate set to 9600 bps
    Wire.begin();  //Begin the wire for communication between the two boards via I2C
    uint16_t ID = tft.readID();
    tft.begin(ID); //Starting up the touchscreen display, without this it will be blank 

    homeScreen(); //Running the function for the home screen
    
}

void homeScreen(){

    currentPage = 0; //Current page global var updated when the function is executed, to prevent buttons bleeding through and triggering on all pages simultaneously. 

    tft.setRotation(0);     //Portrait
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(2);     
    tft.setCursor(100, 0);


    //Dividing lines
    tft.drawLine(5,40,300,40,0x0000ff);     
    tft.drawLine(5,100,300,100,0x0000ff);   
    tft.drawLine(5,140,300,140,0x0000ff);  
    tft.drawLine(5,190,300,190,0x0000ff); 

    //Constant text items
    tft.setCursor (90,10);
    tft.setTextSize (4);
    tft.setTextColor (WHITE,BLACK);
    tft.println ("Lux"); 

    tft.setCursor (90,10);
    tft.setTextSize (4);
    tft.setTextColor (WHITE,BLACK);
    tft.println ("Lux"); 

    tft.setCursor (15,110);
    tft.setTextSize (2);
    tft.setTextColor (WHITE,BLACK);
    tft.println ("Avg Lux Value(15s)"); 

    //Initializing the menu button
    menu_btn.initButton(&tft, 205, 300, 60, 30, WHITE, CYAN, BLACK, "MENU", 2);
    menu_btn.drawButton();

}

void buttonMenu(){

    currentPage = 1;  //Current page global var updated when the function is executed, to prevent buttons bleeding through and triggering on all pages simultaneously. 

    tft.setRotation(0);     //Portrait
    tft.fillScreen(BLACK);
    tft.setTextSize(2);
    tft.setTextColor(WHITE, BLACK);
    tft.setCursor(100, 0);

    back_btn.initButton(&tft, 25, 300, 40, 30, WHITE, CYAN, BLACK, "<-", 3); //Initilization and drawing for the back button to return to the home screen
    back_btn.drawButton();

    //Drawing of the main buttons for the button menu
    tft.drawRoundRect(10, 13, 220, 40, 18, WHITE);
    tft.setCursor (25,25);
    tft.setTextColor (WHITE,BLACK);
    tft.setTextSize (2);
    tft.print("Interesting Tree");

    tft.drawRoundRect(10, 68, 220, 40, 18, WHITE);
    tft.setCursor (25,80);
    tft.setTextColor (WHITE,BLACK);
    tft.setTextSize (2);
    tft.print("Geology");

    tft.drawRoundRect(10, 123, 220, 40, 18, WHITE);
    tft.setCursor (25,135);
    tft.setTextSize(2);
    tft.print("Coffee in Need");

    tft.drawRoundRect(10, 178, 220, 40, 18, WHITE);
    tft.setCursor (25,190);
    tft.setTextColor (WHITE,BLACK);
    tft.setTextSize (2);
    tft.print("Animal Sign");

    tft.drawRoundRect(10, 233, 220, 40, 18, WHITE);
    tft.setCursor (25,245);
    tft.setTextColor (WHITE,BLACK);
    tft.setTextSize (2);
    tft.print("Other");
}


void confirmation_popup(){

    currentPage = 2; //Current page global var updated when the function is executed, to prevent buttons bleeding through and triggering on all pages simultaneously. 

    /*
    Length and width are changed from a fixed origin. Not uniformly on each end from the center. The 2xi modifier adjust the length and width from the origin, which creates the
    thicker effect on the right hand side, which needs to be counteracted by adding i to position to shift it, for a total of 1 pixel spaced concentric rounded rectangles 
    In a nutshell this draws a sequence of concentric boundaries and stacks them up because just one is too thin. 
    */

    for (int i = 0; i < thickness; i++) {
        tft.drawRoundRect(10 + i, 11 + i, 220 - i*2, 303 - i*2, curvature, COLOR_NAVY);
    }
    
    tft.fillRoundRect(10+(thickness), 11+(thickness), 220-(thickness*2), 303-(thickness*2), curvature, WHITE);

    //Breaking out the constant text items that appear in the popup, and the specifics are added in if statements in the loop
    tft.setCursor(20,25);
    tft.setTextColor (BLACK, WHITE);
    tft.setTextSize (2);
    tft.println("Are you sure you");
    tft.setCursor(20,40);
    tft.println("want to write the");
    tft.setCursor(20,55);
    tft.println("GPS and lux data");

    //Initializing and drawing of the confirm and cancel buttons
    confirm_btn.initButton(&tft, 65, 287, 90, 37, COLOR_HOT_PINK, GREEN, BLACK, "Confirm", 2);
    confirm_btn.drawButton();

    cancel_btn.initButton(&tft, 175, 287, 90, 37, COLOR_HOT_PINK, CYAN, BLACK, "Cancel", 2);
    cancel_btn.drawButton();

}

void OK_popup() {

    currentPage = 3; //Current page global var updated when the function is executed, to prevent buttons bleeding through and triggering on all pages simultaneously. 

    /*
    Length and width are changed from a fixed origin. Not uniformly on each end from the center. The 2xi modifier adjust the length and width from the origin, which creates the
    thicker effect on the right hand side, which needs to be counteracted by adding i to position to shift it, for a total of 1 pixel spaced concentric rounded rectangles 
    In a nutshell this draws a sequence of concentric boundaries and stacks them up because just one is too thin. 
    */

    for (int i = 0; i < thickness; i++) {
        tft.drawRoundRect(10 + i, 70 + i, 220 - i*2, 175 - i*2, curvature, COLOR_NAVY);
    }
    
    tft.fillRoundRect(10+(thickness), 70+(thickness), 220-(thickness*2), 175-(thickness*2), curvature, WHITE);

    ok_btn.initButton(&tft, 120, 220, 175, 25, COLOR_HOT_PINK, CYAN, BLACK, "Ok", 2);
    ok_btn.drawButton();

}

void constant_gps_and_lux_data_loop() {  
    /*
    Loop needs to be set on a timer, currently it executes once per second. Otherwise the serial read on the RPI will
    encounter many errors, mostly failed to read character, or outright failure, writing either "Fail" or "0.00" to file. I'm not presently sure why.
    But the solution is to slow it down, once a second seems to work excellently. I would guess the serial port on the RPI is just overloaded with data if it's not slowed down.  
    */
    unsigned long loopTimer = millis();
    if (loopTimer - previousLoopTime >= loopTimingInterval) {
      
      previousLoopTime = loopTimer;
      int reqLength = (sizeof volts) + (sizeof lat) + (sizeof lon) + (sizeof alt); //Keeping track of the size of everything being transfered, so we can make sure it's all there, and not send gibberish.
      int n = Wire.requestFrom(slaveAddress, reqLength); //Requesting bytes from the slave device
    
      if( n == reqLength ) {         // Checks if the number of bytes received was that what was requested

        I2C_readAnything( volts); //Using Nick Gammon I2C library to read the GPS data coming off the RPI so we can display it
        I2C_readAnything( lat);
        I2C_readAnything( lon);
        I2C_readAnything( alt);
        
        //Converting the electrical data from the light sensor into a lux value
        float amps = volts / 10000.0; 
        float microamps = amps * 1000000;
        lux = microamps * 2.0;
        
        Serial.println(lux); //Constantly printing the lux value to the serial so it can be read by the RPI and printed to the .csv file constantly. 
        
        if (currentPage == 0){    
            //Setting up constant text items to display the GPS data from the RPI
            tft.setTextSize (3);
            tft.setCursor (80,60);
            tft.setTextColor (GREEN,BLACK);
            tft.println(lux);
    
            tft.setTextSize (2);
            tft.setTextColor (WHITE,BLACK);
        
            tft.setCursor (5,210);
            tft.println ("lat = ");
            tft.setCursor (78,210);   
            tft.print(lat, 10); 
        
            
            tft.setCursor (5,250);
            tft.println ("lon = ");
            tft.setCursor (78,250);
            //Set to 9, because due to particular geographic location lon has two digits in the integer-part.
            tft.print(lon, 9); 
            
            tft.setCursor (5,290);
            tft.println ("alt = ");
            tft.setCursor (78,290);
            tft.print(alt); 
        }
    
    
      //Calculating the average lux value    
       for (int i=0; i < 40; i++) {
          average = average + lux;
        }
        average = average/40;
        
     //Printing the average lux value to the display every 15 seconds    
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
          if (currentPage == 0) {
            tft.setTextSize (3);
            tft.setCursor (90,160);
            tft.setTextColor (GREEN,BLACK);
            tft.println(average); 
          }
        }
    
        if (Serial.available() > 0) { //Check if there's data to be read, this is data that's already stored in the serial buffer which holds 64 bytes.  
          String gpsString = "";
          String data[3]; //Defining the number of items in the data, which in this case is three, for lat, lon, and alt. 
    
          
          while (Serial.available()) {  
            //Pause to allow the byte to arrive in input buffer without jambling 
            delay(2);
            //Read a single character from the buffer at a time
            char ch = Serial.read();
            //Append that single character to the gpsString 
            gpsString+= ch;
          }
    
        //Initialization of some tracker variables to keep track of the string location and the data index. 
          int stringStart = 0;
          int gpsDataIndex = 0;
          
          for (int i=0; i < gpsString.length(); i++) {
              //Look for seperator comma appended to the end of rasperry pi GPS string data
              if(gpsString.charAt(i) == ',') {
                  //Flush previous values from array to prevent random character noise and error.
                  data[gpsDataIndex] = "";
                  //Save substring into array for each lat, lon, alt. 
                  data[gpsDataIndex] = gpsString.substring(stringStart, i);
                  //Setup new string starting point until the next comma is found.
                  stringStart = (i+1);
                  gpsDataIndex++;
              }
          }
          
        } 
          
        
      }
      
      else {
        Serial.println(F( "Fail")); //If something goes wrong, it prints a fail to the serial output
      }
      
    }
}

void loop(void) {
    //Critically important loop execution
    constant_gps_and_lux_data_loop();
    
    //Setting up the "self location" and press functionality of the Adafruit_gfx buttons 
    TSPoint p = ts.getPoint();
    bool down = Touch_getXY();
    back_btn.press(down && back_btn.contains(pixel_x, pixel_y));
    menu_btn.press(down && menu_btn.contains(pixel_x, pixel_y));
    confirm_btn.press(down && confirm_btn.contains(pixel_x, pixel_y));
    cancel_btn.press(down && cancel_btn.contains(pixel_x, pixel_y));
    ok_btn.press(down && ok_btn.contains(pixel_x, pixel_y));
    
    if (currentPage == 0) { //Check if the page is correct before triggering a button
        if (p.z > 10 && p.z < 1000) { //Checks if the pressure applied to the screen is within range
            if (p.x > 668 && p.x < 854 && p.y > 778 && p.y < 867  && MINPRESSURE && p.z < MAXPRESSURE) { //Seems redundant, but touch input can be finicky, and having another pressure check reduces the chance of a mispress.
                menu_btn.drawButton(true);
                buttonMenu();
            }
        }
    }

/*
For the following large, main buttons, they are going to be done "manually" in-stead of using the Adafruit_gfx buttons press functionality. This is because the Adafruit_gfx buttons are
limited in their customizability and more specifically, didn't work well with the text I wanted to display on them. 
*/

    //Interesting Tree Button
    if (currentPage == 1) {
        if (p.z > 10 && p.z < 1000) {
            if (p.x > 155 && p.x < 840 && p.y > 155 && p.y < 236  && MINPRESSURE && p.z < MAXPRESSURE) {
                buttonNumber = 0;
                tft.drawRoundRect(10, 13, 220, 40, 18, MAGENTA);
                delay(200);
                tft.drawRoundRect(10, 13, 220, 40, 18, WHITE);
                confirmation_popup();

                tft.setCursor(20,70);
                tft.println("of this cool");
                tft.setCursor(20,85);
                tft.println("tree to the .csv?");
            }
        }
    }                  
 
    //Interesting Geology Button, e.g boulders, cliff faces, canyon formations
    if (currentPage == 1) {
        if (p.z > 10 && p.z < 1000) {
            if (p.x > 155 && p.x < 840 && p.y > 277 && p.y < 370  && MINPRESSURE && p.z < MAXPRESSURE) {
                buttonNumber = 1;
                tft.drawRoundRect(10, 68, 220, 40, 18, MAGENTA);
                delay(200);
                tft.drawRoundRect(10, 68, 220, 40, 18, WHITE);
                confirmation_popup();

                tft.setCursor(20,70);
                tft.println("of this geologic");
                tft.setCursor(19,85);
                tft.println("formation to the");
                tft.setCursor(19, 100);
                tft.println(".csv?");
            }
        }
    }    

    //Coffee Tree In Need Button, e.g it's sick and needs fertilizer
    if (currentPage == 1) {
        if (p.z > 10 && p.z < 1000) {
            if (p.x > 155 && p.x < 840 && p.y > 410 && p.y < 498  && MINPRESSURE && p.z < MAXPRESSURE) {
                buttonNumber = 2;
                tft.drawRoundRect(10, 123, 220, 40, 18, MAGENTA);
                delay(200);
                tft.drawRoundRect(10, 123, 220, 40, 18, WHITE);
                confirmation_popup();

                tft.setCursor(20,70);
                tft.println("of this coffee");
                tft.setCursor(19,85);
                tft.println("tree that needs");
                tft.setCursor(19, 100);
                tft.println("attending to, ");
                tft.setCursor(19, 115);
                tft.println("to the .csv?");
            }
        }
    }    

    //Animal Sign Button
    if (currentPage == 1) {
        if (p.z > 10 && p.z < 1000) {
            if (p.x > 155 && p.x < 840 && p.y > 542 && p.y < 630  && MINPRESSURE && p.z < MAXPRESSURE) {
                buttonNumber = 3;
                tft.drawRoundRect(10, 178, 220, 40, 18, MAGENTA);
                delay(200);
                tft.drawRoundRect(10, 178, 220, 40, 18, WHITE);
                confirmation_popup();

                tft.setCursor(20,70);
                tft.println("of this animal");
                tft.setCursor(19,85);
                tft.println("sign to the .csv?");
                tft.setCursor(19, 100);
                tft.println("attending to, ");
                tft.setCursor(19, 115);
                tft.println("to the .csv?");

            }
        }
    }    

    //Other Button, for anything else of note
    if (currentPage == 1) {
        if (p.z > 10 && p.z < 1000) {
            if (p.x > 155 && p.x < 840 && p.y > 667 && p.y < 760  && MINPRESSURE && p.z < MAXPRESSURE) {
                buttonNumber = 4;
                currentTime = millis();
                tft.drawRoundRect(10, 233, 220, 40, 18, MAGENTA);
                delay(200);
                tft.drawRoundRect(10, 233, 220, 40, 18, WHITE);  
                confirmation_popup();              

                tft.setCursor(20,70);
                tft.println("of this other");
                tft.setCursor(19,85);
                tft.println("noteworthy locat-");
                tft.setCursor(19, 100);
                tft.println("ion to the .csv?");
                
                
            }
        }
    }


    if (currentPage == 2 && buttonNumber == 0) {
        if (confirm_btn.justPressed()) {
            confirm_btn.drawButton(true);

            //write current data to csv
            Serial.print(lux);
            Serial.print(",0");
            Serial.println();
        
            for (int i = 0; i < thickness; i++) {
                tft.drawRoundRect(10 + i, 11 + i, 220 - i*2, 303 - i*2 , curvature, COLOR_HOT_PINK);
                delay(150);
            }

            OK_popup();

            tft.setTextColor (BLACK);
            tft.setCursor(20,90);
            tft.println("GPS location and");
            tft.setCursor(20,105);
            tft.println("lux data of" );
            tft.setCursor(20,120);
            tft.println("tree successfully");
            tft.setCursor(20,135);
            tft.println("written to the");
            tft.setCursor(20,150);
            tft.println(".csv file");
            
        }    
    }

/*
Controlling what happens when a specific button is pressed, essentially when a button is pressed, it sends the current lux value to the RPI, along with a number that marks
which button was just pressed. In addition the border of the button turns pink just for asthetics, and on the butttons respective confirmation screens, it prints the 
appropriate text. 
*/

    if (currentPage == 2 && buttonNumber == 1) {
        if (confirm_btn.justPressed()) {
            confirm_btn.drawButton(true);

            //write current data to csv
            Serial.print(lux);
            Serial.print(",1");
            Serial.println();
        
            for (int i = 0; i < thickness; i++) {
                tft.drawRoundRect(10 + i, 11 + i, 220 - i*2, 303 - i*2 , curvature, COLOR_HOT_PINK);
                delay(150);
            }

            OK_popup();

            tft.setCursor(20,90);
            tft.println("GPS location and");
            tft.setCursor(20,105);
            tft.println("lux data of" );
            tft.setCursor(20,120);
            tft.println("geology formation");
            tft.setCursor(20,135);
            tft.println("successfully ");
            tft.setCursor(20,150);
            tft.println("written to the");
            tft.setCursor(20,165);
            tft.println(".csv file");
        }
    }
     
    if (currentPage == 2 && buttonNumber == 2) {
        if (confirm_btn.justPressed()) {
            confirm_btn.drawButton(true);

            //write current data to csv
            Serial.print(lux);
            Serial.print(",2");
            Serial.println();
        
            for (int i = 0; i < thickness; i++) {
                tft.drawRoundRect(10 + i, 11 + i, 220 - i*2, 303 - i*2 , curvature, COLOR_HOT_PINK);
                delay(150);
            }

            OK_popup();

            tft.setCursor(20,90);
            tft.println("GPS location and");
            tft.setCursor(20,105);
            tft.println("lux data of" );
            tft.setCursor(20,120);
            tft.println("coffee tree");
            tft.setCursor(20,135);
            tft.println("successfully ");
            tft.setCursor(20,150);
            tft.println("written to the");
            tft.setCursor(20,165);
            tft.println(".csv file");
        }
    }

    if (currentPage == 2 && buttonNumber == 3) {
        if (confirm_btn.justPressed()) {
            confirm_btn.drawButton(true);

            //write current data to csv
            Serial.print(lux);
            Serial.print(",3");
            Serial.println();
        
            for (int i = 0; i < thickness; i++) {
                tft.drawRoundRect(10 + i, 11 + i, 220 - i*2, 303 - i*2 , curvature, COLOR_HOT_PINK);
                delay(150);
            }

            OK_popup();

            tft.setCursor(20,90);
            tft.println("GPS location and");
            tft.setCursor(20,105);
            tft.println("lux data of" );
            tft.setCursor(20,120);
            tft.println("animal sign");
            tft.setCursor(20,135);
            tft.println("successfully ");
            tft.setCursor(20,150);
            tft.println("written to the");
            tft.setCursor(20,165);
            tft.println(".csv file");
        }
    }
    
    if (currentPage == 2 && buttonNumber == 4) {
        if (confirm_btn.justPressed()) {
            confirm_btn.drawButton(true);

            //write current data to csv
            Serial.print(lux);
            Serial.print(",4");
            Serial.println();
        
            for (int i = 0; i < thickness; i++) {
                tft.drawRoundRect(10 + i, 11 + i, 220 - i*2, 303 - i*2 , curvature, COLOR_HOT_PINK);
                delay(150);
            }

            OK_popup();

            tft.setCursor(20,90);
            tft.println("GPS location and");
            tft.setCursor(20,105);
            tft.println("lux data of");
            tft.setCursor(20,120);
            tft.println("other noteworthy");
            tft.setCursor(20,135);
            tft.println("spot successfully");
            tft.setCursor(20,150);
            tft.println( "written to the");
            tft.setCursor(20,165);
            tft.println(".csv file");
        }
    }              
    
   //Back to home page button
    if (currentPage == 1) {
        if (back_btn.justPressed()) {
            back_btn.drawButton(true);
            homeScreen();
            delay(100);

        }
        
    }

   //Confirmation popup "cancel" button, no need for any "ifs" like the confirm button, since it alwats does the same thing.  
    if (currentPage == 2) {
        if (cancel_btn.justPressed()) {
            cancel_btn.drawButton(true);
            delay(200);
        }

        if (cancel_btn.justReleased()) {
            cancel_btn.drawButton();
            buttonMenu();
        }

    } 

   //The "ok" button after the data has been sent to the RPI   
    if (currentPage == 3) {
        if (ok_btn.justPressed()) {
            ok_btn.drawButton(true);
            delay(100);
        }    

        if (ok_btn.justReleased()) {
            ok_btn.drawButton();
            delay(300);
            buttonMenu();
        }
    }

   
} //End of loop
