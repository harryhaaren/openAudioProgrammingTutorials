
// Contact: Harry van Haaren <harryhaaren@gmail.com>
// Compile: g++ -o pureDataHost puredatahost.cpp main.cpp `pkg-config --cflags --libs libpd`

// this is a simple class that demonstrates how to create an instance of
// the PureDataHost class, which allows simple use of PD patches.

// Note: No audio or MIDI input / output exists in the example, it is purely
// a PureDataHost tutorial.

#include <iostream>

#include "puredatahost.hpp"

using namespace std;

int main()
{
  int nframes = 256;
  int samplerate = 44100;
  
  cout << "Main, creating PureDataHost now... " << endl;
  
  PureDataHost pdHost(nframes, samplerate);
  
  cout << "Successfully created PureDataHost instance! Quitting now" << endl;
  
  return 0;
}
