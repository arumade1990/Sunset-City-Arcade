// This code is for the Power Sequence Panel for the Sunset City Arcade Project
// Andy Rumade - 9/8/26

#include <Arduino.h>
#include <Timer.h>
#include <ToggleFlipFlop.h>
#include <Flasher.h>
#include <OneShotPulse.h>
#include <TimerON.h>
#include <TimerOFF.h>

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
bool InIdle = LOW;

//Instantiation of class objects

//Timers
Timer PcPwrCycleTimer(300000); //5 minutes
Timer PcPwrBtnCycleTimer(480000); //8 minutes
Timer PcPwrBtnDly(2000);
Timer ScreenAudioDly(5000);
Timer PnlShutdownCycleTime(300000); //5 minutes
Timer MntTimer1(3000);
Timer FltTimer(5000);
Timer MntTimerShort(2000);
Timer MntFlashTimer(5000);

//Special timers
TimerOFF PwrBtnCoolDown;
TimerON ShutdownConfirmTmr;
TimerON TotalShutdownTimer;
TimerON MaintPBTimer;
OneShotPulse PcPwrBtnPls;

//Flip Flops
ToggleFlipFlop PwrBtnFF;

//Flashers
Flasher RedFlash(1000);
Flasher GrnFlash(2000); //deafult 2000
Flasher MntFlash1(500);
int MntFlash1Toggles = 0;
Flasher FltFlash(1000);
int FltFlashToggles = 0;

enum RunMode {
  Boot_State,
  PowerUp_State, 
  Maint_State,
  Fault_State, 
  Shutdown_State,
  PcPwrOn_State,
  DispAudPwr_State,
  Idle_State,
  PreBoot
};
RunMode PanelMode = PreBoot;

enum MntMode{
  IdleDoNothing,
  ForceDispAudON,
  ForcePCPwrBtn,
  ForceTotalPwrOFF
};

MntMode MaintModes = IdleDoNothing;
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
  //SPARE_IN = IX07_ST; //SPARE
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

