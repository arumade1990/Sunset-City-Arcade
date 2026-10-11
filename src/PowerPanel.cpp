// This code is for the Power Sequence Panel for the Sunset City Arcade Project
// Andy Rumade - 9/8/26
//Test for commit path

#include <Arduino.h>

// Define the INPUT pins
const int IX00 = 17; // ON PB
const int IX01 = 18; // PLC 1+2 READY
const int IX02 = 19; // PC RUN FROM PLC 2 (FT232H)
const int IX03 = 21; // _60VDC OK
const int IX04 = 22; // MAINT MODE KEY
const int IX05 = 23; // SHUTDOWN READY FROM PLC 2
const int IX06 = 27; // MAINT PB
const int IX07 = 32; // SPARE
const int IX10 = 33; // STEPPER PANEL FAULT

// Input states
bool IX00_ST = LOW;
bool IX01_ST = LOW;
bool IX02_ST = LOW;
bool IX03_ST = LOW;
bool IX04_ST = LOW;
bool IX05_ST = LOW;
bool IX06_ST = LOW;
bool IX07_ST = LOW;
bool IX10_ST = LOW;

// Define the OUTPUT pins 
const int QX00 = 1;  // CR3 - SEQUENCE START
const int QX01 = 2;  // CR4 - SEQUENCE STOP
const int QX02 = 3;  // CR2 - PLC 1 READY
const int QX03 = 4;  // CR5 - GREEN PL
const int QX04 = 5;  // CR6 - RED PL
const int QX05 = 12; // CR7 - 60VDC AND PC POWER
const int QX06 = 13; // CR8 - SYSTEM READY 
const int QX07 = 14; // CR9 - TOTAL POWER OFF
const int QX10 = 15; // CR10 - PC POWER PULSE
const int QX11 = 16; // CR10.5 - DISPLAY AND AUDIO POWER CONACTOR

// Output states
bool QX00_ST = LOW;
bool QX01_ST = LOW;
bool QX02_ST = LOW;
bool QX03_ST = LOW;
bool QX04_ST = LOW;
bool QX05_ST = LOW;
bool QX06_ST = LOW;
bool QX07_ST = LOW;
bool QX10_ST = LOW;
bool QX11_ST = LOW;

// Define the ANALOG INPUT pins
const int IW00 = 34;
const int IW01 = 35;
const int IW02 = 36;
const int IW03 = 39;

// Input aliases
bool PWR_PB = LOW;
bool BOTH_PLC_RDY = LOW;
bool SYS_RDY = LOW;
bool _60VDC_OK = LOW;
bool MNT_MODE_ACTIVE = LOW;
bool SHUTDOWN_RDY = LOW;
bool MNT_MODE_PB = LOW;
bool STP_PNL_FLT = LOW;

//Output aliases
bool SEQ_START_MEM = LOW;
bool SEQ_STOP_MEM = LOW;
bool PLC1_RDY = LOW;
bool GRN_PL = LOW;
bool RED_PL = LOW;
bool MC2_PWR = LOW;
bool SYS_RDY_OUT = LOW;
bool TOTAL_PWR_OFF = LOW;
bool PC_PWR_PLS = LOW;
bool DSP_AUD_PWR = LOW;

//Internal memory bools
bool FAULTED = LOW;
int PwrUpComplete = 0;
int PwrTries = 0;

//Flip flop Class
class ToggleFlipFlop {
  private:
    bool lastToggleState = false;
    unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50; 

  public:
    void update(bool &targetVar, bool toggleInput, bool togglePressedState) {

      bool isTogglePressed = (toggleInput == togglePressedState);

      if (isTogglePressed != lastToggleState) {
        if ((millis() - lastDebounceTime) > debounceDelay) {

          if (isTogglePressed) {
            targetVar = !targetVar; // Flip the state
          }
          lastDebounceTime = millis();
          lastToggleState = isTogglePressed;
        }
      }
    }

    void reset(bool &targetVar){
      targetVar = false; 
      lastToggleState = false; 
    }
};

// Reusable timer engine
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
	
    bool isRunning() {
      return running;
    }

    void stop() {
      running = false;
    }

    void setInterval(unsigned long newInterval) {
      interval = newInterval;
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

// Flasher engine
class Flasher {
  private:
    unsigned long previousMillis;
    unsigned long interval;
    bool running;
    bool idleState;
  
  public:
    Flasher(unsigned long currentInterval) {
      previousMillis = 0;
      interval = currentInterval;
      running = false;
      idleState = false;
    }

    bool start(bool startIdleState) {
      previousMillis = millis();
      idleState = startIdleState;
      running = true;
      return true; 
    }

    bool flash(bool &variableToFlash) {
      if (!running) {
        variableToFlash = idleState;
        return false;
      }
      
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        variableToFlash = !variableToFlash; 
        return true; 
      }
      return false; 
    }

    void setInterval(unsigned long newInterval) {
      interval = newInterval;
    }

    bool stop() {
      running = false;
      return true;
    }

    bool isRunning() {
      return running;
    }
};

