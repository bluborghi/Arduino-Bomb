
const int NUMBER_OF_CABLES = 3;

const int cables[NUMBER_OF_CABLES] = {2, 3, 4};
const int redLight = 5;
const int greenLight = 6;


void setup() {
  Serial.begin(9600);
  for (int i = 0; i < NUMBER_OF_CABLES; i++) {
    pinMode(cables[i], INPUT);
  }
  pinMode(redLight,OUTPUT);
  pinMode(greenLight,OUTPUT);
}

void loop() {
  
}
