//This code is for the Power Sequence Panel for the Sunset City Arcade Project
//Andy Rumade - 9/8/26

#include <Arduino.h>

//Define the INPUT pins.
const int IX00 = 17;
const int IX01 = 18;
const int IX02 = 19;
const int IX03 = 21;
const int IX04 = 22;
const int IX05 = 23;
const int IX06 = 27;
const int IX07 = 32;
const int IX10 = 33;

//Create states for the input pins.
bool IX00_ST = LOW;
bool IX01_ST = LOW;
bool IX02_ST = LOW;
bool IX03_ST = LOW;
bool IX04_ST = LOW;
bool IX05_ST = LOW;
bool IX06_ST = LOW;
bool IX07_ST = LOW;
bool IX10_ST = LOW;

//Define the OUTPUT pins.
const int QX00 = 1;
const int QX01 = 2;
const int QX02 = 3;
const int QX03 = 4;
const int QX04 = 5;
const int QX05 = 12;
const int QX06 = 13;
const int QX07 = 14;
const int QX10 = 15;
const int QX11 = 16;

//Create states for the output pins.
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


//Define the ANALOG INPUT pins.
const int IW00 = 34;
const int IW01 = 35;
const int IW02 = 36;
const int IW03 = 39;

//Define the ANALOG OUTPUT pins.
const int QW00 = 25;
const int QW01 = 26;

//Create bits for easier to read code
bool PWR_PB = LOW;
bool SEQ_START = LOW;
bool SEQ_STOP = LOW;
bool MNT_MODE_ACTIVE = LOW;
bool MNT_MODE_PB = LOW;
bool _60VDC_OK = LOW;
bool SYS_RDY = LOW;
bool SHUTDOWN_RDY = LOW;
bool FRC_SHUTDOWN = LOW;
bool PLC1_RDY = LOW;
bool GRN_PL = LOW;
bool RED_PL = LOW;
bool MC2_PWR = LOW;
bool SYS_RDY_OUT = LOW;
bool BOTH_PLC_RDY = LOW;
bool PC_PWR_PLS = LOW;
bool TOTAL_PWR_OFF = LOW;

//Memory bits and such
bool PWR_PB_MEM = LOW;
bool SYS_RDY_MEM = LOW;
bool PC_PWR_UP_FAILED = LOW;
bool SEQ_START_MEM = LOW;
bool SEQ_STOP_MEM = LOW;
bool PWR_DOWN_INIT = LOW;
bool PWR_PB_COOLDOWN = LOW;
bool RED_LT_ST = LOW;
bool GRN_LT_ST = LOW;
bool TEN_SEC_STRT = LOW;
bool ATMPT_PWR = LOW;
bool STP_PNL_FLT = LOW;

//Bits to use as flags for various things
bool FLAG_A = LOW;
bool FLAG_B = LOW;
bool FLAG_C = LOW;

//Power Up Attempts
int PWR_UP_TRY = 0;

//Create a timer class for a reusable timer setup. 
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

    // Start or restart the timer
    void start() {
      previousMillis = millis();
      running = true;
    }

    // Stop the timer
    void stop() {
      running = false;
    }

    // Change interval dynamically
    void setInterval(unsigned long newInterval) {
      interval = newInterval;
    }

    // Check if the timer has expired
    bool isReady() {
      if (!running) return false;
      
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; // Reset for repeating timer
        return true;
      }
      return false;
    }
	
	bool oneShotReady(){
	  if (!running) return false;

	  unsigned long currentMillis = millis();
	  if (currentMillis - previousMillis >= interval){
		return true;
	  }
	  return false;
	}
};

//Create general timer objects.
Timer DeBounceTimer(50); //Might not be used.
Timer HalfSecTimer(500);
Timer OneSecTimer(1000);
Timer FiveSecTimer(5000);
Timer TenSecTimer(10000);

//Specific Timers.
Timer PcPowerPulse(1000);
Timer SysRdyDly(3000);
Timer PcPowerDly(2000);
Timer PcPulseCycleTimer(600000); //10 Minutes
Timer ThirtySecTimer(30000); //30 seconds
Timer GrnTimer(1000);
Timer RedTimer(1000);