//Special Timers
class OneShotPulse {
  private:
    unsigned long startTime;
    bool isRunning = false;
    bool lastTriggerState = false;

  public:
    void update(bool &pulsedVar, bool pulsedState, bool trigger, bool trigState, unsigned long duration) {
      bool triggerActivated = (trigger == trigState && lastTriggerState != trigState);
      lastTriggerState = trigger;

      if (triggerActivated && !isRunning) {
        pulsedVar = pulsedState;
        startTime = millis();
        isRunning = true;
      }

      if (isRunning) {
        if (millis() - startTime >= duration) {
          pulsedVar = !pulsedState;
          isRunning = false;
        }
      }
    }

    void reset(){
      isRunning = false;
      lastTriggerState = false;
    }
};

class TimerON {
  private:
    unsigned long startTime = 0;
    bool isRunning = false;

  public:
    void update(bool &timedVar, bool timedState, bool trigger, bool trigState, unsigned long duration) {

      bool triggerActivated = (trigger == trigState);

      if (triggerActivated) {
        if (!isRunning) {
          startTime = millis();
          isRunning = true;
        }

        if (millis() - startTime >= duration) {
          timedVar = timedState; 
        } else {
          timedVar = !timedState; 
        }
      } else {
        isRunning = false;
        timedVar = !timedState; 
      }
    }

    void reset(){
      isRunning = false;
      startTime = 0;
    }
};

class TimerOFF{
  private:
    unsigned long startTime;
    bool isRunning = false;
    bool lastTriggerState = false;
    bool trigWasPressed = false;

  public:
    void update(bool &timedVar, bool timedState, bool trigger, bool trigState, unsigned long duration) {

      bool triggerPressed;
      
      if (trigger == trigState){
        triggerPressed = true;
      }else{
        triggerPressed = false;
      }

      if (triggerPressed == true) {
        timedVar = timedState;
        trigWasPressed = true;
        isRunning = false;
      }
      if(triggerPressed == false && trigWasPressed == true && !isRunning){
        startTime = millis();
        isRunning = true;
      }

      if (isRunning) {
        if (millis() - startTime >= duration) {
          timedVar = !timedState;
          trigWasPressed = false;
          isRunning = false;
        }
      }
    }

    void reset(){
      isRunning = false;
      startTime = 0;
      trigWasPressed = false;
    }
};

//Instantiation of class objects

//Timers
Timer PcPwrCycleTimer(300000); //5 minutes
Timer PcPwrBtnCycleTimer(600000); //10 minutes
Timer PcPwrBtnDly(2000);
Timer ScreenAudioDly(5000);
Timer PnlShutdownCycleTime(30000);

//Special timers
TimerOFF PwrBtnCoolDown;
TimerON ShutdownConfirmTmr;
TimerON TotalShutdownTimer;
OneShotPulse PcPwrBtnPls;

//Flip Flops
ToggleFlipFlop PwrBtnFF;

//Flashers
Flasher RedFlash(1000);
Flasher GrnFlash(2000);

enum RunMode {
  Boot_State,
  PowerUp_State, 
  Maint_State,
  Fault_State, 
  Shutdown_State,
  PcPwrOn_State,
  DispAudPwr_State,
  Idle_State
};

RunMode PanelMode = Boot_State;

// ========================================== Application Routines

void readInputs(){
	IX00_ST = digitalRead(IX00); // ON PB
	IX01_ST = digitalRead(IX01); // PLC 1+2 READY
	IX02_ST = digitalRead(IX02); // PC RUN FROM PLC 2 (FT232H)
	IX03_ST = digitalRead(IX03); // _60VDC OK
	IX04_ST = digitalRead(IX04); // MAINT MODE KEY
	IX05_ST = digitalRead(IX05); // SHUTDOWN READY FROM PLC 2
	IX06_ST = digitalRead(IX06); // MAINT PB
	IX07_ST = digitalRead(IX07); // SPARE PIN
	IX10_ST = digitalRead(IX10); // STEPPER PANEL FAULT  

	// Copy values to matching tags
	PWR_PB = IX00_ST; //Green
	BOTH_PLC_RDY = IX01_ST; //Blue
	SYS_RDY = IX02_ST; //Green
	_60VDC_OK = IX03_ST; //Blue
	MNT_MODE_ACTIVE = IX04_ST; //Yellow
	SHUTDOWN_RDY = IX05_ST; //White
	MNT_MODE_PB = IX06_ST; //Black
	STP_PNL_FLT = IX10_ST; //Red  
}


