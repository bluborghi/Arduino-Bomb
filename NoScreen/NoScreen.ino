const int NUMBER_OF_CABLES = 3;
const int cables[NUMBER_OF_CABLES] = {5, 6, 7};
const int correctOrder[NUMBER_OF_CABLES] = {0, 1, 2};
const int tonePin = 4;
bool cablesState[NUMBER_OF_CABLES];//HIGH -> connected, LOW -> disconnected
int cablesCut = 0;
unsigned long lastTimeEverythingWasFine;
unsigned long lastTimeEverythingWasNotFine;

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
  pinMode(tonePin, OUTPUT);
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
    if ((myLed.state == HIGH) && (now >= myLed.lastChangeTime + myLed.timeOn)) {
      setLightState(myLed, LOW);
    }
    else if ((myLed.state == LOW) && (myLed.cyclePeriod == 0)) { //"just blink once" type of blink
      stopLightBlink(myLed);
    }
    else if ((myLed.state == LOW) && (now >= myLed.lastChangeTime + (myLed.cyclePeriod - myLed.timeOn) )) {
      setLightState(myLed, HIGH);
    }
  } 
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
        tone(tonePin,440,100);
        break;
      }
    case exploded: {
        setLightState(redLed, HIGH);
        tone(tonePin,800,5000);
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
  Serial.println("Checking cables");
  for (int i = 0; i < NUMBER_OF_CABLES; i++) {
    Serial.print(i);
    Serial.print(") -> ");
    cablesState[i] = digitalRead(cables[i]);
    Serial.print(cablesState[i]);
    if (cablesState[i] == LOW)
      everythingIsFine = false;
    Serial.println();
  }
  Serial.print("everything is fine: ");
  Serial.println(everythingIsFine);
  if (everythingIsFine) lastTimeEverythingWasFine = millis();
  else lastTimeEverythingWasNotFine = millis();

  Serial.print("lastTimeEverythingWasFine: "); Serial.println(lastTimeEverythingWasFine);
  Serial.print("lastTimeEverythingWasNotFine: "); Serial.println(lastTimeEverythingWasNotFine);
  Serial.print("now: "); Serial.println(millis());
  
  if (lastTimeEverythingWasFine >= lastTimeEverythingWasNotFine +5000)
    setBombState(countdown);
}
