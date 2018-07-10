#include <Arduino.h>
#include "enums.h"

unsigned long currentMillis;

/** Cooling Unit */
int coolingUnitPin = 8; //Relais 1
bool coolingUnitState = false;
unsigned long freezeTime = 900000;

unsigned long afterFreezeTime = 15000;

/** Water Pump */
int waterPumpPin = 7; //Relais 2
bool waterPumpState = false;
unsigned long fillStartTime;
unsigned long fillTime = 15000;

/** Valve */
int valvePin = 6; //Relais 3
bool valveState = false;
unsigned long looseTime = 300000;

/** Motor */
int containerMotorPin = 5; //Relais 4
boolean containerMotorState = false;

/** Fan */
int fanPin = 4; //Fan
bool fanState = false;

int endStopUp = A5;
int endStopDown = A4;
int temperatureSensor = A6;

int waterSensor = A7;

unsigned long startupSleep = 10000;

GlobalState globalState = NOTHING;
GlobalState lastGlobalState = NOTHING;
unsigned long startOfState;

ContainerFill wantedContainerFill = EMPTY;
ContainerFill currentContainerFill = EMPTY;

ContainerPosition wantedContainerPosition = DOWN;
ContainerPosition currentContainerPosition = BETWEEN;

CoolingUnitState wantedCoolingUnitState = OFF;
CoolingUnitState currentCoolingUnitState = OFF;

ValveState wantedValveState = CLOSED;
ValveState currentValveState = CLOSED;

void toggleDevice(int* pin, bool* state) {
    if (*state == false) {
        digitalWrite(*pin, HIGH);
        *state = true;
    } else {
        digitalWrite(*pin, LOW);
        *state = false;
    }
}

void containerRoutine() {
    bool upState = digitalRead(endStopUp);
    bool downState = digitalRead(endStopDown);
    
    if (upState == true && downState == false) {
        currentContainerPosition = UP;
    } else if (upState == false && downState == true) {
        currentContainerPosition = DOWN;
    } else {
        currentContainerPosition = BETWEEN;
    }

    if (currentContainerPosition == wantedContainerPosition)  {
      if (containerMotorState == true) {
          digitalWrite(containerMotorPin, LOW);
          containerMotorState = false;
      }
      return;
    } else {
      if (containerMotorState == false) {
          digitalWrite(containerMotorPin, HIGH);
          containerMotorState = true;
      }
    }
}

void pumpRoutine() {
  if (wantedContainerFill == currentContainerFill) {
      if (waterPumpState == true) {
            digitalWrite(waterPumpPin, LOW);
            waterPumpState = false;
      }
      
    return;
  }
  
  if (wantedContainerFill == FULL && wantedContainerPosition != UP) {
    wantedContainerPosition = UP;
  }

  if (wantedContainerFill == FULL && currentContainerPosition == UP) {

      if (waterPumpState == false) {
            digitalWrite(waterPumpPin, HIGH);
            waterPumpState = true;
            fillStartTime = currentMillis;
      } else if (currentMillis - fillStartTime > fillTime) {
        digitalWrite(waterPumpPin, LOW);
        waterPumpState = false;
        currentContainerFill = FULL;
   }
  }  
}

void coolingUnitRoutine() {
    if (wantedCoolingUnitState == ON && currentCoolingUnitState == OFF) {
      if (coolingUnitState == false) {
          digitalWrite(coolingUnitPin, HIGH);
          coolingUnitState = true;
      }

      currentCoolingUnitState = ON;
    } else if (wantedCoolingUnitState == OFF && currentCoolingUnitState == ON) {
       if (coolingUnitState == true) {
          digitalWrite(coolingUnitPin, LOW);
          coolingUnitState = false;
      }
    }
}

