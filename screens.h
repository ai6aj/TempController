#ifndef __SCREENS_H__
#define __SCREENS_H__
#include <EEPROM.h>
#include <ctype.h>
#include "format.h"

#define EEPROM_E6_ROLL_COUNT  0

class Timer {
    public:
      void setCountdownTime(long millis);
      void start();
      long getElapsedTime();
      long getRemainingTime();
      long timeStarted;
      long timeFinished;
      long countdownTime;
};

void Timer::setCountdownTime(long countdown) {
  countdownTime = countdown;
}

void Timer::start() {
  timeStarted = millis();
  timeFinished = timeStarted+countdownTime;
}

long Timer::getElapsedTime() {
  return millis() - timeStarted;
}

long Timer::getRemainingTime() {
  long rt = timeFinished - millis();
  if (rt < 0) return 0;
  return rt;
}



class Screen {
	public:
		Screen();
		virtual Screen* next()=0;
		virtual void up()=0;
		virtual void down()=0;				
		virtual void display()=0;				
		virtual Screen* switchTo(Screen* from);				
		virtual Screen* back();	
    virtual int loop();        

	protected:
		Screen* prevScreen;					
};

Screen::Screen() {
	prevScreen = NULL;
}

Screen* Screen::switchTo(Screen* from) {
	prevScreen = from;
	return this;
}

int Screen::loop() {
  return 0;
}

Screen* Screen::back() {
	if (prevScreen != NULL) return prevScreen;
	return this;
}

class ChoiceScreen:public Screen {
	public:
		ChoiceScreen(const char* title,const char** screenItems,void** dataItems);
		
		const char *title;
		const char **items;
		void** data;
		int maxItems;
		int choice;
		
		virtual Screen* next();
		virtual void up();
		virtual void down();				
		virtual void display();				
		virtual void init(Screen* from);				
};

ChoiceScreen::ChoiceScreen(const char* sTitle,
							const char** screenItems,
							void** dataItems) {
	items = screenItems;
	data = dataItems;
	title = sTitle;
	
	int len=0;
	while (items[len] != NULL) len++;
	maxItems = len;
	choice = 0;
}

void ChoiceScreen::up() {
	choice++;
	if (choice >= maxItems) 
		{ choice -= maxItems; }
}

void ChoiceScreen::down() {
	choice--;
	if (choice < 0 ) 
		{ choice += maxItems; }
}

Screen* ChoiceScreen::next() {
	Screen* nextScreen = (Screen*)data[choice];
	if (nextScreen != NULL) {
		nextScreen->switchTo(this);
		return (Screen*)data[choice];
	}
	return this;
}

void ChoiceScreen::display() {
	lcd.setCursor(0,0);
  lcd.print(title);
	lcd.setCursor(0,1);
  lcd.print(items[choice]);
}

void ChoiceScreen::init(Screen* prevScreen) {
	if (prevScreen != this) choice = 0;
}


#define PROCESS_C41  0
#define PROCESS_C41R  1
#define PROCESS_E6  2

class ColorExposureScreen:Screen {
  public:
    ColorExposureScreen(int colorMode);
    int stops;
    int process;
    virtual Screen* next();
    virtual void up();
    virtual void down();        
    virtual void display();
};

class C41RFirstDeveloperScreen:Screen {
  public:
    C41RFirstDeveloperScreen(int);
    int choice;
    virtual Screen* next();
    virtual void up();
    virtual void down();        
    virtual void display();    
};

class ColorStandbyScreen:public Screen {
  public:
    virtual Screen* next();
    virtual void up();
    virtual void down();        
    virtual void display();       

    // Called when navigating forward 
    Screen* switchTo(Screen* previous, int nProcess, int nStops, int fdTime, int rlComp, int tTemp);       

    // Called when develop mode cancelled
    Screen* switchTo(Screen* previous);
    virtual Screen* back();       
    virtual int loop();
    
  protected:
    int process;
    int stops;
    int lastTime = 0;
    long firstDevTime = 0;
    long targetTemp = 960;
    int rollComp;
    Timer timer;    
};

#define C41_PRESOAK 0
#define C41_DEVELOP 1
#define C41_BLIX 2
#define C41_WASH 3
#define C41_STABILIZE 4
#define C41_DONE 5

#define C41R_PRESOAK 0
#define C41R_FIRST_DEVELOP 1
#define C41R_STOP 2
#define C41R_REEXPOSE 3
#define C41R_COLOR_DEVELOP 4
#define C41R_BLIX 5
#define C41R_DONE 6

