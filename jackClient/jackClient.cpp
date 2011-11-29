
// Contact: Harry van Haaren <harryhaaren@gmail.com>
// Compile: g++ jackClient.cpp `pkg-config --cflags --libs jack`

#include <iostream>
#include <jack/jack.h>

// declare two "jack_port_t" pointers, which will each represent a port
// in the JACK graph (ie: Connections tab in QJackCtl)
jack_port_t* inputPort = 0;
jack_port_t* outputPort = 0;


// this function is the main audio processing loop, JACK calls this function
// every time that it wants "nframes" number of frames to be processed.
// nframes is usually between 64 and 256, but you should always program
// so that you can work with any amount of frames per process() call!
int process(jack_nframes_t nframes, void* )
{
  // here's a touch tricky, port_get_buffer() will return a pointer to
  // the data that we will use, so cast it to (float*), so that we
  // can use the data as floating point numbers. JACK will always pass
  // floating point samples around, the reason that we have to cast it
  // ourselves is so that it could be changed in the future... don't worry
  // about it too much.
  float* inputBuffer = (float*)jack_port_get_buffer ( inputPort , nframes);
  float* outputBuffer= (float*)jack_port_get_buffer ( outputPort, nframes);
  
  
  // this is the intresting part, we work with each sample of audio data
  // one by one, copying them across. Try multiplying the input by 0.5,
  // it will decrease the volume...
  for ( int i = 0; i < (int) nframes; i++)
  {
    // copy data from input to output. Note this is not optimized for speed!
    outputBuffer[i] = inputBuffer[i];
  }
  
  return 0;
}

int main()
{
  std::cout << "JACK client tutorial" << std::endl;
  
  // create a JACK client and activate
  jack_client_t* client = jack_client_open ("JackClientTutorial",
                                            JackNullOption,
                                            0,
                                            0 );
  
  // register the process callback
  jack_set_process_callback  (client, process , 0);
  
  // register two ports, one input one output, both of AUDIO type
  inputPort = jack_port_register ( client,
                                    "input",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsInput,
                                    0 );
  
  outputPort = jack_port_register ( client,
                                    "output",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput,
                                    0 );
  
  // activate the client, ie: enable it for processing
  jack_activate(client);
  
  // pause a while, letting JACK copy the data across
  sleep(30);
  
  // tell JACK to stop processing the client
  jack_deactivate(client);
  
  // close the client
  jack_client_close(client);
  
  return 0;
}
