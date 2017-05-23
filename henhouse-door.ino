
#include <Bounce2.h>


#define RELAY_PLUS_MODE 1
#define RELAY_MINUS_MODE 0

#define RELAY_1_PIN 3//black wire motor
#define RELAY_2_PIN 4// red wire motor
#define DOOR_UP_CONTACT_PIN 5
#define DOOR_DOWN_CONTACT_PIN 6
#define LIGHT_SENSOR_ANALOG_PIN A0

unsigned long SLEEP_TIME = 1000; // Sleep time between reads (in milliseconds)
unsigned long DOOR_TIME = 10000;  // time to keep motor on to open or close the door (in milliseconds)

const int OPEN_DOOR_THRESHOLD = 60;  //TODO configure that !
const int CLOSE_DOOR_THRESHOLD = 40; //TODO configure that !

const int batteryMinThreshold = 11900; // mV

//MySensor gw;
//MyMessage msg(CHILD_ID_LIGHT, V_LIGHT_LEVEL);
int lastLightLevel;

boolean isDoorOpen = false;


Bounce debouncerDown = Bounce();
Bounce debouncerUp = Bounce();


void setup() {
  Serial.begin(9600);
  //gw.begin(NULL,NODE_ID);
  //gw.sendSketchInfo("PouleDoor", "1.0");
  //gw.present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);

  pinMode(DOOR_UP_CONTACT_PIN, INPUT_PULLUP);
  pinMode(DOOR_DOWN_CONTACT_PIN, INPUT_PULLUP);
  debouncerUp.attach(DOOR_UP_CONTACT_PIN);
  debouncerUp.interval(50); // interval in ms
  debouncerDown.attach(DOOR_DOWN_CONTACT_PIN);
  debouncerDown.interval(50); // interval in ms

  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);



  //test digital light sensor
  pinMode(8, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  motorStop();
  ledOff();
}

void ledOn() {
  digitalWrite(LED_BUILTIN, HIGH);
}
void ledOff() {
  digitalWrite(LED_BUILTIN, LOW);
}

void motorStop() {
  //attention à choisir le mode repos des relais pour le moteur arreté
  // donc changer les branchements pas ce code, ni la valeur de RELAY_MINUS_MODE qui doit etre zero (?)
  digitalWrite(RELAY_1_PIN, RELAY_MINUS_MODE);
  digitalWrite(RELAY_2_PIN, RELAY_MINUS_MODE);
}

void motorUp() {
  digitalWrite(RELAY_1_PIN, RELAY_MINUS_MODE);
  digitalWrite(RELAY_2_PIN, RELAY_PLUS_MODE);
}

void motorDown() {
  digitalWrite(RELAY_1_PIN, RELAY_PLUS_MODE);
  digitalWrite(RELAY_2_PIN, RELAY_MINUS_MODE);
}

boolean contactUp() {
  //return (digitalRead(DOOR_UP_CONTACT_PIN) == LOW);
  return (debouncerUp.read()  == LOW);
}
boolean contactDown() {
  //return (digitalRead(DOOR_DOWN_CONTACT_PIN) == LOW);
  return (debouncerDown.read()  == LOW);
}

int getLightLevel() {
  return (1023 - analogRead(LIGHT_SENSOR_ANALOG_PIN)) / 10.23;
}

int getBatteryMilliVolt() {
  //TODO check it is really A0
  // ~12V-to-mersure=V1---R1---V2=A0---R2--- 0V
  //R1=1M - R2=68K
  //17.2 Max - 1.1 Ref
  //V1 (volt) = V2*((R1+R2)/R2) = analogRead(A0)/1023*1.1*1068/68
  // analogRead(A0) is 0 to 1023
  long res = ((long)analogRead(A0)) * 1100L * 1068L / 1023L / 68L;
  return (int)res;
}

void loop() {

  if (getBatteryMilliVolt() < batteryMinThreshold) {
    ledOn();
  }


  debouncerUp.update();
  debouncerDown.update();

  int lightLevel = getLightLevel();
  Serial.println("----");
  Serial.println(lightLevel);

  //  int toto = digitalRead(8);
  //  Serial.println(toto);


  if (lightLevel != lastLightLevel) {
    //gw.send(msg.set(lightLevel));
    lastLightLevel = lightLevel;
  }

  if (lightLevel > OPEN_DOOR_THRESHOLD && !isDoorOpen) {
    motorUp();
    unsigned long st = millis();
    while ((!contactUp()) && (millis() < (st + DOOR_TIME))) {
    }

    motorStop();
    //gw.sleep(DOOR_TIME);
    isDoorOpen = true;
  }

  if (lightLevel < CLOSE_DOOR_THRESHOLD && isDoorOpen) {
    motorDown();
    unsigned long st = millis();
    boolean contactUpHasBeenReleaseOneTime = false;
    while ((!contactDown()) && (millis() < (st + DOOR_TIME))) {
      if ((contactUp() && contactUpHasBeenReleaseOneTime)) {
        stopAllError();
        ledOn();
        break;
      }
      if (!contactUp()) {
        contactUpHasBeenReleaseOneTime = true;
      }
    }
    //gw.sleep(DOOR_TIME);
    motorStop();
    isDoorOpen = false;
  }


  //gw.sleep(SLEEP_TIME);
  delay(SLEEP_TIME);

}

void stopAllError() {
  while (true) {
    ledOn();
    delay(300);
    ledOff();
    delay(300);
  }
}

