#include <Arduino.h>
#include <ESP_FlexyStepper.h>

// Stepper pin assignments (Safe on C3 Super Mini)
const int MOTOR_STEP_PIN = 3;
const int MOTOR_DIRECTION_PIN = 4;

// Input pin assignments
const int ROT_TO_VERT_PIN = 5;
const int ROT_TO_HORZ_PIN = 6;
const int HOME_SET_PIN = 7;

// Maintenance pin assignments (Driven by FT232H Module)
const int MAINT_MODE = 1;
const int JOG_DIR = 10;
const int JOG_PB = 20;

// Safety pin assignment (Has external 10k ohm pulldown, measured clear at 0.6mV)
const int EMERGENCY_STOP_PIN = 21; 

// Output Pin Assignments
const int CMD_RCV = 8;

// Dynamic Speed Profiles
const int SPEED_VAL = 1000;
const int ACCEL_VAL = 1000;
const int DECEL_VAL = 1000;
long CURRENT_POS;

const int SAFE_SPD = 100;
const int SAFE_ACL = 100;
const int SAFE_DCL = 100;

const long VERT_POS = 500;
const long HORZ_POS = 0;

// State Variables
bool VERT_PIN_STATE;
bool HORZ_PIN_STATE;
bool IS_MOVING;
bool HOME_SET = false;
bool HOME_BUTTON_STATE;
bool MAINT_MODE_STATE; 
bool JOG_DIR_STATE;
bool JOG_PB_STATE;
bool E_STOP_TRIGGERED;
long STP_TO_TGT = 0;
int horzVertMode = 0;


// Edge tracking for FT232H pulse signal
bool LAST_JOG_PB_STATE = LOW; 

bool MAINT_SPDS_SET = LOW;
bool AUTO_SPDS_SET = LOW;
bool INIT_SPDS_SET = LOW;

class Timer {
  private:
    unsigned long previousMillis;
    unsigned long interval;
    bool running;

  public:
    Timer(unsigned long currentInterval) {
      interval = currentInterval;
      previousMillis = 0;
      running = false;
    }

    void start() {
      previousMillis = millis();
      running = true;
    }
	
    bool isRunning(){
      return running;
    }

    void stop() {
      running = false;
    }

    bool isReady() {
      if (!running) return false;
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; 
        return true;
      }
      return false;
    }
};

Timer MntModeTimer(1000);
Timer HomeDelayTimer(1000);
Timer StepStateDly(2000); 

ESP_FlexyStepper stepper;

void IRAM_ATTR emergencySwitchHandler() {
  E_STOP_TRIGGERED = HIGH;
}

void setHome() {
  if (!HomeDelayTimer.isRunning()) {
    HomeDelayTimer.start();
    HOME_SET = false;
  }

  if (HomeDelayTimer.isReady()) {
    stepper.setCurrentPositionAsHomeAndStop();
    CURRENT_POS = stepper.getCurrentPositionInSteps();
    HOME_SET = true;
    HomeDelayTimer.stop(); 
  }
}
void readInputs(){
  HOME_BUTTON_STATE = digitalRead(HOME_SET_PIN);
  VERT_PIN_STATE = digitalRead(ROT_TO_VERT_PIN);
  HORZ_PIN_STATE = digitalRead(ROT_TO_HORZ_PIN);
  MAINT_MODE_STATE = digitalRead(MAINT_MODE);
  JOG_DIR_STATE = digitalRead(JOG_DIR);
  JOG_PB_STATE = digitalRead(JOG_PB);
  E_STOP_TRIGGERED = digitalRead(EMERGENCY_STOP_PIN);
}
void setup() {
  
  pinMode(ROT_TO_VERT_PIN, INPUT);
  pinMode(ROT_TO_HORZ_PIN, INPUT);
  pinMode(HOME_SET_PIN, INPUT);
  pinMode(MAINT_MODE, INPUT); 
  pinMode(JOG_DIR, INPUT);
  pinMode(JOG_PB, INPUT);
  pinMode(EMERGENCY_STOP_PIN, INPUT);

  pinMode(CMD_RCV, OUTPUT);
  digitalWrite(CMD_RCV, LOW);
  
  attachInterrupt(digitalPinToInterrupt(EMERGENCY_STOP_PIN), emergencySwitchHandler, RISING);

  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);

  stepper.setSpeedInStepsPerSecond(SAFE_SPD);
  stepper.setAccelerationInStepsPerSecondPerSecond(SAFE_ACL);
  stepper.setDecelerationInStepsPerSecondPerSecond(SAFE_DCL);

  stepper.startAsService(0);

}

