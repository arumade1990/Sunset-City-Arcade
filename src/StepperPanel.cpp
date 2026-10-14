//This code is for the Stepper Panel for the Sunset City Arcade Project
//Andy Rumade - 9/8/26

#include <Arduino.h>
#include <Timer.h>
#include <ToggleFlipflop.h>
#include <Flasher.h>
#include <OneShotPulse.h>
#include <TimerON.h>
#include <TimerOFF.h>

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
const int QX00 = 1; //should normally be 1 
const int QX01 = 2; 
const int QX02 = 3;
const int QX03 = 4;
const int QX04 = 5;
const int QX05 = 12;
const int QX06 = 13;
const int QX07 = 14;
const int QX10 = 15;
const int QX11 = 16; //should normally be 16

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

//Create general timer objects.
Timer HalfSecTimer(500);
Timer OneSecTimer(1000);
Timer ThreeSecTimer(3000);
Timer FiveSecTimer(5000);
Timer PwrDownTimer(3000);
Timer RotateCycleTimer(60000); //60 second timer for the screen to rotate in the event of an issue.
Timer ShutdownWaitTimer(180000); 
Timer VarTimer(1000);
Timer MaintTimer(5000);
Timer MaintTimer2(5000);
Timer CL86T_Reset_Timer(5000);

//Create the Special timers
OneShotPulse horzPulse;
OneShotPulse vertPulse;
TimerON CL86T_EN_DLY;

//Input bits with easier to read names.
bool VERT_CALL = LOW;
bool HORZ_CALL = LOW;
bool PWR_DN_TRGD = LOW;
bool SYS_RDY_IN = LOW;
bool STEPR_MV = LOW;
bool MAINT_KEY = LOW;
bool STEP_ALM = LOW;
bool SEQ_STOP_IN = LOW;

//Output bits with easier to read names.
bool CL86T_EN = LOW;
bool MAINT_MODE = LOW;
bool RDY_PWR_DN = LOW;
bool SYS_RDY = LOW;
bool STP_PNL_FLT = LOW;
bool SEQ_STOP = LOW;
bool PLC2_RDY = LOW;
bool VERT_SIG_OUT = LOW;
bool HORZ_SIG_OUT = LOW;
bool SPARE1 = LOW;

//Various memory bits.
bool HORZ_CALL_MEM = LOW;
bool VERT_CALL_MEM = LOW;
bool AUTO_MODE_OK = LOW;
bool SYS_RDY_MEM = LOW;
bool FAULTED = LOW;
bool MAINT_MODE_SUTDOWN = LOW;
bool PWR_DN_PRP = LOW;
bool VERT_SIG_OUT_MEM = LOW; 
bool HORZ_SIG_OUT_MEM = LOW;
bool CL86T_Flag = LOW;
bool STEPR_MV_MEM = LOW;
bool VERT_CYC_COMP = LOW;
bool HORZ_CYC_COMP = LOW;
bool HORZ_OUT_TRIG = LOW;
bool VERT_OUT_TRIG = LOW;
bool MaintVarSet = LOW;
bool GoHome = LOW;

//Program Flow Control Stuff
bool CL_DLY = LOW;

enum RunMode{
	Boot_State, //0
	Auto_State, //1 
	Maint_State, //2
	Fault_State,  //3
	Shutdown_State, //4
	Home_State //5
};

RunMode PanelMode = Boot_State; //Setup for initial start

void bootUpEN(){
	if(!ThreeSecTimer.isRunning()){
		ThreeSecTimer.start();
		CL86T_EN = HIGH;
	}else if(ThreeSecTimer.isReady()){
		ThreeSecTimer.stop();
		PanelMode = Auto_State;
	}
}

void readInputs(){

	IX00_ST = digitalRead(IX00); //VERTICAL CALL
	IX01_ST = digitalRead(IX01); //HORIZONTAL CALL
	IX02_ST = digitalRead(IX02); //POWER DOWN TRIGGERED
	IX03_ST = digitalRead(IX03); //SYSTEM READY (PROGRAM BOOTED)
	IX04_ST = digitalRead(IX04); //STEPPER MOVING (CMD RCV)
	IX05_ST = digitalRead(IX05); //MAINT KEY/MAINT MODE
	IX06_ST = digitalRead(IX06); //STEPPER ALARM
	IX07_ST = digitalRead(IX07); //SEQ STOP/SHUTDOWN
	IX10_ST = digitalRead(IX10); //SPARE
	
	VERT_CALL = IX00_ST;
	HORZ_CALL = IX01_ST;
	PWR_DN_TRGD = IX02_ST;
	SYS_RDY_IN = IX03_ST;
	STEPR_MV  = IX04_ST;
	MAINT_KEY = IX05_ST;
	STEP_ALM = IX06_ST;
	SEQ_STOP_IN = IX07_ST;
	//SPARE = IX10_ST;
	
}
 