#define E6_PRESOAK 0
#define E6_FIRST_DEVELOP 1
#define E6_STOPWASH 2
#define E6_COLOR_DEVELOP 3
#define E6_CDWASH 4
#define E6_BLIX 5
#define E6_DONE 6

/* --------------------------------------------------------------------------------------
 *  COLOR DEVELOP SCREEN
 * --------------------------------------------------------------------------------------
 */

class ColorDevelopScreen:public ColorStandbyScreen {
  public:
    virtual Screen* next();
    virtual void display();       
    Screen* switchTo(Screen* previous, int nProcess, int nStops, int firstDevTime, int rlComp, int targetTemp);       
    virtual int loop();    
    virtual Screen* back();
  
  protected:
    virtual void init();
    virtual void startStep(int step);
    const char* processName;
    const char* stepName;
    int done;
    int cancelRequested;
    int triggerEvent;
    long processStep;
    long maxProcessStep;
    long presoakTime;
    long developTime;
    long firstDevelopTime;
    int rollComp; // For E-6... this will get written to EEPROM once we get to BLIX
    long reexposeTime;
    long stopTime;
    long blixTime;
    long washTime;
    long targetTemp;
    long stabilizeTime;
};

Screen* ColorDevelopScreen::back() {
  if (!cancelRequested) {
    cancelRequested=1;
    return this;
  }

  prevScreen->switchTo(NULL);
  return prevScreen;
}


Screen* ColorDevelopScreen::next() {
  return this;
}

int ColorDevelopScreen::loop() {
  int haveEvent = 0;
  long timeRemaining;

  if (!done) {
    timeRemaining = timer.getRemainingTime();
    if (timeRemaining == 0) {
      processStep++;
      startStep(processStep);
      timeRemaining = timer.getRemainingTime();
      haveEvent = 1;      
    }
    timeRemaining /= 1000;
  }
  else {
    timeRemaining = timer.getElapsedTime() / 1000;
  }

  if (timeRemaining != lastTime) haveEvent = 1;
  lastTime = timeRemaining;
  
  return haveEvent;
}

Screen* ColorDevelopScreen::switchTo(Screen* previous,int nProcess, int nStops, int fdTime, int rlComp, int tTemp) {
  stops = nStops; 
  process = nProcess;
  firstDevelopTime = fdTime;
  targetTemp = tTemp;
  rollComp = rlComp;
  init();
  return Screen::switchTo(previous);
}

void ColorDevelopScreen::init() {
  cancelRequested=0;
  switch(process) {
     case PROCESS_C41:
        processName = "C-41";
        maxProcessStep = C41_DONE;
        presoakTime = 60;
        if (stops==1) {
          // Commented out times are for 102F
          // current times are for 95F
          //developTime = 262;
          developTime = 430;
        } else if (stops==2) {
          //developTime = 315;
          developTime = 520;
        } else {
          //developTime = 210;          
          developTime = 345;
        }
        // blixTime = 390;
        blixTime = 420;
        washTime = 180;
        stabilizeTime = 60;        
     break;

     case PROCESS_C41R:
        processName = "C-41R";
        maxProcessStep = C41R_DONE;
        presoakTime = 60;

        if (targetTemp < 970) {
          developTime = 345;          
        } else {
          developTime = 5*60;
        }
        reexposeTime = 300;
        stopTime = 60;
        blixTime = 600;        
        washTime = 180;
        stabilizeTime = 60;        
        break;

     case PROCESS_E6:
        processName = "E6";
        maxProcessStep = E6_DONE;
        presoakTime = 60;

        float fdtTmp = 390;
        for (int i=0; i<rollComp; i++) { fdtTmp *= 1.04; }
        firstDevelopTime = (int)fdtTmp;
        developTime = 270;          
        blixTime = 600;        
        stopTime = 120;
        washTime = 180;
        stabilizeTime = 60;        

        break;
  }
  startStep(0);
}