void loop() {

  readInputs();
  if (E_STOP_TRIGGERED == HIGH) {
    stepper.emergencyStop(true);
  }else {
    stepper.releaseEmergencyStop();
  }

  if(E_STOP_TRIGGERED == LOW && INIT_SPDS_SET == LOW){
    stepper.setSpeedInStepsPerSecond(250);
    stepper.setAccelerationInStepsPerSecondPerSecond(250);
    stepper.setDecelerationInStepsPerSecondPerSecond(250);
    INIT_SPDS_SET = HIGH;
  } else{
    INIT_SPDS_SET = LOW;
  }

  //Track motion states 
  STP_TO_TGT = stepper.getDistanceToTargetSigned();
  IS_MOVING = !stepper.motionComplete();

  //Tracking to PLC
  if (IS_MOVING || STP_TO_TGT != 0) {
    digitalWrite(CMD_RCV, HIGH);
  } else {
    digitalWrite(CMD_RCV, LOW);
    CURRENT_POS = stepper.getCurrentPositionInSteps(); 
  }

  //Homing Sequence Logic
  if (HOME_SET == false || HOME_BUTTON_STATE == HIGH) {
    setHome(); 
  }
  
  // =================================================================== MAINT MODE
  if (MAINT_MODE_STATE == HIGH) {
    if(MAINT_SPDS_SET == LOW){
      stepper.setSpeedInStepsPerSecond(250);
      stepper.setAccelerationInStepsPerSecondPerSecond(250);
      stepper.setDecelerationInStepsPerSecondPerSecond(250);
      MAINT_SPDS_SET = HIGH;
    }
    if (JOG_PB_STATE == HIGH && LAST_JOG_PB_STATE == LOW) {
      digitalWrite(CMD_RCV, HIGH); 
      if (JOG_DIR_STATE == HIGH) {
        stepper.setTargetPositionRelativeInSteps(1);  
      } else {
        stepper.setTargetPositionRelativeInSteps(-1); 
      }
    }
    
    if (stepper.motionComplete() == true && STP_TO_TGT == 0 && JOG_PB_STATE == LOW) {
      digitalWrite(CMD_RCV, LOW);
      MntModeTimer.stop();
    }
    LAST_JOG_PB_STATE = JOG_PB_STATE;
  }else{
    MAINT_SPDS_SET = LOW;
  }

  // =================================================================== AUTO MODE
  if (HOME_SET == true && MAINT_MODE_STATE == LOW) {
    readInputs();
    if(AUTO_SPDS_SET == LOW){
      stepper.setSpeedInStepsPerSecond(SPEED_VAL);
      stepper.setAccelerationInStepsPerSecondPerSecond(ACCEL_VAL);
      stepper.setDecelerationInStepsPerSecondPerSecond(DECEL_VAL);
      AUTO_SPDS_SET = HIGH;
    }

    if(!VERT_PIN_STATE && HORZ_PIN_STATE && !IS_MOVING){
      horzVertMode = 2;
    }else if(VERT_PIN_STATE && !HORZ_PIN_STATE && !IS_MOVING){
      horzVertMode = 1;
    }

    if(!IS_MOVING){
      switch (horzVertMode)
      {
        case 0:
          readInputs();
          break;
        case 1:
          readInputs();
          stepper.setTargetPositionInSteps(VERT_POS);
          if(!StepStateDly.isRunning() && stepper.motionComplete()){
            StepStateDly.start();
          }else if(StepStateDly.isReady()){
            horzVertMode = 0;
            StepStateDly.stop();
          }else if(!stepper.motionComplete()){
            horzVertMode = 1;
          }
          break;
        case 2:
          readInputs();
          stepper.setTargetPositionInSteps(HORZ_POS);
          if(!StepStateDly.isRunning() && stepper.motionComplete()){
            StepStateDly.start();
          }else if(StepStateDly.isReady()){
            horzVertMode = 0;
            StepStateDly.stop();
          }else if(!stepper.motionComplete()){
            horzVertMode = 2;
          }  
          break;
        default:
          break;
      }
    }
  }else{
    AUTO_SPDS_SET = LOW;
  }
}