void writeOutputs(){

	QX00_ST = CL86T_EN;
	QX01_ST = MAINT_MODE;
	QX02_ST = RDY_PWR_DN;
	QX03_ST = SYS_RDY_MEM;
    QX04_ST = STP_PNL_FLT;	
	QX05_ST = SEQ_STOP;
	QX06_ST = PLC2_RDY;
	QX07_ST = VERT_SIG_OUT;
	QX10_ST = HORZ_SIG_OUT;
    QX11_ST = SPARE1;
	
	digitalWrite(QX00, QX00_ST); //CR11 - CL86T ENABLE
	digitalWrite(QX01, QX01_ST); //CR14 - MAINT MODE
	digitalWrite(QX02, QX02_ST); //CR15-READY FOR POWER DOWN
	digitalWrite(QX03, QX03_ST); //SYSTEM READY/PC RUN
	digitalWrite(QX04, QX04_ST); //STEPPER PANEL FAULT
	digitalWrite(QX05, QX05_ST); //SEQUENCE STOP
	digitalWrite(QX06, QX06_ST); //PLC2 READY
	digitalWrite(QX07, QX07_ST); //VERTICAL SIGNAL OUT TO SMGP 5
	digitalWrite(QX10, QX10_ST); //HORIZONTAL SIGNAL OUT TO SMGP 6
	digitalWrite(QX11, QX11_ST); //SPARE
	
}

void RotateHorz(){

	if(!RotateCycleTimer.isRunning()){ //Start the cycle timer.
		RotateCycleTimer.start();
		STEPR_MV = LOW;
		HORZ_SIG_OUT_MEM = LOW;
		CL86T_Flag = LOW; 
		HORZ_CALL_MEM = HIGH;
		CL86T_EN_DLY.reset();
		horzPulse.reset();
	}

	if(STEPR_MV == LOW && HORZ_SIG_OUT_MEM == LOW && CL86T_Flag == LOW){ //Step 1 wait a set time before sending DIR signal to driver.
		CL86T_EN_DLY.update(CL86T_Flag, HIGH, CL86T_EN, HIGH, 1000); //TimerON
	}

	if(CL86T_Flag == HIGH){                                              //Step 2 Once the wait is done, send the rotate command as a set pulse.
		horzPulse.update(HORZ_SIG_OUT, HIGH, CL86T_Flag, HIGH, 1000); //OneShotPulse
	}     
		if(HORZ_SIG_OUT == HIGH){                                        //Step 3 Remember that the pulse was sent. 
			HORZ_SIG_OUT_MEM = HIGH;  
		}

	if(STEPR_MV == HIGH && HORZ_SIG_OUT_MEM == HIGH){                    //Step 4 Once the stepper movement is detected, remember that it was moving. 
		STEPR_MV_MEM = HIGH;
	}

	if(STEPR_MV == LOW && STEPR_MV_MEM == HIGH && HORZ_SIG_OUT == LOW && HORZ_SIG_OUT_MEM == HIGH){  //Step 5 If the stepper moved and the movement command happened,                                                                                               
		if(!OneSecTimer.isRunning()){                                                              //check if the movement happened and completed.
			OneSecTimer.start();
		}
	}

	if(OneSecTimer.isReady()){
		OneSecTimer.stop();
		HORZ_CYC_COMP = HIGH;
		VERT_CYC_COMP = LOW;
	}

	if(HORZ_CYC_COMP == HIGH){            //Step 6 Stop the cycle timer. 
		RotateCycleTimer.stop();
		STEPR_MV_MEM = LOW;
		HORZ_SIG_OUT_MEM = LOW;
		HORZ_CALL_MEM = LOW;
		CL86T_Flag = LOW;
		CL86T_EN_DLY.reset();
		horzPulse.reset();
	}

	if(RotateCycleTimer.isReady()){ //Step 0 If the cycle timer ever finishes, something is wrong. 
		STP_PNL_FLT = HIGH;
		RotateCycleTimer.stop();
		OneSecTimer.stop();
		STEPR_MV_MEM = LOW;
		HORZ_SIG_OUT_MEM = LOW;
		HORZ_CALL_MEM = LOW;
		CL86T_EN = LOW;
		CL86T_Flag = LOW;
	}
}