void ColorDevelopScreen::startStep(int step) {
  
  if (process == PROCESS_C41) {
      done = 0;
      switch (processStep) {
        case C41_PRESOAK:
          stepName = "Presoak";
          timer.setCountdownTime(presoakTime*1000);
          timer.start();
          break;    
        
        case C41_DEVELOP:
          stepName = "Develop";
          timer.setCountdownTime(developTime*1000);
          timer.start();
          break;    

        case C41_BLIX:
          stepName = "Blix";
          timer.setCountdownTime(blixTime*1000);
          timer.start();
          break;    
          
        case C41_WASH:
          stepName = "Wash";
          timer.setCountdownTime(washTime*1000);
          timer.start();
          break;    
        
        case C41_STABILIZE:
          stepName = "Stabilize";
          timer.setCountdownTime(stabilizeTime*1000);
          timer.start();
          break;
              
        case C41_DONE:
          stepName = "Done";
          timer.start();
          done=1;
          break;        
      }
  } 
  else if (process == PROCESS_C41R) {
      done = 0;
      switch (processStep) {
        case C41R_PRESOAK:
          stepName = "Presoak";
          timer.setCountdownTime(presoakTime*1000);
          timer.start();
          break;    
        
        case C41R_FIRST_DEVELOP:
          stepName = "First Dev";
          timer.setCountdownTime(firstDevelopTime*1000);
          timer.start();
          break;    

        case C41R_STOP:
          stepName = "Stop";
          timer.setCountdownTime(stopTime*1000);
          timer.start();
          break;    
          
        case C41R_REEXPOSE:
          stepName = "Reexpose";
          timer.setCountdownTime(reexposeTime*1000);
          timer.start();
          break;    
        
        case C41R_COLOR_DEVELOP:
          stepName = "Color Dev";
          timer.setCountdownTime(developTime*1000);
          timer.start();
          break;
              
        case C41R_BLIX:
          stepName = "Blix";
          timer.setCountdownTime(blixTime*1000);
          timer.start();
          break;    

        case C41R_DONE:
          stepName = "Done";
          timer.start();
          done=1;
          break;        
      }
  } 
    else if (process == PROCESS_E6) {

      /*
       * #define E6_PRESOAK 0
#define E6_FIRST_DEVELOP 1
#define E6_STOPWASH 2
#define E6_COLOR_DEVELOP 3
#define E6_CDWASH 4
#define E6_BLIX 5
#define E6_DONE 6

       */

      done = 0;
      switch (processStep) {
        case E6_PRESOAK:
          stepName = "Presoak";
          timer.setCountdownTime(presoakTime*1000);
          timer.start();
          break;    
        
        case E6_FIRST_DEVELOP:
          stepName = "First Dev";
          timer.setCountdownTime(firstDevelopTime*1000);
          timer.start();
          break;    

        case E6_STOPWASH:
          EEPROM.write(EEPROM_E6_ROLL_COUNT,rollComp+1);
          stepName = "Wash x7";
          timer.setCountdownTime(stopTime*1000);
          timer.start();
          break;    
          
        case E6_COLOR_DEVELOP:
          stepName = "Color Dev";
          timer.setCountdownTime(developTime*1000);
          timer.start();
          break;

        case E6_CDWASH:
          stepName = "Wash x7";
          timer.setCountdownTime(washTime*1000);
          timer.start();
          break;    
              
        case E6_BLIX:
          stepName = "Blix";
          timer.setCountdownTime(blixTime*1000);
          timer.start();
          break;    

        case E6_DONE:
          stepName = "Done";
          timer.start();
          done=1;
          break;        
      }
    }
  else {
    done = 1;
    stepName = ":(";
  }
}

void ColorDevelopScreen::display() {
  lcd.setCursor(0,0);
  lcd.print(processName);
  lcd.print(" ");
  lcd.print(stepName);
  lcd.print("        ");
  lcd.setCursor(0,1);
  lcd.print(formatTime(lastTime));
  lcd.setCursor(10,1);
  lcd.print(formatTemp(tempSensor.getAsyncTemperature(0),'F'));
  lcd.setCursor(6,1);
  if (theHeater.getHeaterState()) {
    lcd.print("+++");
  } else {
    lcd.print("   ");    
  }
}


ColorDevelopScreen colorDevelopScreen;

/* -----------------------------------------------
 *  Color standby screen
 * -----------------------------------------------
 */
ColorStandbyScreen colorStandbyScreen;  

Screen* ColorStandbyScreen::next() {
  colorDevelopScreen.switchTo(this,process,stops,firstDevTime,rollComp,targetTemp);
  return &colorDevelopScreen;
}

void ColorStandbyScreen::up() { 
}

void ColorStandbyScreen::down() {
}

int ColorStandbyScreen::loop() {
  int timeElapsed = timer.getElapsedTime() / 1000;
  if (timeElapsed != lastTime) {
    lastTime = timeElapsed;
    return 1;
  }
  return 0;
}