void valveRoutine() {
    if (wantedValveState == OPEN && currentValveState == CLOSED) {
      if (valveState == false) {
          digitalWrite(valvePin, HIGH);
          valveState = true;
      }

      currentValveState = OPEN;
    } else if (wantedValveState == CLOSED && currentValveState == OPEN) {
       if (valveState == true) {
          digitalWrite(valvePin, LOW);
          valveState = false;
      }

      currentValveState = CLOSED;
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("BOOT");

    pinMode(coolingUnitPin, OUTPUT);
    pinMode(waterPumpPin, OUTPUT);
    pinMode(valvePin, OUTPUT);
    pinMode(containerMotorPin, OUTPUT);
    pinMode(fanPin, OUTPUT);

    pinMode(endStopUp, INPUT);
    pinMode(endStopDown, INPUT);
    pinMode(waterSensor, INPUT);
}

void loop() {
    currentMillis = millis();
  
    containerRoutine();
    pumpRoutine();
    coolingUnitRoutine();
    valveRoutine();

    if (globalState == NOTHING) {
      Serial.println("NOTHING");
      wantedContainerFill = EMPTY;
      wantedContainerPosition = DOWN;
      wantedCoolingUnitState = OFF;
      wantedValveState = OPEN;

      if (lastGlobalState != NOTHING) {
        lastGlobalState = NOTHING;
        startOfState = currentMillis;
      }

      if (currentMillis - startOfState > startupSleep) {
        globalState = FILL_CONTAINER;
      }

    }

    if (globalState == FILL_CONTAINER) {
      Serial.println("FILL_CONTAINER");
      wantedContainerFill = FULL;
      wantedContainerPosition = UP;
      wantedCoolingUnitState = OFF;
      wantedValveState = CLOSED;

      if (lastGlobalState != FILL_CONTAINER) {
        lastGlobalState = FILL_CONTAINER;
        startOfState = currentMillis;
      }

      if (currentContainerFill == FULL) {
        globalState = FREEZE;
      }
    }

    if (globalState == FREEZE) {
      Serial.println("FREEZE");
      wantedContainerFill = FULL;
      wantedContainerPosition = UP;
      wantedCoolingUnitState = ON;
      wantedValveState = CLOSED;

      if (lastGlobalState != FREEZE) {
        Serial.println("IFFFF");
        Serial.println(freezeTime);
        Serial.println(currentMillis - startOfState);
        lastGlobalState = FREEZE;
        startOfState = currentMillis;
      }

      if (currentMillis - startOfState > freezeTime) {
        globalState = AFTER_FREEZE;
      }      
    }

    
    if (globalState == AFTER_FREEZE) {
      Serial.println("AFTER_FREEZE");
      wantedContainerFill = EMPTY;
      wantedContainerPosition = DOWN;
      wantedCoolingUnitState = ON;
      wantedValveState = CLOSED;

      if (lastGlobalState != AFTER_FREEZE) {
        lastGlobalState = AFTER_FREEZE;
        startOfState = currentMillis;
      }

      if (currentMillis - startOfState > afterFreezeTime) {
        globalState = LOOSE;
      }        
    }

    if (globalState == LOOSE) {
      Serial.println("LOOSE");
      wantedContainerFill = EMPTY;
      wantedContainerPosition = DOWN;
      wantedCoolingUnitState = OFF;
      wantedValveState = OPEN;

      if (lastGlobalState != LOOSE) {
        lastGlobalState = LOOSE;
        startOfState = currentMillis;
      }

      if (currentMillis - startOfState > looseTime) {
        globalState = NOTHING;
      }
    }

    /**
     *
     * Wenn Status Leerlauf
     * stelle Status auf containerReset
     *
     *
     *
     * Wenn Status == containerReset
     * Endschalter Status prüfen
     * ist er unten? containerStatus = DOWN
     * ist er oben? containerStatus = UP
     * keines von beidem? Fahre runter
     * Stelle Status auf Befülle
     *
     *
     * wenn Status == Befülle
     * Fahre jetzt Hoch
     * Wenn hochgefahren starte Wasserpumpe.
     * Warte bis wasser kommt sonst stoppe nach zeitraum wenn kein wasser kommt.
     * Wenn wasser kommt, warte zeitraum bis voll
     * Schalte Pumpe aus
     * Stelle Status auf Gefriere
     *
     *
     * Wenn Status == Gefriere
     * Schalte Kühlaggregat an
     * warte laaange zeit
     * Fahre den Container runter
     * Schalte Kühlaggregat aus
     * Stelle Status auf Löse Eiswürfel
     *
     * Wenn Status == Löse Eiswürfel
     * Schalte das Ventil an
     * Schalte das Ventil aus
     * Stelle Status auf Leerlauf
     *
     * Status = werfe Eiswürfel in Vorrat
     * Fahre den Container hoch
     */
}