void RotateVert(){

	if(!RotateCycleTimer.isRunning()){ //Start the cycle timer.
		RotateCycleTimer.start();
		STEPR_MV = LOW;
		VERT_SIG_OUT_MEM = LOW;
		CL86T_Flag = LOW;
		VERT_CALL_MEM = HIGH; 
		CL86T_EN_DLY.reset();
		vertPulse.reset();
	}

	if(STEPR_MV == LOW && VERT_SIG_OUT_MEM == LOW && CL86T_Flag == LOW){ //Step 1 wait a set time before sending DIR signal to driver.
		CL86T_EN_DLY.update(CL86T_Flag, HIGH, CL86T_EN, HIGH, 1000);
	}

	if(CL86T_Flag == HIGH){                                              //Step 2 Once the wait is done, send the rotate command as a set pulse pulse.
		vertPulse.update(VERT_SIG_OUT, HIGH, CL86T_Flag, HIGH, 1000);
	}     
		if(VERT_SIG_OUT == HIGH){                                        //Step 3 Remember that the pulse was sent. 
			VERT_SIG_OUT_MEM = HIGH;  
		}

	if(STEPR_MV == HIGH && VERT_SIG_OUT_MEM == HIGH){                    //Step 4 Once the stepper movement is detected, remember that it was moving. 
		STEPR_MV_MEM = HIGH;
	}

	if(STEPR_MV == LOW && STEPR_MV_MEM == HIGH && VERT_SIG_OUT == LOW && VERT_SIG_OUT_MEM == HIGH){  //Step 5 If the stepper moved and the movement command happened,  
		if(!OneSecTimer.isRunning()){                                                              //check if the movement happened and completed.
			OneSecTimer.start();
		}
	}

	if(OneSecTimer.isReady()){
		OneSecTimer.stop();
		HORZ_CYC_COMP = LOW;
		VERT_CYC_COMP = HIGH;
	}

	if(VERT_CYC_COMP == HIGH){            //Step 6 Stop the cycle timer. 
		RotateCycleTimer.stop();
		OneSecTimer.stop();
		STEPR_MV_MEM = LOW;
		VERT_SIG_OUT_MEM = LOW;
		VERT_CALL_MEM = LOW;
		CL86T_Flag = LOW;
		CL86T_EN_DLY.reset();
		vertPulse.reset();
		return;

	}

	if(RotateCycleTimer.isReady()){ //Step 0 If the cycle timer ever finishes, something is wrong. 
		STP_PNL_FLT = HIGH;
		RotateCycleTimer.stop();
		OneSecTimer.stop();
		STEPR_MV_MEM = LOW;
		VERT_SIG_OUT_MEM = LOW;
		VERT_CALL_MEM = LOW;
		CL86T_EN = LOW;
		CL86T_Flag = LOW;
		return;
	}
}

void VarReset(bool reset = false){

	if(reset){
		MaintVarSet = false;
	}

	if(!MaintVarSet){
		RotateCycleTimer.stop();
		OneSecTimer.stop();
		CL86T_EN_DLY.reset();
		vertPulse.reset();
		horzPulse.reset();
		STEPR_MV_MEM = LOW;
		CL86T_Flag = LOW;
		VERT_CYC_COMP = LOW;
		VERT_SIG_OUT_MEM = LOW;
		VERT_CALL_MEM = LOW;
		HORZ_CYC_COMP = LOW;
		HORZ_SIG_OUT_MEM = LOW;
		HORZ_CALL_MEM = LOW;

		MaintVarSet = HIGH;
	}
	
}

void MaintMode(){
	if(MAINT_KEY && !FAULTED){
		VarReset();         //Runs ones from the reset rvaiable. 
		MAINT_MODE = HIGH;
	}else if(FAULTED){
		PanelMode = Fault_State;
		return;
	}else if(!MAINT_KEY){
		MaintVarSet = LOW;
		MAINT_MODE = LOW;
		PanelMode = Auto_State;
		return;
	}
}