//Variable timer
Timer VarTimer(1000); //1 sec used as default. 

//===================================================================Function declaration begin.

void readInputs(){
	IX00_ST = digitalRead(IX00); //ON PB
	IX01_ST = digitalRead(IX01); //PLC 1+2 READY
	IX02_ST = digitalRead(IX02); //PC RUN FROM PLC 2 (FT232H)
	IX03_ST = digitalRead(IX03); //_60VDC OK
	IX04_ST = digitalRead(IX04); //MAINT MODE KEY
	IX05_ST = digitalRead(IX05); //SHUTDOWN READY FROM PLC 2
	IX06_ST = digitalRead(IX06); //MAINT PB
	IX07_ST = digitalRead(IX07); //STEPPER PANEL FAULT
	IX10_ST = digitalRead(IX10); //SPARE

	//Copy really important values to tags with better names.
	PWR_PB = IX00_ST;
	BOTH_PLC_RDY = IX01_ST;
	SYS_RDY = IX02_ST;
	_60VDC_OK = IX03_ST;
	MNT_MODE_ACTIVE = IX04_ST;
	SHUTDOWN_RDY = IX05_ST;
	MNT_MODE_PB = IX06_ST;
	STP_PNL_FLT = IX10_ST;
}

void writeOutputs(){

	QX00_ST = SEQ_START_MEM;
	QX01_ST = SEQ_STOP_MEM;
	QX02_ST = PLC1_RDY;
	QX03_ST = GRN_PL;
	QX04_ST = RED_PL;
	QX05_ST = MC2_PWR;
	QX06_ST = SYS_RDY_OUT;
	QX07_ST = TOTAL_PWR_OFF;
	QX10_ST = PC_PWR_PLS;

	digitalWrite(QX00, QX00_ST); //CR3 - SEQUENCE START
	digitalWrite(QX01, QX01_ST); //CR4 - SEQUENCE STOP
	digitalWrite(QX02, QX02_ST); //CR2 - PLC 1 READY
	digitalWrite(QX03, QX03_ST); //CR5 - GREEN PL
	digitalWrite(QX04, QX04_ST); //CR6 - RED PL
	digitalWrite(QX05, QX05_ST); //CR7 - 60VDC AND PC POWER
	digitalWrite(QX06, QX06_ST); //CR8 - SYSTEM READY 
	digitalWrite(QX07, QX07_ST); //CR9 - TOTAL POWER OFF
	digitalWrite(QX10, QX10_ST); //CR10 - PC POWER PULSE
	digitalWrite(QX11, QX11_ST); //CR10.5? - DISPLAY AND AUDIO POWER CONACTOR
}

void maintMode(){
	while (MNT_MODE_ACTIVE == HIGH){
		readInputs();
		if (MNT_MODE_ACTIVE == HIGH){
			/*
			//
			//
			//Maint Mode code goes here
			//
			//
			*/
		}
		else {
			writeOutputs();
			break;
		}
		writeOutputs();
	}
}

void GrnLtFnc(unsigned long duration, bool G_active){
	static bool GtimerStarted = LOW;
	if(GtimerStarted == LOW && G_active == HIGH){
		GrnTimer.setInterval(duration);
		GrnTimer.start();
		GtimerStarted = HIGH;
	}
	if(GrnTimer.isReady() && RED_LT_ST == LOW && MNT_MODE_ACTIVE == LOW && G_active == HIGH){
	  GRN_PL = !GRN_PL;
		GRN_LT_ST = HIGH;
	}else if(G_active == LOW){
		GrnTimer.stop();
		GRN_PL = LOW;
		GRN_LT_ST = LOW;
		GtimerStarted = LOW;
	}
}

void RedLtFnc(unsigned long duration, bool R_active){
	static bool RtimerStarted = LOW;
	if(RtimerStarted == LOW && R_active == HIGH){
		RedTimer.setInterval(duration);
		RedTimer.start();
		RtimerStarted = HIGH;
	}
	if(RedTimer.isReady() && GRN_LT_ST == LOW && R_active == HIGH){
		RED_PL = !RED_PL;
		RED_LT_ST = HIGH;
	}else if(R_active == LOW){
		RedTimer.stop();
		RED_PL = LOW;
		RED_LT_ST = LOW;
		RtimerStarted = LOW;
	}
}

