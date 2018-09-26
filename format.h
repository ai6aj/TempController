#ifndef __DISPLAY_H__
#define __DISPLAY_H__ 1

/* ---------------------------------------
   Formatting routines for display
   --------------------------------------- */

// Generic buffer for formatXXX functions
char charTempRv[17];

// Format a time given in seconds to MM:SS
// Useful for countdown timers etc.
// NO zero suppression
char* formatTime(int time_in_seconds) {
  int minutes = time_in_seconds/60;
  int minutesTen = minutes/10;
  int minutesOne = minutes - minutesTen*10;
  
  int seconds = time_in_seconds-(minutes*60);
  int secondsTen = seconds/10;
  int secondsOne = seconds - secondsTen*10;
  charTempRv[0] = minutesTen+48;
  charTempRv[1] = minutesOne+48;
  charTempRv[2] = ':';
  charTempRv[3] = secondsTen+48;
  charTempRv[4] = secondsOne+48;
  charTempRv[5] = 0;

  return charTempRv;
}


// Format a temperature given in units * 10
// i.e. 312 = 31.2
// 'unit' is character for temperature unit (C or F)
// Temperate is always in format XXX.XU, leading zero
// suppressed
char* formatTemp(int temperature, char unit) {
  charTempRv[6] = 0;
  int zpad = 0;
  int temptemp = temperature;
  int temp = temperature/1000;
  if (temp > 0) {
     charTempRv[0] = 48+temp;     
     zpad = 1;
  } else {
     charTempRv[0] = ' ';         
  }
  
  temptemp -= (temp*1000);
  temp = temptemp/100;
  if (temp > 0 || zpad) {
     charTempRv[1] = 48+temp;     
  } else {
     charTempRv[1] = ' '; 
  }

  temptemp -= (temp*100);
  temp = temptemp/10;
  charTempRv[2] = 48+temp;     
  charTempRv[3] = '.';     

  temp = temptemp - temp*10;
  charTempRv[4] = 48+temp;     
  charTempRv[5] = unit;     
  return charTempRv;
}

#endif