//Copyright 2017 by Walter Zimmer
// Version 18-05-17
//
// Arduino interface
// 

#include "myApp.h"
c_myApp myApp;
/*-----------------------------------------------------------*/
void setup()
{
  // wait for serial line to come up
  myApp.setup();
}

void loop()
{ myApp.loop();
}


