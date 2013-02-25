/*
 
 Zombie Dice for Arduino with Freetronics LCD & Keypad Shield:
  
  See: http://www.freetronics.com/products/lcd-keypad-shield
  
  Pins used by LCD & Keypad Shield:
  
    A0: Buttons, analog input from voltage ladder
    D4: LCD bit 4
    D5: LCD bit 5
    D6: LCD bit 6
    D7: LCD bit 7
    D8: LCD RS
    D9: LCD E
    D3: LCD Backlight (high = on, also has pullup high so default is on)
  
  ADC voltages for the 5 buttons on analog input pin A0:
  
    RIGHT:  0.00V :   0 @ 8bit ;   0 @ 10 bit
    UP:     0.71V :  36 @ 8bit ; 145 @ 10 bit
    DOWN:   1.61V :  82 @ 8bit ; 329 @ 10 bit
    LEFT:   2.47V : 126 @ 8bit ; 505 @ 10 bit
    SELECT: 3.62V : 185 @ 8bit ; 741 @ 10 bit
*/

/*--------------------------------------------------------------------------------------
  Includes
--------------------------------------------------------------------------------------*/
#include <LiquidCrystal.h>   // include LCD library

/*--------------------------------------------------------------------------------------
  Defines
--------------------------------------------------------------------------------------*/
// Pins in use
#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input
#define LCD_BACKLIGHT_PIN         3  // D3 controls LCD backlight
// ADC readings expected for the 5 buttons on the ADC input
#define RIGHT_10BIT_ADC           0  // right
#define UP_10BIT_ADC            145  // up
#define DOWN_10BIT_ADC          329  // down
#define LEFT_10BIT_ADC          505  // left
#define SELECT_10BIT_ADC        741  // right
#define BUTTONHYSTERESIS         10  // hysteresis for valid button sensing window
//return values for ReadButtons()
#define BUTTON_NONE               0  // 
#define BUTTON_RIGHT              1  // 
#define BUTTON_UP                 2  // 
#define BUTTON_DOWN               3  // 
#define BUTTON_LEFT               4  // 
#define BUTTON_SELECT             5  // 
// Some example macros with friendly labels for LCD backlight/pin control, tested and can be swapped into the example code as you like
#define LCD_BACKLIGHT_OFF()     digitalWrite( LCD_BACKLIGHT_PIN, LOW )
#define LCD_BACKLIGHT_ON()      digitalWrite( LCD_BACKLIGHT_PIN, HIGH )
#define LCD_BACKLIGHT(state)    { if( state ){digitalWrite( LCD_BACKLIGHT_PIN, HIGH );}else{digitalWrite( LCD_BACKLIGHT_PIN, LOW );} }

// For the game.
#define NUM_DICE                  13 // The number of dice that can be pulled from the cup.
#define GREEN_DICE                1  // A green die. 3 Brains, 2 Feed, 1 Shot.
#define YELLOW_DICE               2  // A yellow die. 2 Brains, 2 Feet, 2 Shot.
#define RED_DICE                  3  // A red die. 1 Brain, 2 Run, 3 Shot.

// Process
#define MODE_TITLE       0 // 1. The title screen
#define MODE_SCOREBOARD  1 // 2. The scoreboard.
#define MODE_INGAME      2 // 3. The main game loop.


/*--------------------------------------------------------------------------------
  Variables
--------------------------------------------------------------------------------------*/
byte buttonJustPressed  = false;         // this will be true after a ReadButtons() call if triggered.
byte buttonJustReleased = false;         // this will be true after a ReadButtons() call if triggered.
byte buttonWas          = BUTTON_NONE;   // used by ReadButtons() for detection of button events.

struct Player {
  int score;
};

int turn = 1;  // This keeps track of who's turn it is. Player 1 starts.
int mode = MODE_TITLE;

Player player1 = {0};
Player player2 = {0};

Player arr[2] = {player1, player2};

LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );   // Pins for the freetronics 16x2 LCD shield. LCD: ( RS, E, LCD-D4, LCD-D5, LCD-D6, LCD-D7 )

void setup()
{
   initScreen(); // Setup the freetronics LCD.
   titleScreen();
}

/*--------------------------------------------------------------------------------------
  loop()
  Arduino main loop
--------------------------------------------------------------------------------------*/
void loop()
{
   byte button;
   byte timestamp;
   
   button = ReadButtons();
   if(buttonJustPressed) {       
     switch(mode) {
     
      case MODE_TITLE: 
        titleScreen();
        break;
        
      case MODE_SCOREBOARD:
        drawScoreboard();
        break;

      case MODE_INGAME: 
        turnloop();
        break;
     }
  }
     
  if( buttonJustPressed )
     buttonJustPressed = false;
  if( buttonJustReleased )
     buttonJustReleased = false;
}