void PcPwrOn(){
	GrnLtFnc(1000, LOW);
	RedLtFnc(1000, LOW);
	while(MC2_PWR == HIGH && SYS_RDY_MEM == LOW && PC_PWR_UP_FAILED == LOW && MNT_MODE_ACTIVE == LOW){
		GrnLtFnc(500, HIGH);
		readInputs();
		if(_60VDC_OK == HIGH && SYS_RDY == LOW && FLAG_A == LOW){
			PcPowerDly.start();
			FLAG_A = HIGH;
		}
		if(PcPowerDly.isReady() && FLAG_B == LOW){
			PcPowerDly.stop();
			PC_PWR_PLS = HIGH;
			FLAG_B = HIGH;
			PcPowerPulse.start();
		}
		if(FLAG_A == HIGH && FLAG_B == HIGH && FLAG_C == LOW && PcPowerPulse.isReady()){
			PC_PWR_PLS = LOW;
			PcPowerPulse.stop();
			PcPulseCycleTimer.start();
			FLAG_C = HIGH;
		}
		if(PcPulseCycleTimer.isReady() && SYS_RDY == LOW && PWR_UP_TRY <= 2){
			FLAG_A = LOW;
			FLAG_B = LOW;
			FLAG_C = LOW; 
			PWR_UP_TRY++;
		}else if(SYS_RDY == HIGH){
			SYS_RDY_MEM = HIGH;
			PcPulseCycleTimer.stop();
			PWR_UP_TRY = 0;
			writeOutputs();
			break;
		}
		if(_60VDC_OK == HIGH && SYS_RDY == LOW && PWR_UP_TRY >= 3){
			PC_PWR_UP_FAILED = HIGH;
			writeOutputs();
			GrnLtFnc(500, LOW);
			writeOutputs();
			break;
		}
		writeOutputs();
	}	
}

void PcPwrOnFail(){
	RedLtFnc(500, LOW);
	while(MNT_MODE_ACTIVE == LOW && PC_PWR_UP_FAILED == HIGH){
		RedLtFnc(500, HIGH);
		readInputs();
		RedLtFnc(500, HIGH);
		if(MNT_MODE_ACTIVE == HIGH && MNT_MODE_PB == HIGH){
			RedLtFnc(500, LOW);
			delay(5000);
			PC_PWR_UP_FAILED = LOW;
			writeOutputs();
			break;	
		}
		writeOutputs();
	}
}

void PcPwrOff(){
	while(PWR_DOWN_INIT == HIGH){
		readInputs();
		if(SHUTDOWN_RDY == HIGH && SYS_RDY == LOW && MNT_MODE_ACTIVE == LOW){
			TOTAL_PWR_OFF = HIGH;
		}
		writeOutputs();
	}
}

//===================================================================Function declaration end.

void setup(){
	//Set the input pins as inputs. The boards use 10k pull down resistors.
	pinMode(IX00, INPUT); //ON PB
	pinMode(IX01, INPUT); //PLC 1+2 READY
	pinMode(IX02, INPUT); //PC RUN FROM PLC 2 (FT232H)
	pinMode(IX03, INPUT); //_60VDC OK
	pinMode(IX04, INPUT); //MAINT MODE KEY
	pinMode(IX05, INPUT); //SHUTDOWN READY FROM PLC 2
	pinMode(IX06, INPUT); //MAINT PB
	pinMode(IX07, INPUT); //SPARE
	pinMode(IX10, INPUT); //SPARE

	//Set the output pins as outputs. The boards also use 10k pull down resistors on these too. 
	pinMode(QX00, OUTPUT); //CR3 - SEQUENCE START
	pinMode(QX01, OUTPUT); //CR4 - SEQUENCE STOP
	pinMode(QX02, OUTPUT); //CR2 - PLC 1 READY
	pinMode(QX03, OUTPUT); //CR5 - GREEN PL
	pinMode(QX04, OUTPUT); //CR6 - RED PL
	pinMode(QX05, OUTPUT); //CR7 - 60VDC AND PC POWER
	pinMode(QX06, OUTPUT); //CR8 - SYSTEM READY 
	pinMode(QX07, OUTPUT); //CR9 - TOTAL POWER OFF
	pinMode(QX10, OUTPUT); //CR10 - PC POWER PULSE 
	pinMode(QX11, OUTPUT); //CR10.5? - DISPLAY AND AUDIO POWER CONACTOR

	//Start the timers.
	DeBounceTimer.start();
	HalfSecTimer.start();
	OneSecTimer.start();
	TenSecTimer.start();
}

