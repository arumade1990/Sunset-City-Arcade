#include <Arduino.h>
#include <ESP_FlexyStepper.h>
#include <Preferences.h>

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
bool AT_HORZ_POS = LOW;
bool AT_VERT_POS = LOW;
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

Preferences SaveState;

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

void setInitPos(){
  SaveState.begin("config", false);
  AT_HORZ_POS = SaveState.getBool("HORZ_POS", LOW);
  AT_VERT_POS = SaveState.getBool("VERT_POS", LOW);
	SaveState.end();

  if(AT_HORZ_POS && !AT_VERT_POS){
    setHome();
    return;
  }else if(!AT_HORZ_POS && AT_VERT_POS){
    if(!HomeDelayTimer.isRunning()){
      HomeDelayTimer.start();
      HOME_SET = false;
    }

    if(HomeDelayTimer.isReady()){
      stepper.setCurrentPositionInSteps(VERT_POS);
      stepper.setTargetPositionInSteps(VERT_POS);
      CURRENT_POS = stepper.getCurrentPositionInSteps();
      
      if(stepper.motionComplete()){
        HOME_SET = true;
        HomeDelayTimer.stop();
      }
    }
    return;
  }else if(!AT_HORZ_POS && !AT_VERT_POS){
    setHome();
    return;
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

  if (E_STOP_TRIGGERED) {
    stepper.emergencyStop(true);
  }else {
    stepper.releaseEmergencyStop();
  }

  if(!E_STOP_TRIGGERED && !INIT_SPDS_SET){
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

  //==================================================================== HOMING AND INIT POSITION LOGIC
  if (!HOME_SET) {
    setInitPos(); 
  }

  if(HOME_BUTTON_STATE){
    setHome();
  }
  
  // =================================================================== MAINT MODE
  if (MAINT_MODE_STATE) {
    if(!MAINT_SPDS_SET){
      stepper.setSpeedInStepsPerSecond(250);
      stepper.setAccelerationInStepsPerSecondPerSecond(250);
      stepper.setDecelerationInStepsPerSecondPerSecond(250);
      MAINT_SPDS_SET = HIGH;
    }
    if (JOG_PB_STATE && !LAST_JOG_PB_STATE) {
      digitalWrite(CMD_RCV, HIGH); 
      if (JOG_DIR_STATE) {
        stepper.setTargetPositionRelativeInSteps(1);  
      } else {
        stepper.setTargetPositionRelativeInSteps(-1); 
      }
    }
    
    if (stepper.motionComplete() && STP_TO_TGT == 0 && !JOG_PB_STATE) {
      digitalWrite(CMD_RCV, LOW);
      MntModeTimer.stop();
    }
    LAST_JOG_PB_STATE = JOG_PB_STATE;
  }else{
    MAINT_SPDS_SET = LOW;
  }

  // =================================================================== AUTO MODE
  if (HOME_SET && !MAINT_MODE_STATE) {
    readInputs();
    if(!AUTO_SPDS_SET){
      stepper.setSpeedInStepsPerSecond(SPEED_VAL);
      stepper.setAccelerationInStepsPerSecondPerSecond(ACCEL_VAL);
      stepper.setDecelerationInStepsPerSecondPerSecond(DECEL_VAL);
      AUTO_SPDS_SET = HIGH;
    }

    if(!VERT_PIN_STATE && HORZ_PIN_STATE && !IS_MOVING){ //Horizontal Mode
      horzVertMode = 2;
    }else if(VERT_PIN_STATE && !HORZ_PIN_STATE && !IS_MOVING){ //Vertical Mode
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
            AT_HORZ_POS = LOW;
            AT_VERT_POS = HIGH;
          }else if(StepStateDly.isReady()){
            horzVertMode = 3;
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
            AT_HORZ_POS = HIGH;
            AT_VERT_POS = LOW;
          }else if(StepStateDly.isReady()){
            horzVertMode = 3;
            StepStateDly.stop();
          }else if(!stepper.motionComplete()){
            horzVertMode = 2;
          }  
          break;
        case 3:
          SaveState.begin("config", false);
          SaveState.putBool("HORZ_POS", AT_HORZ_POS);
          SaveState.putBool("VERT_POS", AT_VERT_POS);
          SaveState.end();
          horzVertMode = 0;
        break;
        default:
          break;
      }
    }
  }else{
    AUTO_SPDS_SET = LOW;
  }
}
