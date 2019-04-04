const int NUMBER_OF_CABLES = 3;

const int cables[NUMBER_OF_CABLES] = {5, 6, 7};
const int correctOrder[NUMBER_OF_CABLES] = {0, 1, 2};
bool cablesState[NUMBER_OF_CABLES];//HIGH -> connected, LOW -> disconnected
int cablesCut = 0;
int lastTimeEverythingWasFine;
int lastTimeEverythingWasNotFine;

enum state {
  noState = 0,
  initialization = 1,
  countdown = 2,
  exploded = 3,
  defused = 4,
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

void setLightBlink(led& myLed, int timeOn, int cyclePeriod, bool initState = HIGH); //needs to be declared here in order to use default parameters

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < NUMBER_OF_CABLES; i++) {
    pinMode(cables[i], INPUT);
  }
  ledSetup(redLed, redLightPin);
  ledSetup(greenLed, greenLightPin);
  setBombState(initialization);
}

void loop() {
  switch (bombState) {
    case initialization: {
        waitForStart();
        break;
      }
    case countdown: {
        checkCables();
        break;
      }
  }
  updateLeds();
}

void updateLed(led& myLed) {
  long int now = millis();
  digitalWrite(myLed.pin, myLed.state);

  //  Serial.print("Pin: ");
  //  Serial.print(myLed.pin);
  //  Serial.print(" ");
  //  Serial.print("State: ");
  //  Serial.print(myLed.state);
  //  Serial.print(" ");
  //  Serial.print("isBlinking: ");

  if (isBlinking(myLed)) {
    Serial.println(true);
    if ((myLed.state == HIGH) && (now >= myLed.lastChangeTime + myLed.timeOn)) {
      setLightState(myLed, LOW);
    }
    else if ((myLed.state == LOW) && (myLed.cyclePeriod == 0)) { //"just blink once" type of blink
      stopLightBlink(myLed);
    }
    else if ((myLed.state == LOW) && (now >= myLed.lastChangeTime + (myLed.cyclePeriod - myLed.timeOn) )) {
      setLightState(myLed, HIGH);
    }
  } else Serial.println(false);
}

void updateLeds() {
  updateLed(greenLed);
  updateLed(redLed);
}

bool setBombState(state i) {
  if (i >= 0 && i < numberOfStates) {
    if (bombState != i) {


      int lifesucks2 = i;
      int lifesucks = bombState;
      Serial.print(lifesucks);
      Serial.print(" -> ");
      Serial.print(lifesucks2);
      Serial.print("\n");

      onBombStateLeaving(bombState, i);
      onBombStateEntering(bombState, i);
      bombState = i;
      Serial.println("bomb state set");
    }
    return true;
  }
  return false;
}

void onBombStateLeaving(state& oldState, state& newState) { //are you brexiting? LMAO
  switch (oldState) {
    case initialization: {
        stopLightBlink(redLed);
        stopLightBlink(greenLed);
        break;
      }
    case countdown: {
        stopLightBlink(redLed);
        stopLightBlink(greenLed);
        break;
      }
  }
}

void onBombStateEntering(state& oldState, state& newState) {
  switch (newState) {
    case initialization: {

        Serial.println("initialization entering state 1");
        setLightBlink(redLed, 250, 500);
        setLightBlink(greenLed, 250, 500);
        break;
      }
    case countdown: {

        Serial.println("initialization entering state 2");
        setLightBlink(redLed, 200, 1000);
        break;
      }
    case defused: {
        setLightState(greenLed, HIGH);
        break;
      }
    case exploded: {
        setLightState(redLed, HIGH);
        break;
      }
  }
}


void ledSetup(led& myLed, int pin) {
  pinMode(pin, OUTPUT);
  myLed.pin = pin;
  setLightState(myLed, LOW);
  myLed.timeOn = 0;
  myLed.cyclePeriod = 0;
}

bool isBlinking(led& myLed) {
  return (myLed.timeOn != 0);
}

void setLightBlink(led& myLed, int timeOn, int cyclePeriod, bool initState = HIGH) {
  Serial.print("setting light blink, Pin: ");
  Serial.println(myLed.pin);
  setLightState(myLed, initState);
  myLed.timeOn = timeOn;
  myLed.cyclePeriod = cyclePeriod;
}

void setOneTimeBlink(led& myLed, int timeOn) {
  setLightBlink(myLed, timeOn, 0);
}
void stopLightBlink(led& myLed) {
  Serial.print("stopping light blink: ");
  setLightBlink(myLed, 0, 0, LOW);
}

void setLightState(led& myLed, bool newState) {
  myLed.state = newState;
  myLed.lastChangeTime = millis();
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
        setBombState(exploded);
      }
      else {
        setOneTimeBlink(greenLed, 100);
        if (cablesCut == NUMBER_OF_CABLES) setBombState(defused);
      }
    }
  }
}

void waitForStart() { //starts only if all the cables are connected
  bool everythingIsFine;

  everythingIsFine = true;
  for (int i = 0; i < NUMBER_OF_CABLES; i++) {
    cablesState[i] = digitalRead(cables[i]);
    if (cablesState[i] == LOW)
      everythingIsFine = false;
  }

  
  if (everythingIsFine) lastTimeEverythingWasFine = millis();
  else lastTimeEverythingWasNotFine = millis();

  if (lastTimeEverythingWasFine >= lastTimeEverythingWasNotFine +5000)
    setBombState(countdown);
}