void PcPwrOff(){

  static bool shutdownConfirm = LOW;
  bool CycleTimeout = PnlShutdownCycleTime.isReady();

  if(!SEQ_STOP_MEM){
    SEQ_STOP_MEM = HIGH;
  }

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

    ShutdownConfirmTmr.update(MC2_PWR, LOW, shutdownConfirm, HIGH, 5000); //Shuts off MC2 once SHUTDOWN READY  signal is received. 

    if(!MC2_PWR && shutdownConfirm){
      TotalShutdownTimer.update(TOTAL_PWR_OFF, HIGH, MC2_PWR, LOW, 3000);
      if(!PnlShutdownCycleTime.isRunning()){
        PnlShutdownCycleTime.start();
      }
    }

    if(CycleTimeout){
      PanelMode = Fault_State;
      FAULTED = HIGH;
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

  // Handle manual or automatic sequence resets
  if(reset){
    complete = false;
    Step_1 = false;
    Step_2 = false;
    PwrBtnTrig = false;
    PwrTries = 0;              // Reset your global try counter
    PcPwrBtnDly.stop();
    PcPwrBtnPls.reset();
    PcPwrBtnCycleTimer.stop();
    return false;
  }

  if(!FAULTED && !MNT_MODE_ACTIVE){
    
    if(!PcPwrBtnDly.isRunning() && !complete && PwrTries == 0){
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

    if(Step_2 && !PcPwrBtnCycleTimer.isRunning()){
      PcPwrBtnCycleTimer.start();
    }

    if(PcPwrBtnCycleTimer.isRunning() && PcPwrBtnCycleTimer.isReady()){
      PcPwrBtnCycleTimer.stop();

      if(PwrTries >= 3 && !SYS_RDY){
        FAULTED = HIGH;
        complete = false;
        PanelMode = Fault_State;
      } else {
        Step_2 = LOW;
        PwrBtnTrig = LOW;
        PcPwrBtnPls.reset();
      }
    }

    if(SYS_RDY){
      PcPwrBtnCycleTimer.stop();
      PcPwrBtnDly.stop();
      SYS_RDY_OUT = HIGH;
      complete = true;
      PanelMode = DispAudPwr_State;
    }

  } else if(FAULTED){
    return false;
  } else if(MNT_MODE_ACTIVE){
    PcPwrBtnCycleTimer.stop();
    PcPwrBtnDly.stop();
    return false;
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
    if(!MNT_MODE_ACTIVE){
      PanelMode = Idle_State;
    }else if(MNT_MODE_ACTIVE){
      return complete;
    }
  }
  return complete;
}

void FaultMode() { // If a fault occurs, you basically have to do the power sequence manually.

  static bool timeMet = false; // Initialize explicitly

  if (timeMet && MNT_MODE_ACTIVE && FAULTED) {
    delay(3000);
    RED_PL = LOW;
    FltTimer.stop();
    FAULTED = LOW;
    timeMet = false;      
    PanelMode = Maint_State;
    return;
  }

  else if (FAULTED && MNT_MODE_ACTIVE) {
    if (!FltTimer.isRunning()) {
      FltTimer.start();
    }

    if (FltTimer.isReady()) {
      timeMet = true;         
      RED_PL = HIGH;
      FltTimer.stop();
    }
  } 

  else {
    FltTimer.stop();
  }
  return;
}


bool PowerUpMode(bool reset = false){

  static bool complete = false;
  bool result = complete;
  bool CycleTimeout = PcPwrCycleTimer.isReady();

  if(!PcPwrCycleTimer.isRunning()){
    PcPwrCycleTimer.start();
  }
  
  if(reset){
    complete = false;
    reset = false;
  }
  if(!CycleTimeout){
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
  }
  
  if(CycleTimeout){
    FAULTED = HIGH;
    PanelMode = Fault_State;
    complete = false;
    PcPwrCycleTimer.stop();
  }

  return result;

}

void DirectPwrDown(){
  static bool OneShot = LOW;

  if(MNT_MODE_PB && !OneShot){
    OneShot = HIGH;
  }

  if(!MNT_MODE_PB && OneShot){
    TOTAL_PWR_OFF = HIGH;
  }
}

void MaintMode(){
  static int MaintBtnPresses = 0;
  static bool PBCountTrig = LOW;
  static bool oneShot = LOW;
  static bool ModeSet = LOW;
  static bool SecondOneShot = LOW;

  if(MNT_MODE_ACTIVE && !FAULTED){
    
    MaintPBTimer.update(PBCountTrig, HIGH, MNT_MODE_PB, HIGH, 150); //Maint PB TON
    
    if(!oneShot && PBCountTrig && !ModeSet){  //Count the number of times the maint pb is pressed. Reset stuff when you do that. 
      //Serial.println("Maint PB Pressed.");
      MntTimerShort.start();
      MntTimer1.stop();
      MntFlash1.stop(GRN_PL);
      MntFlashTimer.stop();    
      MaintBtnPresses++;
      oneShot = HIGH;
    }else if(!PBCountTrig){
      oneShot = LOW;
    }
    if(MntTimerShort.isReady()){  //This section sets the variable to indicate with the green PL how many PB presses you made. 
      MntFlash1Toggles = MaintBtnPresses; 
      MntTimerShort.stop();
      //Serial.println("You pressed the button ");
      //Serial.println(MaintBtnPresses);
      //Serial.println("times.");
      MntFlash1.start(LOW, MntFlash1Toggles);
      MntFlashTimer.start();
    }

    if(MntFlashTimer.isRunning()){ //This section actually does the flashing of the green PL.
      MntFlash1.flash(GRN_PL);
    }

    if(MntFlashTimer.isReady()){ //This section is purely for delay to let the user decide if they made a mistake. Think quick!
      MntFlashTimer.stop();
      MntFlash1.stop(GRN_PL);
      MntTimer1.start();
    }

    if(MntTimer1.isReady()){ //This section sets the final mode. No going back unless you exit maint mode and reset the variables. 
      //Serial.print("Mode is Set.");
      ModeSet = HIGH;
      GRN_PL = HIGH;
      MntTimer1.stop();

      if(MaintBtnPresses == 1){
        MaintModes = ForceDispAudON;
      }else if(MaintBtnPresses == 2){
        MaintModes = ForcePCPwrBtn;
      }else if(MaintBtnPresses == 3){
        MaintModes = ForceTotalPwrOFF;
      }else if(MaintBtnPresses > 3){
        MaintBtnPresses = 0;
        MaintModes = IdleDoNothing;
      }

    }

    if(ModeSet){
      switch (MaintModes)
      {
      case IdleDoNothing:
        delay(10);
        break;
      case ForceDispAudON:
      //Serial.println("ForceDispAudON State entered");
        if(MNT_MODE_PB && !SecondOneShot){
          SecondOneShot = HIGH;
          DSP_AUD_PWR = !DSP_AUD_PWR;
        }else if(!MNT_MODE_PB){
          SecondOneShot = LOW;
        }
        break;
      case ForcePCPwrBtn:
        PC_PWR_PLS = MNT_MODE_PB;
        break;
      case ForceTotalPwrOFF:
        DirectPwrDown();
        break;
      default:
        delay(10);
        break;
      }
    }
    return;
    
  }else if(!MNT_MODE_ACTIVE){
    SecondOneShot = LOW;
    oneShot = LOW;
    MaintBtnPresses = 0;
    MaintModes = IdleDoNothing;
    MntFlash1.stop(GRN_PL);
    MntTimer1.stop();
    MntTimerShort.stop();
    MntFlashTimer.stop();
    MaintPBTimer.reset();
    ModeSet = LOW;
    PanelMode = Idle_State;
  }
  return;
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

  //Serial.begin(115200);

  //while(!Serial){
	//delay(100);
	//}

  //Serial.println("Serial Initialized.");

}

void loop(){ 
	delay(10);
	readInputs();

    switch (PanelMode)
    {
    case Boot_State:
      //Serial.println("Boot State");
      PLC1_RDY = HIGH;
      GrnFlash.flash(GRN_PL);
      PanelMode = PowerUp_State;
      break;
    case PowerUp_State:
      //Serial.println("Power Up State");
      GrnFlash.setInterval(1000);
      GrnFlash.flash(GRN_PL);
      PowerUpMode();
      break;
    case PcPwrOn_State:
      //Serial.println("PcPwrOn State");
      GrnFlash.setInterval(500);
      GrnFlash.flash(GRN_PL);
      PcPwrHandler();
    break;
    case DispAudPwr_State:
      //Serial.println("DispAudPwr State");
      DisplayAudioPwr();
      break;
    case Idle_State:
      //Serial.println("Idle State");
      if(GrnFlash.isRunning() || !InIdle){
        GrnFlash.start(GRN_PL);
        GrnFlash.stop(GRN_PL);
        GRN_PL = HIGH;
        InIdle = HIGH;
      }
      break;
    case Shutdown_State:
      //Serial.println("Shutdown State");
      GrnFlash.flash(GRN_PL);
      PcPwrOff();
      break;
    case Fault_State:
      //Serial.println("Fault State");
      FaultMode();
      RedFlash.flash(RED_PL);
      GRN_PL = LOW;
      break;
    case Maint_State:
     //Serial.println("Maint State");
      if(GrnFlash.isRunning() || RedFlash.isRunning() || InIdle){;
        GrnFlash.stop(GRN_PL);
        RedFlash.stop(RED_PL);
        RED_PL = LOW;
        GRN_PL = LOW;
        InIdle = LOW;
      }
      MaintMode();
      break;
    case PreBoot:
      delay(90);
      break;
    default:
      break;
    }
  
  if(PwrBtn() && PanelMode == PreBoot){
    PanelMode = Boot_State;
  }else if(!PwrBtn() && PLC1_RDY){
    if(!GrnFlash.isRunning()){
      GrnFlash.start(LOW);
      GrnFlash.setInterval(2000);
    }
    PanelMode = Shutdown_State;
  }
  
  if(MNT_MODE_ACTIVE && !FAULTED){
    PanelMode = Maint_State;
  }
  
  if(FAULTED){
    if(GrnFlash.isRunning()){
      GrnFlash.stop(GRN_PL);
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