#include <Key.h>
#include <Keypad.h>
#include <stdio.h>
#include <LiquidCrystal.h>
#include <limits.h>
enum modalita {
  timer = 0,
  code = 1
};

bool isToneEnabled = true;

modalita currentMod = 0;

int firePin = A5;
bool firePinStatus = LOW;
long int fireEndTime;

int tonePin = 10;
bool tonePinStatus = LOW;
long int toneEndTime;
float mod = 1;
unsigned long toneInterval = 1000;
unsigned long toneDuration = 100;
unsigned long lastTimeOn = 0;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

String entries[20][2] = {                  //BombStatus
  {"----------------", "----------------"}, //0
  {"Bomba pronta    ", "A=opzioni       "}, //1
  {"Quanti min?     ", "min:<  >    A=ok"}, //2
  {"Password?       ", "pw:<      > A=ok"}, //3
  {"min:   pw:      ", "A=start B=abort "}, //4
  {"Esplode in mm:ss", "pw:<      > A=ok"}, //5
  {"Codice sbagliato", "RIPROVA TRA s..."}, //6
  {"Disinnescata! :)", "mm:ss     B=esci"}, //7
  {"Esplosa! :(     ", "00:00     B=esci"}, //8
  {"Modalita'?       ", "A=timer B=codice"}, //9
  {"Quanti secondi? ", "sec:<  >    A=ok"}, //10
  {"sec:   pw:      ", "A=ok    B=abort "}, //11
  {"pw?  x tentativi", "pw:<      > A=ok"}, //12
  {"Missione fallita", "          B=esci"}, //13
  {"Esplode in mm:ss", "**allontanarsi**"}, //14
  {"Bomba esplosa!!!", "          B=esci"} //15
};

unsigned long endTime;
unsigned long nextTry;
unsigned long timeLeft;
unsigned long lastRefresh;
unsigned long timerStart = 0;

//questi servono solo durante l'inizializzazione
int BombMin = 0;
int BombSec = 0;

int remainingAttemps = 0;
int BombStatus = 0;

char password[7] = {'0', '0', '0', '0', '0', '0', '\0'};
char tryPassword[7] = {'0', '0', '0', '0', '0', '0', '\0'};

