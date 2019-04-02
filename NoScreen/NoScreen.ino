const int NUMBER_OF_CABLES = 3;

const int cables[NUMBER_OF_CABLES] = {5, 6, 7};
const int correctOrder[NUMBER_OF_CABLES] = {0, 1, 2};
bool cablesState[NUMBER_OF_CABLES];//HIGH -> connected, LOW -> disconnected
int cablesCut = 0;

long int lastChangeRed = 0;
long int lastChangeGreen = 0;
bool redLightState = LOW;
bool greenLightState = LOW;

enum state {
  initialization = 0,
  countdown = 1,
  exploded = 2,
  defused = 3,
  numberOfStates
};

state bombState;

const int redLightPin = 8;
const int greenLightPin = 9;

struct led {
  int pin;
  bool state;
  long int lastChangeTime;
  long int timeOn;
  long int cyclePeriod;
} redLed, greenLed;


void setup() {
  setBombState(initialization);
  Serial.begin(9600);
  for (int i = 0; i < NUMBER_OF_CABLES; i++) {
    pinMode(cables[i], INPUT);
  }
 
  ledSetup(redLed,redLightPin);
  ledSetup(greenLed, greenLightPin);

  waitForStart();
  setBombState(countdown);
}

void loop() {
  switch (bombState) {
    case countdown: {
        checkCables();
        break;
      }
    case exploded:
    case defused: delay(1000);
  }
  updateLeds();
}

void updateLed(led& myLed) {
  long int now = millis();
  if ((myled.state == HIGH) && (now >= myLed.lastChangeTime + myLed.timeOn)) {
    setLightState(myLed, LOW);
    myLed.lastChangeTime = now;
  }
  else if ((myled.state == LOW) && (now >= myLed.lastChangeTime + (myLed.cyclePeriod - myLed.timeOn) )) {
    setLightState(myLed, HIGH);
    myLed.lastChangeTime = now;
  }
}

void updateLeds() {
  updateLed(greenLed);
  updateLed(redLed);
}

bool setBombState(state i) {
  if (i >= 0 && i < numberOfStates) {
    if (bombState != i) {
      onBombStateLeaving(bombState, i);
      onBombStateEntering(bombState, i);
      bombState = i;
    }
    return true;
  }
  return false;
}

void onBombStateLeaving(state oldState, state newState) { //are you brexiting? LMAO
  /*switch(oldState){
    countdown: turnOff(redLed);
  }*/
}

void onBombStateEntering(state oldState, state newState) {
  
}

void ledSetup(led& myLed, int pin){
  pinMode(pin, OUTPUT);
  myLed.pin = pin;
  myLed.state = LOW;
  myLed.lastChangeTime = 0;
  myLed.timeOn = 0;
  myLed.cyclePeriod = 0;
}

void setLightState(led& myLed, bool newState) {
  myLed.state = newState;
}

void toggleLightState(led& myLed) {
  setLightState(myLed, !myLed.state);
}

void checkCables() {
  for (int i = 0; i < NUMBER_OF_CABLES; i++) {
    bool currentState = digitalRead(cables[i]); //if HIGH it's still connected, if LOW it has been cut
    if ((currentState == LOW) && (cablesState[i] == HIGH) ) {
      cablesState[i] = currentState;
      cablesCut ++;
      if (correctOrder[cablesCut - 1] != i) {
        bombExploded();
      } else if (cablesCut == NUMBER_OF_CABLES) {
        bombDefused();
      }
    }
  }
}

void bombExploded() {
  setBombState(exploded);
  setLightState(redLight, redLightState, HIGH);
  setLightState(greenLight, greenLightState, LOW);
}

void bombDefused() {
  setBombState(defused);
  setLightState(redLight, redLightState, LOW);
  setLightState(greenLight, greenLightState, HIGH);
}

/*
void ledBlink(const int ledPin, bool &ledState, long int &lastChange, int millisOn, int millisOff) {
  long int now = millis();
  if ((ledState == HIGH) && (now >= lastChange + millisOn)) {
    setLightState(ledPin, ledState, LOW);
    lastChange = now;
  }
  else if ((ledState == LOW) && (now >= lastChange + millisOff)) {
    setLightState(ledPin, ledState, HIGH);
    lastChange = now;
  }
}
*/
void waitForStart() { //starts only if all the cables are connected
  bool everythingIsFine;
  do {
    toggleLightState(greenLight, greenLightState);
    toggleLightState(redLight, redLightState);
    delay(250);

    everythingIsFine = true;
    for (int i = 0; i < NUMBER_OF_CABLES; i++) {
      cablesState[i] = digitalRead(cables[i]);
      if (cablesState[i] == LOW)
        everythingIsFine = false;
    }
  } while (!everythingIsFine);

  setLightState(greenLight, greenLightState, LOW);
  setLightState(redLight, redLightState, LOW);
}
