#include <Arduino.h>

unsigned long switchTime = 2000;      // in ms
unsigned long FotgjengerDelay = 1000; // in ms
unsigned long changeTime = 7000;      // in ms
unsigned long previousChange = 0;

int currentState = 2;
int previousCycleState = currentState;
bool EastKnapp = false;
bool NorthKnapp = false;
bool changedFotgjenger = true;
int fotgjengerStateStart = 3;
// States
#define SouthDrive 0
#define NorthDrive 1
#define EastDrive 2
#define FotgjengerEast 3
#define FotgjengerNorth 4
#define FotgjengerBegge 5
// trafikklys
#define SouthStraight 0
#define SouthEast 1
#define NorthStraight 2
#define NorthEast 3
#define East 4
// Arduino pins
#define EastFotgjengerLys 16
#define NorthFotgjengerLys 15
#define EastFotgjengerKnapp 17
#define NorthFotgjengerKnapp 18

class trafikklys {
  uint8_t green, yellow, red;
  unsigned long waittime = 0;
  bool isOff = true, isGoalOff = true;

public:
  trafikklys(uint8_t g = 0, uint8_t y = 0, uint8_t r = 0) {
    green = g;
    yellow = y;
    red = r;
    digitalWrite(red, HIGH);
  }
  void changeLys(unsigned long currentTime, bool on) {
    if (((isOff) && (isGoalOff) && (!on)) || ((!isOff) && (!isGoalOff) && (on)))
      return;
    isGoalOff = !on;
    isOff = on;
    digitalWrite(yellow, HIGH);
    if (!on) {
      digitalWrite(green, LOW);
      currentTime += switchTime / 2;
    } else {
      currentTime += switchTime / 2;
    }
    waittime = currentTime + switchTime;
  }
  void doScheduledChange(unsigned long currentTime) {
    if ((currentTime < waittime) || (isOff == isGoalOff))
      return;
    if (isGoalOff) {
      digitalWrite(red, HIGH);
    } else {
      digitalWrite(red, LOW);
      digitalWrite(green, HIGH);
    }
    isOff = isGoalOff;
    digitalWrite(yellow, LOW);
  }
};
trafikklys Lys[5];

void setup() {
  for (int i = 0; i < 17; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  pinMode(EastFotgjengerKnapp, INPUT);
  pinMode(NorthFotgjengerKnapp, INPUT);
  Lys[SouthStraight] = trafikklys(0, 1, 2);
  Lys[SouthEast] = trafikklys(3, 4, 5);
  Lys[NorthStraight] = trafikklys(6, 7, 8);
  Lys[NorthEast] = trafikklys(9, 10, 11);
  Lys[East] = trafikklys(12, 13, 14);
}

void TurnOn(int id[5], unsigned long t, unsigned int size = 2) {
  for (int i = 0; i < 5; i++) {
    bool commonId = false;
    for (unsigned int k = 0; k < size; k++) {
      if (!size)
        break;
      if (id[k] == i)
        commonId = true;
    }
    Lys[i].changeLys(t, commonId);
  }
}

void loop() {
  unsigned long t = millis();
  for (int i = 0; i < 5; i++) {
    Lys[i].doScheduledChange(t);
  }

  if (currentState != FotgjengerBegge) {
    if ((!EastKnapp) && (currentState != FotgjengerEast))
      EastKnapp = digitalRead(EastFotgjengerKnapp);
    if ((!NorthKnapp) && (currentState != FotgjengerNorth))
      NorthKnapp = digitalRead(NorthFotgjengerKnapp);
  }

  if ((!changedFotgjenger) &&
      (t >= previousChange + FotgjengerDelay + switchTime)) {
    switch (currentState) {
    case FotgjengerEast:
      digitalWrite(EastFotgjengerLys, HIGH);
      break;
    case FotgjengerBegge:
      digitalWrite(EastFotgjengerLys, HIGH);
    case FotgjengerNorth:
      digitalWrite(NorthFotgjengerLys, HIGH);
    }
    changedFotgjenger = true;
  } else if ((changedFotgjenger) &&
             (t >= previousChange + changeTime - FotgjengerDelay) &&
             (currentState >= fotgjengerStateStart)) {
    digitalWrite(EastFotgjengerLys, LOW);
    digitalWrite(EastFotgjengerLys, LOW);
    digitalWrite(NorthFotgjengerLys, LOW);
  }
  // change state after time interval
  if (t < previousChange + changeTime)
    return;
  // choose the new state accordingly
  if ((EastKnapp) && (NorthKnapp)) {
    currentState = FotgjengerBegge;
  } else if (EastKnapp) {
    currentState = FotgjengerEast;
  } else if (NorthKnapp) {
    currentState = FotgjengerNorth;
  } else {
    previousCycleState++;
    if (previousCycleState >= fotgjengerStateStart)
      previousCycleState = 0;
    currentState = previousCycleState;
  }
  // apply new state
  switch (currentState) {
  case SouthDrive: {
    int enable[] = {SouthStraight, SouthEast};
    TurnOn(enable, t);
  } break;
  case NorthDrive: {
    int enable[] = {NorthStraight, NorthEast};
    TurnOn(enable, t);
  } break;
  case EastDrive: {
    int enable[] = {East, SouthEast};
    TurnOn(enable, t);
  } break;
  case FotgjengerEast: {
    int enable[] = {SouthStraight, NorthStraight};
    TurnOn(enable, t);
  } break;
  case FotgjengerNorth: {
    int enable[] = {SouthEast};
    TurnOn(enable, t, 1);
  } break;
  case FotgjengerBegge: {
    int enable[1];
    TurnOn(enable, t, 0);
  } break;
  }
  previousChange = t;
  if (currentState >= fotgjengerStateStart) {
    changedFotgjenger = false;
    EastKnapp = false;
    NorthKnapp = false;
  } else {
    previousCycleState = currentState;
  }
}