void writeOutputs() {
  QX00_ST = SEQ_START_MEM; //Green
  QX01_ST = SEQ_STOP_MEM; //Yellow
  QX02_ST = PLC1_RDY; //Purple
  QX03_ST = GRN_PL; //Green
  QX04_ST = RED_PL; //Red
  QX05_ST = MC2_PWR; //Blue
  QX06_ST = SYS_RDY_OUT; //Green
  QX07_ST = TOTAL_PWR_OFF; //Red
  QX10_ST = PC_PWR_PLS; //Lavender
  QX11_ST = DSP_AUD_PWR; //Orange

  digitalWrite(QX00, QX00_ST);
  digitalWrite(QX01, QX01_ST);
  digitalWrite(QX02, QX02_ST);
  digitalWrite(QX03, QX03_ST);
  digitalWrite(QX04, QX04_ST);
  digitalWrite(QX05, QX05_ST);
  digitalWrite(QX06, QX06_ST);
  digitalWrite(QX07, QX07_ST);
  digitalWrite(QX10, QX10_ST);
  digitalWrite(QX11, QX11_ST);
}

void MaintMode(){
  return;
}

void PcPwrOff(){

  static bool shutdownConfirm = LOW;

  if(PwrTries > 0){
    if(SHUTDOWN_RDY){
      shutdownConfirm = HIGH;
    }else if(!SHUTDOWN_RDY){
      shutdownConfirm = LOW;
    }else if(PwrTries == 0){
      TOTAL_PWR_OFF = HIGH;
      if(!PnlShutdownCycleTime.isRunning()){
        PnlShutdownCycleTime.start();
      }
    }

    ShutdownConfirmTmr.update(MC2_PWR, LOW, shutdownConfirm, HIGH, 5000);

    if(!MC2_PWR && shutdownConfirm){
      TotalShutdownTimer.update(TOTAL_PWR_OFF, HIGH, MC2_PWR, LOW, 3000);
      if(!PnlShutdownCycleTime.isRunning()){
        PnlShutdownCycleTime.start();
      }
    }

    if(PnlShutdownCycleTime.isReady()){
      PanelMode = Fault_State;
      PnlShutdownCycleTime.stop();
      return;
    }
  }
  return;
}

bool PwrBtn(){
  static bool coolDown = LOW;
  static bool PWR_PB_MEM = LOW;

  if(!coolDown){
    PwrBtnFF.update(PWR_PB_MEM, PWR_PB, HIGH);
  }
  PwrBtnCoolDown.update(coolDown, HIGH, PWR_PB, HIGH, 5000);
  
  return PWR_PB_MEM;
}

bool PcPwrHandler(bool reset = false){
  
  static bool PwrBtnTrig = false;
  static bool complete = false;
  static bool Step_1 = false;
  static bool Step_2 = false;
  bool result = complete;

  if(reset){
    complete = false;
    reset = false;
    PcPwrBtnDly.stop();
    PcPwrBtnPls.reset();
    PcPwrBtnCycleTimer.stop();
  }

  if(!FAULTED && !MNT_MODE_ACTIVE){
    if(!PcPwrBtnDly.isRunning() && !complete){
      PcPwrBtnDly.start();
    }

    if(PcPwrBtnDly.isReady() && !Step_1){
      PcPwrBtnDly.stop();
      Step_1 = true;
    }

    if(Step_1 && !Step_2){
      PwrBtnTrig = HIGH;
      Step_2 = HIGH;
      ++PwrTries;
    }

    PcPwrBtnPls.update(PC_PWR_PLS, HIGH, PwrBtnTrig, HIGH, 1000);

    if(!PcPwrBtnCycleTimer.isRunning()){
      PcPwrBtnCycleTimer.start();
    }

    if(PcPwrBtnCycleTimer.isReady() && PwrTries >= 3){
      PcPwrBtnCycleTimer.stop();
      FAULTED = HIGH;
      complete = false;
    }else if(PcPwrBtnCycleTimer.isReady()){
      Step_2 = LOW;
      PwrBtnTrig = LOW;
    }

    if(SYS_RDY){
      PcPwrBtnCycleTimer.stop();
      SYS_RDY_OUT = HIGH;
      complete = true;
      PanelMode = DispAudPwr_State;
    }

  }else if(FAULTED){
    PanelMode = Fault_State;
  }else if(MNT_MODE_ACTIVE){
    PanelMode = Maint_State;
    PcPwrBtnCycleTimer.stop();
    PcPwrBtnDly.stop();
  }

  return complete;

}

