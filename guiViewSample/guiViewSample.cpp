
// Contact: Harry van Haaren <harryhaaren@gmail.com>
// Compile: g++ -oguiViewSample guiViewSample.cpp `pkg-config --cflags --libs sndfile gtkmm-2.4`

// In this tutorial you will learn how to create a simple Gtkmm window,
// and then show a custom widget inside it to draw a waveform of a sample
// Please read the "loopedSample" tutorial for info on how to read samples

#include <iostream>
#include <vector>
#include <sndfile.hh>

// to get access to the GUI functions
#include <gtkmm.h>

// get a vector, that's an Array that we can dynamically resize. It contains
// floats in this case, so we can store audio samples in it.
std::vector<float> sampleVector;

int loadSample()
{
  // create a "Sndfile" handle, it's part of the sndfile library we 
  // use to load samples
  SndfileHandle fileHandle( "sample.wav" , SFM_READ,  SF_FORMAT_WAV | SF_FORMAT_FLOAT , 1 , 44100);
  
  // get the number of frames in the sample
  int size  = fileHandle.frames();
  
  if ( size == 0 )
  {
    // file doesn't exist or is of incompatible type, main handles the -1
    return -1;
  }
  
  // get some more info of the sample
  int channels   = fileHandle.channels();
  int samplerate = fileHandle.samplerate();
  
  // we declared sampleVector earlier, now we resize it
  sampleVector.resize(size);
  
  // this tells sndfile to 
  fileHandle.read( &sampleVector.at(0) , size );
  
  std::cout << "Loaded a file with " << channels << " channels, and a samplerate of " <<
      samplerate << " with " << size << " samples, so its duration is " <<
      size / samplerate << " seconds long." << std::endl;
  
  return 0;
}


int main(int argc, char** argv)
{
  std::cout << "GUI View Sample tutorial" << std::endl;
  
  int loadNotOK = loadSample();
  
  if ( loadNotOK )
  {
    std::cout << "Error in sample loading function, check the sample\
exists in the directory the program is running!" << std::endl;
    return 0; // quit, as we didn't setup correctly
  }
  
  // create an instance of Gtk::Main, we use it to make the GUI run later
  Gtk::Main kit(argc, argv);
  
  // create a "Window"
  Gtk::Window window;
  
  // set some attributes
  window.set_default_size(300,150);
  window.set_position(Gtk::WIN_POS_CENTER);
  window.show_all();
  
  
  // now call on the Gtk::Main to run, and it does the work for us.
  kit.run(window);
  
  // note that until the Window is closed (and the Gtk::Main quits) the
  // program will stop on the line above this, and the program will exit
  // as soon as the window is closed!
  
  return 0;
}
