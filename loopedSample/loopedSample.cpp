
// Contact: Harry van Haaren <harryhaaren@gmail.com>
// Compile: g++ jackClient.cpp `pkg-config --cflags --libs jack`

// If this tutorial seems a little advanced, please look back over the
// Simple JACK client tutorial. https://github.com/harryhaaren/Linux-Audio-Programming-Documentation/

#include <iostream>
#include <vector>
#include <jack/jack.h>


// include sndfile so we have access to the sample loading library.
// Note we've included a .hh file, that's the C++ wrapper for sndfile.
#include <sndfile.hh>

jack_port_t* outputPort = 0;

// get a vector, that's an Array that we can dynamically resize. It contains
// floats in this case, so we can store audio samples in it.
std::vector<float> sampleVector;

// create an "index", variable that keeps track of where we are currently
// playing back in the file.
int playbackIndex = 0;

int process(jack_nframes_t nframes, void* )
{
  float* outputBuffer= (float*)jack_port_get_buffer ( outputPort, nframes);
  
  // this is the intresting part, we work with each sample of audio data
  // one by one, copying them across from the vector that has the sample
  // in it, to the output buffer of JACK.
  for ( int i = 0; i < (int) nframes; i++)
  {
    // here we check if index has gone played the last sample, and if it
    // has, then we reset it to 0 (the start of the sample).
    if ( playbackIndex >= sampleVector.size() ) {
      playbackIndex = 0;
    }
    
    // because we have made sure that index is always between 0 -> sample.size(),
    // its safe to access the array .at(index)  The program would crash it furthur
    outputBuffer[i] = sampleVector.at(playbackIndex);
    
    // increase the index, so that we play the next sample
    playbackIndex++;
  }
  
  return 0;
}

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


int main()
{
  std::cout << "Looped Sample Playback tutorial" << std::endl;
  
  // call the loadSample function above, just to separte the sample
  // loading code from the jack client creation code below.
  int loadNotOK = loadSample();
  
  if ( loadNotOK )
  {
    std::cout << "Error in sample loading function, check the sample\
exists in the directory the program is running!" << std::endl;
    return 0; // quit, as we didn't setup correctly
  } 
  
  jack_client_t* client = jack_client_open ("LoopedSample",
                                            JackNullOption,
                                            0,
                                            0 );
  
  jack_set_process_callback  (client, process , 0);
  
  // register an audio output port
  outputPort = jack_port_register ( client,
                                    "output",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput,
                                    0 );
  
  jack_activate(client);
  
  sleep(60);
  
  std::cout << "Quitting, 60 seconds sleep is over :)" << std::endl;
  
  jack_deactivate(client);
  
  jack_client_close(client);
  
  return 0;
}