bool DisplayAudioPwr(bool reset = false){

  static bool complete = false;

  if(reset){
    complete = false;
    ScreenAudioDly.stop();
  }

  if(!ScreenAudioDly.isRunning() && !complete){
    ScreenAudioDly.start();
  }
  
  if(ScreenAudioDly.isReady() && !complete){
    ScreenAudioDly.stop();
    DSP_AUD_PWR = HIGH;
    complete = true;
    PanelMode = Idle_State;
  }

  return complete;
}

void FaultMode(){
  return;
}

bool PowerUpMode(bool reset = false){
  static bool complete = false;
  bool result = complete;

  if(!PcPwrCycleTimer.isRunning()){
    PcPwrCycleTimer.start();
  }
  
  if(reset){
    complete = false;
    reset = false;
  }
  if(!PcPwrCycleTimer.isReady()){
    if(!BOTH_PLC_RDY && !_60VDC_OK && !complete){ //Initial condition to trigger the MC2 on
      MC2_PWR = HIGH;
    }else if(MC2_PWR && BOTH_PLC_RDY &&_60VDC_OK){ //Condition to seal it in
      MC2_PWR = HIGH;
      complete = true;
      PanelMode = PcPwrOn_State;
      PcPwrCycleTimer.stop();
    }else{
      complete = false;
    }
  }else if(PcPwrCycleTimer.isReady()){
    PanelMode = Fault_State;
    complete = false;
    PcPwrCycleTimer.stop();
  }

  return result;

}

void setup(){
	pinMode(IX00, INPUT);
	pinMode(IX01, INPUT);
	pinMode(IX02, INPUT);
	pinMode(IX03, INPUT);
	pinMode(IX04, INPUT);
	pinMode(IX05, INPUT);
	pinMode(IX06, INPUT);
	pinMode(IX07, INPUT);
	pinMode(IX10, INPUT);

	pinMode(QX00, OUTPUT);
	pinMode(QX01, OUTPUT);
	pinMode(QX02, OUTPUT);
	pinMode(QX03, OUTPUT);
	pinMode(QX04, OUTPUT);
	pinMode(QX05, OUTPUT);
	pinMode(QX06, OUTPUT);
	pinMode(QX07, OUTPUT);
	pinMode(QX10, OUTPUT);
	pinMode(QX11, OUTPUT);

  RedFlash.start(LOW);
  GrnFlash.start(LOW);

}

void loop(){ 
	delay(10);
	readInputs();

  if(PwrBtn() || PanelMode == Shutdown_State){
    switch (PanelMode)
    {
    case Boot_State:
      PLC1_RDY = HIGH;
      GrnFlash.flash(GRN_PL);
      PanelMode = PowerUp_State;
      break;
    case PowerUp_State:
      GrnFlash.setInterval(1000);
      GrnFlash.flash(GRN_PL);
      PowerUpMode();
      break;
    case PcPwrOn_State:
      GrnFlash.setInterval(500);
      GrnFlash.flash(GRN_PL);
      PcPwrHandler();
    break;
    case DispAudPwr_State:
      DisplayAudioPwr();
      break;
    case Idle_State:
      if(GrnFlash.isRunning()){
        GrnFlash.stop();
        GRN_PL = HIGH;
      }
      break;
    case Shutdown_State:
      GrnFlash.flash(GRN_PL);
      PcPwrOff();
      break;
    case Fault_State:
      RedFlash.flash(RED_PL);
      break;
    case Maint_State:
      GrnFlash.flash(GRN_PL);
      break;
    default:
      break;
    }
  }else if(!PwrBtn() && PLC1_RDY){
    if(!GrnFlash.isRunning()){
      GrnFlash.start(LOW);
      GrnFlash.setInterval(2000);
    }
    PanelMode = Shutdown_State;
  }else if(MNT_MODE_ACTIVE){
    if(!GrnFlash.isRunning()){
      GrnFlash.start(LOW);
    }
    GrnFlash.setInterval(3000);
    PanelMode = Maint_State;
  }else if(FAULTED){
    if(GrnFlash.isRunning()){
        GrnFlash.stop();
        GRN_PL = LOW;
    }
    if(!RedFlash.isRunning()){
      RedFlash.start(LOW);
    }
    RedFlash.setInterval(1000);
    PanelMode = Fault_State;
  }

  if(PanelMode == Idle_State){
    SEQ_START_MEM = HIGH;
  }else{
    SEQ_START_MEM = LOW;
  }

	writeOutputs();
}