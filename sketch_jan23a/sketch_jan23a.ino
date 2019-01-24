#include <Tone.h>
#include <Key.h>
#include <Keypad.h>
#include <stdio.h>
#include <LiquidCrystal.h>


int tonePin = 10;
float mod = 1;
unsigned long toneInterval = 1000;
unsigned long toneDuration = 1;
unsigned long lastTimeOn = 0;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

String entries[20][2] = {                  //BombStatus
  {"----------------","----------------"}, //0
  {"Bomba pronta    ","A=opzioni       "}, //1
  {"Quanti min?     ","min:<  >    A=ok"}, //2
  {"Password?       ","pw:<      > A=ok"}, //3
  {"min:   pw:      ","A=start B=abort "}, //4
  {"Esplode in mm:ss","pw:<      > A=ok"}, //5
  {"Codice sbagliato","RIPROVA TRA s..."}, //6
  {"Disinnescata! :)","mm:ss     B=esci"}, //7
  {"Esplosa! :(     ","00:00     B=esci"}  //8
};

unsigned long nextTry;
unsigned long timeLeft;
unsigned long lastRefresh;
unsigned long timerStart = 0;
int BombMin = 0;
int BombStatus = 0;
char password[7] = {'0','0','0','0','0','0'}; 
char tryPassword[7] = {'0','0','0','0','0','0'};
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
  pinMode(tonePin,OUTPUT);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  BombStatus = 0;  
  printEntry(BombStatus);
  
  delay(1000);
  BombStatus = 1;  
  printEntry(BombStatus);
  lastRefresh = millis();
}

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY){
    Serial.println(key);
    switch (BombStatus) {
      case 1:
            {
            if (key == 'A') BombStatus = 2;
            break;
            }
      case 2:
            {
              if (key == 'A' && BombMin>0) BombStatus = 3;
              else if(key >= '0' && key <= '9') 
              {
                BombMin = BombMin%10; // lascia solo la cifra delle unità
                BombMin = BombMin*10; // sposta la cifra delle unità nella cifra delle decine
                BombMin = BombMin + (key - 48); // Aggiungo il valore di key, il -48 è per la coinversione ascii -> int
              } 
              break;
            }
       case 3:
            {
              if (key == 'A') BombStatus = 4;
              else if(key >= '0' && key <= '9') 
              {
                for (int i=0; i<5; i++){
                  password[i] = password[i+1];                  
                }
                password[5] = key;
              } 
              break;
            }
        case 4:
            {
              if (key == 'A')
              {
                BombStatus = 5;
                for (int i = 0; i<6; i++) tryPassword[i] = '0';
                timerStart = millis();
                mod = 1; //sets interval between beeps "normal"
              }
              else if(key == 'B') BombStatus = 1;
              break;
            }
        case 5:
            {
              if (key == 'A') 
              {
                bool equal = true;
                for (int i=0;i<6;i++)
                {
                  if (password[i] != tryPassword[i]) equal = false;
                }
                if (!equal) 
                {
                  BombStatus = 6;
                  nextTry = millis() + 5000;
                }
                else BombStatus = 7;
              }
              else if(key >= '0' && key <= '9') 
              {
                for (int i=0; i<5; i++){
                  tryPassword[i] = tryPassword[i+1];                  
                }
                tryPassword[5] = key;
              } 
              break;
            }
        case 7:
        case 8:
            {
              if (key == 'B') 
              {
                BombStatus = 1;
              }
              break;
            }
    }
  }


  if (BombStatus == 6 && millis()>=nextTry)
  {
    BombStatus = 5;
  }

  if (timerStart != 0 && (BombStatus==5 || BombStatus==6) ) //se siamo nella fase di conto alla rovescia
  {
    unsigned long elapsedTime = millis() - timerStart;
    unsigned long LongBombMin = BombMin;
    unsigned long endTime = timerStart + LongBombMin*60*1000;
    timeLeft = endTime - millis();
    if (millis()>=endTime) BombStatus = 8;
    Serial.println("-------------------------------------");
    Serial.println(LongBombMin);
    Serial.println(elapsedTime);
    Serial.println(endTime);
    Serial.println(timerStart);
    Serial.println(timeLeft);

    if (timeLeft<=30000 && timeLeft>10000) mod = 0.5;
    if (timeLeft<=10000 && timeLeft>3000) mod = 0.2;
    if (timeLeft<=3000) mod = 0.1;
  }
  
  
  if ((millis() - lastRefresh)>50)
  {
    printEntry(BombStatus);
    lastRefresh = millis();
  }
 
  if ((BombStatus == 5 || BombStatus == 6) && (millis() - lastTimeOn)>=toneInterval*mod )
  {
    analogWrite(tonePin,400);
    lastTimeOn = millis();
  }

  if ( (millis() - lastTimeOn) >= toneDuration )
  {
    if ((BombStatus!=8) || (BombStatus==8 && ( (millis() - lastTimeOn) >= toneDuration*2000)) )
    analogWrite(tonePin,0);
  }

  
  
 /*TEST
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
  
  char timestamp[10];

  sprintf(timestamp, "%d:%02d:%02d", ore, minuti, secondi);
  lcd.print(timestamp); 
  lcd.print("");//does not compile without this
  */
}

void printEntry(int i){
  lcd.setCursor(0,0);
  lcd.print(entries[i][0]);
  lcd.setCursor(0,1);
  lcd.print(entries[i][1]);

  switch (i) {
      case 2:
           { 
            lcd.setCursor(5,1);
            char mins[2];
            sprintf(mins, "%02d",BombMin);
            lcd.print(mins);
            break;
           }
      case 3:
           { 
            lcd.setCursor(4,1);
            lcd.print(password);
            break;
           }
      case 4:
           { 
            lcd.setCursor(4,0);
            char mins[2];
            sprintf(mins, "%02d",BombMin);
            lcd.print(mins);
            lcd.setCursor(10,0);
            lcd.print(password);
            break;
           }
      case 5:
           { 
            lcd.setCursor(11,0);
            char timestamp[6];
            int secondiTOT = timeLeft/1000;
            int minutiTOT = secondiTOT/60;
            int secondi = secondiTOT%60;
            sprintf(timestamp, "%02d:%02d",minutiTOT,secondi);
            lcd.print(timestamp);
            
            lcd.setCursor(4,1);
            lcd.print(tryPassword);
            break;
           }
      case 6:
           { 
            lcd.setCursor(12,1);
            if (millis()<=nextTry)
            {
              lcd.print( (nextTry-millis())/1000 +1 );
            }
            break;
           }
      case 7:
           {
            lcd.setCursor(0,1);
            char timestamp[6];
            int secondiTOT = timeLeft/1000;
            int minutiTOT = secondiTOT/60;
            int secondi = secondiTOT%60;
            sprintf(timestamp, "%02d:%02d",minutiTOT,secondi);
            lcd.print(timestamp);
            break;    
           }
    }
}