Screen* ColorStandbyScreen::switchTo(Screen* previous) {
  if (previous == NULL) {
    timer.start();
  }
}

Screen* ColorStandbyScreen::switchTo(Screen* previous, int nProcess, int nStops, int fdTime, int rlComp, int tTemp) {
  pinMode(13,OUTPUT);
  digitalWrite(13,1);
  stops = nStops;
  process = nProcess;
  targetTemp = tTemp;
  firstDevTime = fdTime;
  rollComp = rlComp;
  timer.start();
  theHeater.setPowerState(1);
  theHeater.setTargetTemp(tTemp);
  return Screen::switchTo(previous);
}

Screen* ColorStandbyScreen::back() {
  pinMode(13,OUTPUT);
  digitalWrite(13,0);
  return Screen::back();
}


void ColorStandbyScreen::display() {
  lcd.setCursor(0,0);
  
  switch(process) {
    case PROCESS_C41:
      lcd.print("C41 Stnby ");
      break;
        
    case PROCESS_C41R:
      lcd.print("C41R Stnby");
      break;

    case PROCESS_E6:
      lcd.print("E6 Stnby  ");
      break;

  }
  lcd.setCursor(10,0);
  lcd.print(formatTemp(theHeater.getTargetTemp(),'F'));
  
  lcd.setCursor(0,1);
  lcd.print(formatTime(lastTime));
  lcd.setCursor(10,1);
  lcd.print(formatTemp(tempSensor.getAsyncTemperature(0),'F'));
  lcd.setCursor(6,1);
  if (theHeater.getHeaterState()) {
    lcd.print("+++");
  } else {
    lcd.print("   ");    
  }
}


ColorExposureScreen::ColorExposureScreen(int colorMode)
{
  process = colorMode;
  stops = 0;
}

Screen* ColorExposureScreen::next() {
  colorStandbyScreen.switchTo(this,process,stops,0,0,960);
  return &colorStandbyScreen;
}

void ColorExposureScreen::up() {
  stops++; if (stops > 2) stops = 2;
}

void ColorExposureScreen::down() {
  stops--; if (stops < 0) stops = 0;
}


void ColorExposureScreen::display() {
  lcd.setCursor(0,0);
  switch(process) {
    case PROCESS_C41:
      lcd.print("C-41 Push?      ");
      break;
        
    case PROCESS_C41R:
      lcd.print("C-41R FD?");
      break;
  }
  
  lcd.setCursor(0,1);
  if (stops > 0) {
    lcd.print("+");
    lcd.print(stops);
    lcd.print("              ");
  } else if (stops == 0) {
    lcd.print("0               ");
  } else {
    lcd.print(stops);
    lcd.print("              ");    
  }
}

/* ------------------------------------------
 *  C-41 Reversal First Developer choice
 * ------------------------------------------
 */

int c41r_fd_max_choice = 3;
int c41r_fdtimes[] = { 12*60, 18*60, 13*60, 30*60 };  // Previous was 13min, highlights a little blown
int c41r_fdtemps[] = { 1020, 1020, 1020, 960 };
char* c41r_fdnames[] = { "D23           ", "D23 1:1      ", "D23HighCntrst", "Rodinal 1+25 " };

C41RFirstDeveloperScreen::C41RFirstDeveloperScreen(int dummy)
{
  choice = 0;
}

Screen* C41RFirstDeveloperScreen::next() {
  int fdTime = c41r_fdtimes[choice];
  int tTemp = c41r_fdtemps[choice];
  colorStandbyScreen.switchTo(this,PROCESS_C41R,0,fdTime,0,tTemp);
  return &colorStandbyScreen;
}

void C41RFirstDeveloperScreen::up() {
  choice--; if (choice < 0) choice = c41r_fd_max_choice;
}

void C41RFirstDeveloperScreen::down() {
  choice++; if (choice > c41r_fd_max_choice) choice = 0;
}


void C41RFirstDeveloperScreen::display() {
  lcd.setCursor(0,0);
  lcd.print("C-41R FD?    ");
  
  lcd.setCursor(0,1);
  lcd.print(c41r_fdnames[choice]);
}

/* -------------------------------------------------------------
 *  E-6 Screens 
 *  
 *  E-6 push/pull -> E-6 roll compensation -> E-6 Standby -> E-6 Develop
 * -------------------------------------------------------------
 */

