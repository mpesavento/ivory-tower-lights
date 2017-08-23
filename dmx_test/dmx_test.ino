#include <DmxSimple.h>

void setup() {
  DmxSimple.usePin(3);
  DmxSimple.maxChannel(11);
}

void loop() {
  int channel = 2;
//  for (int c=1; c<7; c++) {
    for (int value = 0; value <= 255; value++) {
      
      /* Update DMX channel 1 to new brightness */
      DmxSimple.write(channel, value);
  
      /* Small delay to slow down the ramping */
      delay(100);
    }
//  }

}