/**
 * Initialise the LCD screen.
 */
void initScreen() {
  // Button adc input.
  pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
  digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0
  // LCD backlight control.
  digitalWrite( LCD_BACKLIGHT_PIN, HIGH );  //backlight control pin D3 is high (on)
  pinMode( LCD_BACKLIGHT_PIN, OUTPUT );     //D3 is an output
  // Set up the LCD number of columns and rows: 
  lcd.begin( 16, 2 );
}

/**
 * Wipe everything from the screen.
 */
void clearScreen() {
  lcd.setCursor( 0, 0 );
  lcd.print( "                " );
  lcd.setCursor(0,1);
  lcd.print( "                " );
}  

/**
 * Display the title screen.
 */
void titleScreen() {
  clearScreen();
  lcd.setCursor(2,0);
  lcd.print("Zombie Dice!");
  lcd.setCursor(3,1);
  lcd.print("Press any key");
  mode = MODE_SCOREBOARD;
}

/**
 * Draw the scoreboard.
 */
void drawScoreboard() {
  clearScreen();
  lcd.setCursor(11,0);
  lcd.print("P1:");
  lcd.print(player1.score);
  lcd.setCursor(11,1);
  lcd.print("P2:");
  lcd.print(player2.score);
  mode = MODE_INGAME;
}

/**
 * Choose dice.
 */
void turnloop() {
  clearScreen();
  lcd.setCursor(0,0);
  lcd.print("Player ");
  lcd.print(turn);
  lcd.setCursor(0,1);
  lcd.print("Choose your dice!");
    
  byte button;
  byte timestamp;
  
  // Put the dice in the container.
  float green = 6.0;
  float yellow = 4.0;
  float red = 3.0;
  float total = green + yellow + red;
  
  while (1) {
   
    button = ReadButtons();
    if(buttonJustPressed) {  
      switch(button) {
        case BUTTON_SELECT:

          int die1 = chooseDie(total, green, yellow, red);
          total = total - 1;
          int die2 = chooseDie(total, green, yellow, red);
          total = total - 1;
          int die3 = chooseDie(total, green, yellow, red);
          total = total - 1;

          clearScreen();
          lcd.setCursor(0,0);
          lcd.print("You selected:");
          lcd.setCursor(0,1);
          lcd.print(die1);
          lcd.setCursor(4,1);
          lcd.print(die2);
          lcd.setCursor(8,1);
          lcd.print(die3);
 
          break;
      }  
    }  
  }  
}

int chooseDie(float total, float green, float yellow, float red) {

  float greenNums = green / total * 100;
  float yellowNums = greenNums + (yellow / total * 100);
  
  int die = rand() % 100;
  
  if (die < greenNums) {
    return GREEN_DICE;
  }
  else if (die > yellowNums) {
    
    return RED_DICE;
  }
  else {
    return YELLOW_DICE;
  } 
}

/**
 * ReadButtons().
 * Detect the button pressed and return the value.
 * Uses global values buttonWas, buttonJustPressed, buttonJustReleased.
 */
byte ReadButtons()
{
   unsigned int buttonVoltage;
   byte button = BUTTON_NONE;   // Return no button pressed if the below checks don't write to btn.
   
   // Read the button ADC pin voltage.
   buttonVoltage = analogRead( BUTTON_ADC_PIN );
   // Sense if the voltage falls within valid voltage windows.
   if( buttonVoltage < ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_RIGHT;
   }
   else if(   buttonVoltage >= ( UP_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( UP_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_UP;
   }
   else if(   buttonVoltage >= ( DOWN_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( DOWN_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_DOWN;
   }
   else if(   buttonVoltage >= ( LEFT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( LEFT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_LEFT;
   }
   else if(   buttonVoltage >= ( SELECT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( SELECT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_SELECT;
   }
   // Handle button flags for just pressed and just released events.
   if( ( buttonWas == BUTTON_NONE ) && ( button != BUTTON_NONE ) )
   {
      // The button was just pressed, set buttonJustPressed, this can optionally be used to trigger a once-off action for a button press event.
      // It's the duty of the receiver to clear these flags if it wants to detect a new button change event.
      buttonJustPressed  = true;
      buttonJustReleased = false;
   }
   if( ( buttonWas != BUTTON_NONE ) && ( button == BUTTON_NONE ) )
   {
      buttonJustPressed  = false;
      buttonJustReleased = true;
   }
   
   // Save the latest button value, for change event detection next time round.
   buttonWas = button;
   
   return( button );
}