class E6RollCompScreen:public Screen {
  public:
    E6RollCompScreen();
    int pushOrPull;
    int rollComp;
    virtual Screen* next();
    virtual void up();
    virtual void down();        
    virtual void display();       
    virtual Screen* switchTo(Screen* from, int pushPullCount);       
};


E6RollCompScreen::E6RollCompScreen() {
}

Screen* E6RollCompScreen::switchTo(Screen* from, int pushPullCount) {
  prevScreen = from;
  pushOrPull = pushPullCount;
  rollComp = EEPROM.read(EEPROM_E6_ROLL_COUNT);
}

void E6RollCompScreen::down() {
  rollComp++; if (rollComp > 255) rollComp = 255;
}

void E6RollCompScreen::up() {
  rollComp--; if (rollComp < -1) rollComp = -1;
}

Screen* E6RollCompScreen::next() {
  if (rollComp == -1) {
    EEPROM.write(EEPROM_E6_ROLL_COUNT,0);
    rollComp = 0;
    return this;
  } else {
      colorStandbyScreen.switchTo(this,PROCESS_E6,pushOrPull,0,rollComp,1050);
      return &colorStandbyScreen;
  }
}

void E6RollCompScreen::display() {
  lcd.setCursor(0,0);
  lcd.print("E6 Roll Comp?");
  lcd.setCursor(0,1);
  if (rollComp == -1) {
    lcd.print("Next -> Reset");
  } else {
    lcd.print(rollComp);
    lcd.print("            ");    
  }
}


/* --- push/pull screen ---- */

E6RollCompScreen e6RollCompScreen;

class E6PushPullScreen:public Screen {
  public:
    E6PushPullScreen();
    int pushOrPull;
    virtual Screen* next();
    virtual void up();
    virtual void down();        
    virtual void display();       
};

E6PushPullScreen::E6PushPullScreen() {
  pushOrPull=0;
}

Screen* E6PushPullScreen::next() {
  e6RollCompScreen.switchTo(this,pushOrPull);
  return &e6RollCompScreen;
}

void E6PushPullScreen::down() {
  pushOrPull++; if (pushOrPull > 2) pushOrPull = 2;
}

void E6PushPullScreen::up() {
  pushOrPull--; if (pushOrPull < -2) pushOrPull = -2;
}

void E6PushPullScreen::display() {
  lcd.setCursor(0,0);
  lcd.print("E6 Push/Pull?");
  lcd.setCursor(0,1);
  lcd.print(pushOrPull);
  lcd.print("    ");
}

/* --------------
 *  B & W
 * --------------
 */

class BWTimeScreen:Screen {
  public:
    BWTimeScreen(int temp);
    int temp;
    int time;
    virtual Screen* next();
    virtual void up();
    virtual void down();        
    virtual void display();       
    virtual Screen* switchTo(Screen* from);       
};

BWTimeScreen::BWTimeScreen(int targetTemp) {
  temp=targetTemp;
  time=7*60+30;
}

void BWTimeScreen::up() {
  time += 15;
}

void BWTimeScreen::down() {
  time -= 15;
}

void BWTimeScreen::display() {
  lcd.setCursor(0,0);
  
  lcd.print("B&W time? ???F");
  
  lcd.setCursor(10,0);
  lcd.print(formatTemp(theHeater.getTargetTemp(),'F'));
  
  lcd.setCursor(0,1);
  lcd.print(formatTime(time));
  lcd.setCursor(10,1);
  lcd.print(formatTemp(tempSensor.getAsyncTemperature(0),'F'));
  lcd.setCursor(6,1);
  if (theHeater.getHeaterState()) {
  } else {
    lcd.print("   ");    
  }
}

Screen* BWTimeScreen::switchTo(Screen* previous) {
  pinMode(13,OUTPUT);
  digitalWrite(13,1);
  if (temp) {
    theHeater.setPowerState(1);
    theHeater.setTargetTemp(temp);
  }
  return Screen::switchTo(previous);
}

/*
void BWTimeScreen::display() {
  printf("B&W @ %dF\n",temp);
  printf("%d S\n",time);
}
*/

Screen* BWTimeScreen::next() {
  return this;
}


const char* film_process_modes[] = {
    "C-41            ",
    "E-6             ",
    "C-41R           ",
    "B&W (ambient)   ",
    "B&W (68F)       ",
    "B&W (72F)       ",
    "B&W (75F)       ",
    NULL
};