void AutoMode(){

	bool isBusy = (HORZ_CALL_MEM || VERT_CALL_MEM);

	if(!MAINT_KEY && !FAULTED && CL86T_EN){
		
		//System ready checks
		if(SYS_RDY_IN && !PWR_DN_TRGD && !SEQ_STOP_IN && !RDY_PWR_DN){
			AUTO_MODE_OK = HIGH;
			SYS_RDY_MEM = HIGH;
			SYS_RDY = SYS_RDY_MEM;
		}else if(!SYS_RDY_IN && SYS_RDY_MEM){ 
			AUTO_MODE_OK = HIGH; 
		}else{
			AUTO_MODE_OK = LOW;
		}

		if(!STP_PNL_FLT && AUTO_MODE_OK) {
			
			// 1. CHOOSE AN ACTION (Only allowed if the system is NOT busy!)
			if(!isBusy) {
				if(HORZ_CALL && !VERT_CALL && !HORZ_CYC_COMP) {
					HORZ_CALL_MEM = HIGH;
					VERT_CYC_COMP = LOW; // Prep the opposite flag for next time
				}
				else if(VERT_CALL && !HORZ_CALL && !VERT_CYC_COMP) {
					VERT_CALL_MEM = HIGH;
					HORZ_CYC_COMP = LOW; // Prep the opposite flag for next time
				}
			}

			if(HORZ_CALL_MEM) {
				RotateHorz();
			} 
			else if(VERT_CALL_MEM) {
				RotateVert();
			}
		}

	}else if(MAINT_KEY){
		PanelMode = Maint_State;
		return;
	}else if(FAULTED){
		PanelMode = Fault_State;
		return;
	}else if(SEQ_STOP){
		return;
	}else if(!CL86T_EN){
		CL86T_EN = HIGH;
		if(!CL86T_Reset_Timer.isRunning()){
			CL86T_Reset_Timer.start();
		}
		while(CL86T_Reset_Timer.isRunning()){
			readInputs();
			if(CL86T_Reset_Timer.isReady()){
				CL86T_Reset_Timer.stop();
				break;
			}

			isBusy = (HORZ_CALL_MEM || VERT_CALL_MEM);

			if(!isBusy) {
				if(HORZ_CALL && !VERT_CALL && !HORZ_CYC_COMP) {
					HORZ_CALL_MEM = HIGH;
					VERT_CYC_COMP = LOW; // Prep the opposite flag for next time
				}
				else if(VERT_CALL && !HORZ_CALL && !VERT_CYC_COMP) {
					VERT_CALL_MEM = HIGH;
					HORZ_CYC_COMP = LOW; // Prep the opposite flag for next time
				}
			}

			writeOutputs();
		}
	}
}

void HomeMode(){
	bool isBusy = (HORZ_CALL_MEM || VERT_CALL_MEM);

	if(PWR_DN_TRGD && !HORZ_CYC_COMP){
		// 1. CHOOSE AN ACTION (Only allowed if the system is NOT busy!)
		if(!isBusy){
			if(HORZ_CALL && !VERT_CALL && !HORZ_CYC_COMP){
				HORZ_CALL_MEM = HIGH;
				VERT_CYC_COMP = LOW; // Prep the opposite flag for next time
			}
			else if(VERT_CALL && !HORZ_CALL && !VERT_CYC_COMP){
				VERT_CALL_MEM = HIGH;
				HORZ_CYC_COMP = LOW; // Prep the opposite flag for next time
			}
		}

		if(HORZ_CALL_MEM){
			RotateHorz();
		} 
		else if(VERT_CALL_MEM){
			RotateVert();
		}
	}
	
	if(!VarTimer.isRunning() && !HORZ_CALL_MEM){
		VarTimer.setInterval(30000);
		VarTimer.start();
	}else if(HORZ_CALL_MEM){
		VarTimer.stop();
	}

	if(VarTimer.isReady()){
		HORZ_CALL_MEM = HIGH;
	}

	if(HORZ_CYC_COMP){
		VarTimer.stop();
		PanelMode = Shutdown_State;
		return;
	}
}

