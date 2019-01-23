#include <Key.h>
#include <Keypad.h>
#include <stdio.h>
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


const byte rows = 4; //four rows
const byte cols = 4; //three columns
char keys[rows][cols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[rows] = {A3,A2,A1,A0}; //connect to the row pinouts of the keypad
byte colPins[cols] =  {9,8,7,6};//connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

void setup() {
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  char key = keypad.getKey();

  if (key != NO_KEY){
    Serial.println(key);
    lcd.setCursor(13,1);
    lcd.print(key);
  }
  
  lcd.setCursor(0,2);
  // print the number of seconds since reset:
  int secondiTOT = millis()/1000;
  int minutiTOT = secondiTOT/60;
  int ore = minutiTOT/60;
  int minuti = minutiTOT%60;
  int secondi = secondiTOT%60;

  

  char tbs[10];


 
  sprintf(tbs, "%d:%02d:%02d", ore, minuti, secondi);
  lcd.print(tbs); 
  lcd.print("");//does not compile without this
}