const byte rows = 4; //four rows
const byte cols = 4; //three columns
char keys[rows][cols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
//byte rowPins[rows] = {A3, A2, A1, A0}; //connect to the row pinouts of the keypad
//byte colPins[cols] =  {9, 8, 7, 6}; //connect to the column pinouts of the keypad
byte colPins[rows] = {A3, A2, A1, A0}; //connect to the row pinouts of the keypad
byte rowPins[cols] =  {9, 8, 7, 6};  //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

void setup() {
  Serial.begin(9600);
  pinMode(tonePin, OUTPUT);
  pinMode(firePin, OUTPUT);
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
  if (key != NO_KEY) {
    Serial.println(key);
    keyPressed(key); //what to do when a key is pressed (logic)
  }

  performTimeDependantActions();

  //refresh the LCD
  if ((millis() - lastRefresh) > 150)
  {
    printEntry(BombStatus);
    lastRefresh = millis();
  }









  /*

    //man this code is shit....
    bool x = false;
    if (timerStart != 0 && (BombStatus == 5 || BombStatus == 6) && currentMod == timer) //se siamo nella fase di conto alla rovescia della modalità timer
    {
      unsigned long elapsedTime = millis() - timerStart;
      unsigned long LongBombMin = BombMin;
      unsigned long endTime = timerStart + LongBombMin * 60 * 1000;
      timeLeft = endTime - millis();
      if (millis() >= endTime)
      {
        BombStatus = 8;
        digitalWrite(firePin, HIGH);
        x = true;
      }

      Serial.println("-------------------------------------");
      Serial.println(LongBombMin);
      Serial.println(elapsedTime);
      Serial.println(endTime);
      Serial.println(timerStart);
      Serial.println(timeLeft);

      if (timeLeft <= 30000 && timeLeft > 10000) mod = 0.5;
      if (timeLeft <= 10000 && timeLeft > 3000) mod = 0.2;
      if (timeLeft <= 3000) mod = 0.1;
    }



    if (x || ((BombStatus == 5 || BombStatus == 6) && (millis() - lastTimeOn) >= toneInterval * mod) )
    {
      analogWrite(tonePin, 400);
      lastTimeOn = millis();
      x = false;
    }

    if ( (millis() - lastTimeOn) >= toneDuration )
    {
      if ((BombStatus != 8) || (BombStatus == 8 && ( (millis() - lastTimeOn) >= toneDuration * 4000)) )
      {
        analogWrite(tonePin, 0);
        digitalWrite(firePin, LOW);
      }
    }
  */
}

void performTimeDependantActions() {
  updatePinsStatus();
  setBeepFrequencyMod();

  if (BombStatus == 6 && millis() >= nextTry) {
   // Serial.println("wrong state over");
    if (currentMod == timer) BombStatus = 5;
    else if (currentMod == code) BombStatus = 12;
  }

  if (getTimeLeft() <= 0 && (BombStatus == 5 || BombStatus == 6 || BombStatus == 14)) {
    if (currentMod == timer) BombStatus = 8; //bomba esplosa  (hai perso)
    else if (currentMod == code) BombStatus = 15; //bomba esplosa (hai vinto)

    firePinStatus = HIGH;
    fireEndTime = millis() + 4000;
    tonePinStatus = HIGH;
    toneEndTime = millis() + 4000;
  }

  if ((BombStatus == 5 || BombStatus == 6 || BombStatus == 14) && (millis() - lastTimeOn) >= toneInterval * mod) {
    tonePinStatus = HIGH;
    toneEndTime = millis() + toneDuration;
  }

}

void setBeepFrequencyMod() {
  long int timeLeft = getTimeLeft();
  if (timeLeft <= 30000 && timeLeft > 10000) mod = 0.5;
  if (timeLeft <= 10000 && timeLeft > 3000) mod = 0.2;
  if (timeLeft <= 3000) mod = 0.1;
}

void updatePinsStatus() {
  long int now = millis();
  if (now >= fireEndTime) firePinStatus = LOW;
  if (now >= toneEndTime) tonePinStatus = LOW;
    

  digitalWrite(firePin, firePinStatus);

  if (tonePinStatus == HIGH)
    lastTimeOn = now;

  if (isToneEnabled || tonePinStatus == LOW) 
    analogWrite(tonePin, tonePinStatus * 400); //0 or 400
    
}


long int getTimeLeft() {
  long int now = millis();
  /*Serial.print("getting time left: ");
  Serial.print(endTime);
  Serial.print(" - ");
  Serial.print(now);
  Serial.print(endTime-now);
  Serial.println();*/
  return endTime-now;
}

void keyPressed(char key)
{
  switch (BombStatus) {
    case 1: {
        if (key == 'A') BombStatus = 9;
        break;
      }
    case 2: {
        if (key == 'A' && BombMin > 0)
          BombStatus = 3;
        else if (key >= '0' && key <= '9')
          addCharToIntFromRight(BombMin, key, 2);
        break;
      }
    case 3: {
        if (key == 'A') {
          if (currentMod == timer)  BombStatus = 4;
          else if (currentMod == code) BombStatus = 11;
        }
        else if (key >= '0' && key <= '9') {
          addCharToWordFIFO(password, key);
        }
        break;
      }
    case 4: {
        if (key == 'A') {
          BombStatus = 5;
          bombTimerStart();
        }
        else if (key == 'B')
          BombStatus = 1;
        break;
      }
    case 5: {
        if (key == 'A') {
          if (isPasswordCorrect (tryPassword))
            BombStatus = 7; //disinnescata
          else {
            wrongPasswordState();
          }
        }
        else if (key >= '0' && key <= '9')
          addCharToWordFIFO(tryPassword, key);
        break;
      }
    case 7:
    case 8:
    case 13:
    case 15: {
        if (key == 'B')
          BombStatus = 1;
        break;
      }
    case 9: {
        if (key == 'A') {
          currentMod = timer;
          BombStatus = 2;
        }
        else if (key == 'B') {
          currentMod = code;
          BombStatus = 10;
        }
        modInit(currentMod);
        break;
      }
    case 10: {
        if (key == 'A' && BombSec > 0) BombStatus = 3;
        else if (key >= '0' && key <= '9')
          addCharToIntFromRight(BombSec, key, 2);
        break;
      }
    case 11: {
        if (key == 'A') BombStatus = 12;
        else if (key == 'B') BombStatus = 1;
        break;
      }
    case 12: {
        if (key == 'A') {
          bool isCorrect = isPasswordCorrect(tryPassword);
          if (isCorrect ) {
            //Serial.println("Correct Password because isPasswordCorrect(tryPassword) returned TRUE");
            BombStatus = 14; //esplode in...
            bombTimerStart();
          }
          else {
            //Serial.println("Incorrect Password because isPasswordCorrect(tryPassword) returned FALSE");
            if (remainingAttemps > 1) {
              //Serial.print("remainingAttemps: ");
              //Serial.print(remainingAttemps);
              remainingAttemps--;
              //Serial.print(" -> ");
              //Serial.println(remainingAttemps);
              wrongPasswordState();
            }
            else
             // Serial.println("no more chanses left, going to bombstatus 13");
              BombStatus = 13; //missione fallita
          }
        }
        else if (key >= '0' && key <= '9')
          addCharToWordFIFO(tryPassword, key);
        break;
      }
  }
}

void modInit(modalita m) {
  if (m == timer) {
    //write "timer" mode init code here
  }
  else if (m == code) {
    //write "code" mode init code here
    endTime = LONG_MAX;
    setWordToZeros(tryPassword);
    remainingAttemps = 3;
  }
}

int wordLength(char _word[]) {
  //int wl = sizeof(_word) / sizeof(_word[0]) - 1; // returns the size of the char array (how many letters)
  int wl = 0;
  char letter = _word[0];

  //Serial.println("------------------------");
  while (letter != '\0') {
    //Serial.print(letter);
    wl++;
    letter = _word[wl];
  }
 // Serial.println();
  //Serial.print("Word Length: ");
  //Serial.println(wl);
  return wl;
}

void setWordToZeros(char _word[]) {
  int numberOfChars = wordLength(_word);
  //Serial.println("***setting to zero***");
  //Serial.print("numberOfChars = ");
  //Serial.println(numberOfChars);
  //Serial.println("List of operations:");
  for (int i = 0; i < numberOfChars; i++) {
    //Serial.print(_word[i]);
    //Serial.print(" -> ");
    _word[i] = '0';
    //Serial.println(_word[i]);
  }
  _word[numberOfChars] = '\0';
  //Serial.println("*********************");
}

void wrongPasswordState() {
  //Serial.println("entering in wrong password state");
  BombStatus = 6; //riprova tra...
  nextTry = millis() + 5000;
}

void addCharToIntFromRight(int &i, char c, int maxDigits) {
  i = i * 10; // sposta di un posto verso sinistra tutte le cifre
  i = i + (c - 48); // Aggiungo il valore di key nel posto delle unità, il -48 è per la coinversione ascii -> int
  i = i % (int) (pow(10, maxDigits) + 1); //rimuove tutte le cifre in eccesso
}

void addCharToWordFIFO(char _word[], char c) {
  int w_length = wordLength(_word);
  for (int i = 0; i < w_length - 1; i++) {
    _word[i] = _word[i + 1];
  }
  _word[w_length - 1] = c;
}

void bombTimerStart() {
  timerStart = millis();
  if (currentMod == timer) {
    setWordToZeros(tryPassword); //sets trypassword to zero
    endTime = timerStart + ((long int) BombMin) * 60 * 1000;
  }
  else if (currentMod == code) {
    endTime = timerStart + ((long int) BombSec) * 1000;
  }

  mod = 1; //sets interval between beeps "normal"

}

bool isPasswordCorrect(char pw[]) {
  bool isEqual = strcmp(password, pw) == 0;
  int c = 0;
  while (pw[c] != '\0') {
   // Serial.print(pw[c]);
    //Serial.print(" <-> ");
    //Serial.print(password[c]);
    //Serial.println();
    c++;
  }
 // if (isEqual) Serial.println("TRUE");
 // else Serial.println("FALSE");
  
  return isEqual;
}

void printEntry(int state) {
  
  //rendering static parts of the entry
  lcd.setCursor(0, 0);
  lcd.print(entries[state][0]);
  lcd.setCursor(0, 1);
  lcd.print(entries[state][1]);

  //rendering dynamic parts of the entry
  switch (state) {
    case 2:
      {
        lcd.setCursor(5, 1);
        char mins[2];
        sprintf(mins, "%02d", BombMin); //copia il numero a due cifre BombMin nella stringa mins
        lcd.print(mins);
        break;
      }
    case 3:
      {
        lcd.setCursor(4, 1);
        lcd.print(password);
        break;
      }
    case 4:
      {
        lcd.setCursor(4, 0);
        char mins[2];
        sprintf(mins, "%02d", BombMin);
        lcd.print(mins);
        lcd.setCursor(10, 0);
        lcd.print(password);
        break;
      }
    case 5:
      {
        lcd.setCursor(11, 0);
        char timestamp[6];
        int secondiTOT = timeLeft / 1000;
        int minutiTOT = secondiTOT / 60;
        int secondi = secondiTOT % 60;
        sprintf(timestamp, "%02d:%02d", minutiTOT, secondi);
        lcd.print(timestamp);

        lcd.setCursor(4, 1);
        lcd.print(tryPassword);
        break;
      }
    case 6:
      {
        lcd.setCursor(12, 1);
        if (millis() <= nextTry)
        {
          lcd.print( (nextTry - millis()) / 1000 + 1 );
        }
        break;
      }
    case 7:
      {
        lcd.setCursor(0, 1);
        char timestamp[6];
        int secondiTOT = timeLeft / 1000;
        int minutiTOT = secondiTOT / 60;
        int secondi = secondiTOT % 60;
        sprintf(timestamp, "%02d:%02d", minutiTOT, secondi);
        lcd.print(timestamp);
        break;
      }
    case 10:
      {
        lcd.setCursor(5, 1);
        char secs[2];
        sprintf(secs, "%02d", BombSec); //copia il numero a due cifre BombSec nella stringa secs
        lcd.print(secs);
        break;
      }
    case 11:
      {
        lcd.setCursor(4, 0);
        char secs[2];
        sprintf(secs, "%02d", BombSec);
        lcd.print(secs);
        lcd.setCursor(10, 0);
        lcd.print(password);
        break;
      }
    case 12:
      {
        //STAMPA LA PASSWORD
        lcd.setCursor(4, 1);
        lcd.print(tryPassword);

        //stampa numero di tentativi rimasti
        lcd.setCursor(5, 0);
        lcd.print(remainingAttemps);

        break;
      }
    case 14:
      {
        lcd.setCursor(11, 0);
        char timestamp[6];
        int secondiTOT = getTimeLeft() / 1000;
        int minutiTOT = secondiTOT / 60;
        int secondi = secondiTOT % 60;
        sprintf(timestamp, "%02d:%02d", minutiTOT, secondi);
        lcd.print(timestamp);
        break;
      }


  }
}