void ShutdownInit(){
	
	if(HORZ_CYC_COMP && PWR_DN_TRGD && !SYS_RDY_IN && !PWR_DN_PRP && !PwrDownTimer.isRunning()){
		SYS_RDY_MEM = LOW;
		PWR_DN_PRP = HIGH;
		PwrDownTimer.start();
	}
	
	if(PWR_DN_PRP && PwrDownTimer.isReady() && !ShutdownWaitTimer.isRunning()){
		RDY_PWR_DN = HIGH;
		PwrDownTimer.stop();
		ShutdownWaitTimer.start();
	}

	if(!MAINT_KEY && RDY_PWR_DN){
			
		if(ShutdownWaitTimer.isReady()){
			STP_PNL_FLT = HIGH;
			FAULTED = STP_PNL_FLT;
			ShutdownWaitTimer.stop();
			PanelMode = Fault_State;
			return;
		}
	} 
}

void FaultMode(){
	if(MAINT_KEY && FAULTED){
		if(!FiveSecTimer.isRunning()){
			AUTO_MODE_OK = LOW;
			FiveSecTimer.start();
	}

		if(FiveSecTimer.isReady()){
			STP_PNL_FLT = LOW;
			FAULTED = LOW;
			PanelMode = Maint_State;
			FiveSecTimer.stop();
			return;
		}
	}else if(!MAINT_KEY && FAULTED){
		FiveSecTimer.stop();
		PanelMode = Fault_State;
		return;
	}
}

void setup(){
  //Set the input pins as inputs. The boards use 10k pull down resistors.
  pinMode(IX00, INPUT); //VERTICAL CALL
  pinMode(IX01, INPUT); //HORIZONTAL CALL
  pinMode(IX02, INPUT); //POWER DOWN TRIGGERED
  pinMode(IX03, INPUT); //SYSTEM READY (PROGRAM BOOTED)
  pinMode(IX04, INPUT); //STEPPER MOVING (CMD RCV)
  pinMode(IX05, INPUT); //MAINT KEY/MAINT MODE
  pinMode(IX06, INPUT); //STEPPER ALARM
  pinMode(IX07, INPUT); //SEQ STOP/SHUTDOWN
  pinMode(IX10, INPUT); //SPARE
  
  //Set the output pins as outputs. The boards also use 10k pull down resistors on these too. 
  pinMode(QX00, OUTPUT); //CR11 - CL86T ENABLE
  pinMode(QX01, OUTPUT); //CR14 - MAINT MODE
  pinMode(QX02, OUTPUT); //CR15-READY FOR POWER DOWN
  pinMode(QX03, OUTPUT); //SYSTEM READY/PC RUN
  pinMode(QX04, OUTPUT); //STEPPER PANEL FAULT
  pinMode(QX05, OUTPUT); //SEQUENCE STOP
  pinMode(QX06, OUTPUT); //PLC2 READY 
  pinMode(QX07, OUTPUT); //VERTICAL SIGNAL OUT TO SMGP 5
  pinMode(QX10, OUTPUT); //HORIZONTAL SIGNAL OUT TO SMGP 6
  pinMode(QX11, OUTPUT); //SPARE
  
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++LOOP BEGIN
void loop(){
	delay(10); //Scan time
	readInputs(); //This should always be at the top.

	if(!MAINT_KEY && !FAULTED){
		switch (PanelMode)
		{
		case Boot_State: //boot mode
			PLC2_RDY = HIGH;
			bootUpEN();
			break;
		case Auto_State: //auto mode
			AutoMode();
			break;
		case Maint_State: //maint mode
			MaintMode();
			break;
		case Fault_State: //fault mode
			FaultMode();
			break;
		case Shutdown_State: //shutdown
			ShutdownInit();
			break;
		case Home_State:
			HomeMode(); //In-between mode to ensure the screen gets to horizontal position before shutting down. 
			break;
		default:
			break;
		}
	}else if(MAINT_KEY && !FAULTED){
		PanelMode = Maint_State;
	}else if(FAULTED){
		PanelMode = Fault_State;
	}
	
	if(SEQ_STOP && !HORZ_CYC_COMP){
		PanelMode = Home_State;
	}else if(SEQ_STOP && HORZ_CYC_COMP){
		PanelMode = Shutdown_State;
	}

	if(SEQ_STOP_IN && !SEQ_STOP){
		SEQ_STOP = HIGH;
	}
	SPARE1 = AUTO_MODE_OK;

	writeOutputs();
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++LOOP END
