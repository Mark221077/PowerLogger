// 
// 
// 

#include "LCDControl.h"



void setLine(String lineNew, String linePrev, int row, LiquidCrystal_I2C lcd) {

  int i;
  for (i = 0; i < lineNew.length(); ++i) {      //go through the chars in the new string
    if (linePrev.length() < i || linePrev[i] != lineNew[i]) {   //if there is an empty(" ") char on the lcd
      lcd.setCursor(i, row);                    //or the two chars in the same place dont match
      lcd.print(lineNew[i]);                    //replace it
    }
  }
  for (; i < linePrev.length(); ++i) {      //if the previous line was longer
    lcd.setCursor(i, row);            //write blank chars over the previous string
    lcd.print(" ");
  }
}

void LCDControl::setLine1(String line)
{
  setLine(line, line1, 0, lcd);
  line1 = line;
}

void LCDControl::setLine2(String line)
{
  setLine(line, line2, 1, lcd);
  line2 = line;
}