ColorExposureScreen c41ExposureScreen(PROCESS_C41);
E6PushPullScreen e6PushPullScreen;
C41RFirstDeveloperScreen c41RFirstDeveloperScreen(0);
BWTimeScreen bWTimeScreen68(680);
BWTimeScreen bWTimeScreen72(720);
BWTimeScreen bWTimeScreen75(750);
BWTimeScreen bWTimeScreenAmbient(0);

void* film_process_screens[] = {
  (void *)(&c41ExposureScreen),
  (void *)(&e6PushPullScreen),
  (void *)(&c41RFirstDeveloperScreen),
  (void *)(&bWTimeScreenAmbient),
  (void *)(&bWTimeScreen68),
  (void *)(&bWTimeScreen72),
  (void *)(&bWTimeScreen75),
  };  

class FilmProcessScreen:public ChoiceScreen {
  public:
    FilmProcessScreen() : ChoiceScreen("Film process    ",
                      film_process_modes,
                      film_process_screens)
                      {};
};
   



/* 
   -----------------------------------------------------
   Sous Vide / Hold Temp Screens 
   -----------------------------------------------------
*/

class HoldTempScreen:public Screen {
  public:
    virtual Screen* next();
    virtual void up();
    virtual void down();        
    virtual void display();       

    // Called when navigating forward 
    Screen* switchTo(Screen* previous, int tTemp);       
    virtual Screen* switchTo(Screen* previous);
    virtual int loop();
    
  protected:
    long targetTemp = 960;
    int lastTime = 0;

    Timer timer;    
};


HoldTempScreen holdTempScreen;  

Screen* HoldTempScreen::next() {
  return this;
}

void HoldTempScreen::up() { 
}

void HoldTempScreen::down() {
}

int HoldTempScreen::loop() {
  int timeElapsed = timer.getElapsedTime() / 1000;
  if (timeElapsed != lastTime) {
    lastTime = timeElapsed;
    return 1;
  }
  return 0;
}

Screen* HoldTempScreen::switchTo(Screen* previous) {
  if (previous == NULL) {
    timer.start();
  }
}

Screen* HoldTempScreen::switchTo(Screen* previous, int tTemp) {
  pinMode(13,OUTPUT);
  digitalWrite(13,1);
  targetTemp = tTemp;
  timer.start();
  theHeater.setPowerState(1);
  theHeater.setTargetTemp(tTemp);
  return Screen::switchTo(previous);
}


void HoldTempScreen::display() {
  lcd.setCursor(0,0);

  lcd.print("Sous Vide ");
  lcd.setCursor(10,0);
  lcd.print(formatTemp(theHeater.getTargetTemp(),'F'));
  
  lcd.setCursor(0,1);
  lcd.print(formatTime(lastTime));
  lcd.setCursor(10,1);
  lcd.print(formatTemp(tempSensor.getAsyncTemperature(0),'F'));
  lcd.setCursor(5,1);
  if (theHeater.getHeaterState()) {
    lcd.print(" +++");
  } else {
    lcd.print("    ");    
  }
}












const char* sous_vide_items[] = {
    "Emulsion        ",
    "Beef rare       ",
    "Beef medium rare",
    "Beef medium     ",
    "Beef medium well",
    "Beef well done  ",
    NULL
     };

void* sous_vide_data[] = {
  (void *)(1200),
  (void *)(1300),
  (void *)(1400),
  (void *)(1500),
  (void *)(1600),
  };  

class SousVideScreen:public ChoiceScreen {
  public:
    SousVideScreen() : ChoiceScreen("Cook...",
                      sous_vide_items,
                      sous_vide_data)
                      {};

    virtual Screen* next();
};

Screen* SousVideScreen::next() {
    holdTempScreen.switchTo(this,sous_vide_data[choice]);
    return &holdTempScreen;
}




FilmProcessScreen filmProcessScreen;
SousVideScreen sousVideScreen;

/* -----------------------------------
   Start screen
   ----------------------------------- */
const char* start_screen_modes[] = {
    "Process Film    ",
    "Sous-vide       ",
//    "Hold temp       ",
    NULL };

void* start_screen_screens[] = {
   (void*)&filmProcessScreen,
   (void*)&sousVideScreen,
   NULL };
   
class StartScreen:public ChoiceScreen {
  public:
    StartScreen() : ChoiceScreen("I want to...    ",
                  start_screen_modes,
                  start_screen_screens) {};
};

#endif