void loop(){
	delay(10); //Scan time
	readInputs();
	PLC1_RDY = HIGH; //PLC READY
	
	//===================================================================Fault handler logic begin.
	while(STP_PNL_FLT == HIGH){
		RedLtFnc(500,STP_PNL_FLT);
		if(STP_PNL_FLT == LOW){
			break;
		}
	}
	//===================================================================Fault handler logic end.
	//===================================================================Power button handler logic begin.
	if(PWR_DOWN_INIT == LOW && PWR_PB == HIGH && PWR_PB_MEM == LOW){
		PWR_PB_MEM = HIGH;
		FiveSecTimer.start(); //Start the 5 second timer.
		ThirtySecTimer.start();
	}
	if(SEQ_STOP_MEM ==  LOW && PWR_PB_MEM == HIGH && MNT_MODE_ACTIVE == LOW && FiveSecTimer.isReady() && SEQ_START == LOW ){
		PWR_PB_MEM = LOW;
		SEQ_START = HIGH; //Indicates initilization of system startup.(Trying to power up system.)	
		FiveSecTimer.stop();
	}else if(SEQ_START == HIGH && SEQ_START_MEM == HIGH && SYS_RDY_MEM == HIGH && PWR_PB_MEM == HIGH){
		SEQ_START = LOW;
	}
	if(SEQ_START_MEM == LOW && SEQ_START == HIGH && SYS_RDY_MEM == HIGH){
		SEQ_START_MEM = HIGH; //System successfully started.
	}
	//===================================================================Power button handler logic end. 
	//===================================================================MC2 Energize begin.
	if(SEQ_START == HIGH && PLC1_RDY == HIGH){
		MC2_PWR = HIGH;
	}
	if(MC2_PWR == HIGH && BOTH_PLC_RDY == HIGH){
		MC2_PWR = HIGH; //Set the output high as a seal-in.
	}
	//===================================================================MC2 Energize end.
	//===================================================================Green light indicator begin. 
	if(SEQ_START = HIGH && SEQ_START_MEM == LOW){
		GrnLtFnc(1000, HIGH);
		if(TEN_SEC_STRT = LOW){
			TenSecTimer.start();
		}
	}else if(SEQ_START_MEM = HIGH){
		GrnLtFnc(1000, LOW);
		GRN_PL = SEQ_START_MEM;
	}
	
	//==================================================================Green light indicator end.
	//==================================================================PC power pulse begin.
	if((MC2_PWR == HIGH && ATMPT_PWR == LOW && TenSecTimer.isReady()) || ATMPT_PWR == HIGH){
		ATMPT_PWR = HIGH;
		TenSecTimer.stop(); 
		PcPwrOn();
	}
	//==================================================================PC power pulse end. 
	//==================================================================PC power up failure condition handler begin.
	if(PC_PWR_UP_FAILED == HIGH){
		PcPwrOnFail();
	}
	//==================================================================PC power up failure condition handler end.
	//==================================================================PC power off begin.
	if(ThirtySecTimer.isReady() || PWR_PB_COOLDOWN == HIGH){
		PWR_PB_COOLDOWN = HIGH;
		if(PWR_DOWN_INIT == LOW && PWR_PB_MEM == HIGH && SEQ_START_MEM == HIGH && PWR_PB_COOLDOWN == HIGH)
			PWR_DOWN_INIT = HIGH;
			ThirtySecTimer.stop();
			PcPwrOff();
	}
	//==================================================================PC Power off end.
	writeOutputs();
	maintMode();
}