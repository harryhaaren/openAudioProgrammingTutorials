
// Contact: Harry van Haaren <harryhaaren@gmail.com>
// Compile: g++ waveview.cpp guiFftAnalyzer.cpp `pkg-config --cflags --libs gtkmm-2.4 jack`

#include <iostream>

#include <gtkmm.h>
#include <jack/jack.h>

// include the ringbuffer that JACK has
#include <jack/ringbuffer.h>

// include our custom widget to display the audio data
#include "waveview.hpp"

jack_port_t* inputPort = 0;

// a global pointer to a ring buffer, to move the audio data between the threads
jack_ringbuffer_t *ringbuffer = 0;


int process(jack_nframes_t nframes, void* )
{
  // get the input buffer pointer
  float* inputBuffer = (float*)jack_port_get_buffer ( inputPort , nframes);
  
  //    we now want to write nframes of data to the ringbuffer, if it
  //    has enough space to hold that amount of data
  
  // get amount of space we can write
  int availableWrite = jack_ringbuffer_write_space(ringbuffer);
  
  if (availableWrite >= sizeof(float) * nframes )
  {
    // write data directly from inputBuffer to the ringbuffer
    jack_ringbuffer_write( ringbuffer, (const char*) inputBuffer , nframes * sizeof(float) );
  }
  
  return 0;
}

// the GUI timeout signal will trigger this to happen every X milliseconds
bool redrawGui(GWaveView* waveview)
{
  std::cout << "Redrawing GUI now" << std::endl;
  
  // get read space
  int availableRead = jack_ringbuffer_read_space(ringbuffer);
  
  float startOfData[availableRead];
  
  for(int i = 0; i < availableRead; i++ )
    startOfData[i] = 0.f;
  
  // if there's data to be read
  if ( availableRead > sizeof(float) )
  {
    // read all of it, and pass it on to the Waveview widget
    int result = jack_ringbuffer_read( ringbuffer, (char*) &startOfData[0], availableRead );
    
    // tell the waveview to draw the new data
    waveview->draw( availableRead / sizeof(float), &startOfData[0] );
  }
  
}

int main(int argc, char** argv)
{
  std::cout << "GUI FFT Analyzer tutorial" << std::endl;
  
  // setup GTKMM
  Gtk::Main mainLoop(argc, argv);
  
  Gtk::Window window;
  
  // create custom widget instance
  GWaveView waveview;
  window.add( waveview );
  window.show_all();
  window.set_title("FFT Analyzer");
  
  // set a timer to call the GUI draw() function every X milliseconds
  Glib::signal_timeout().connect( sigc::bind(sigc::ptr_fun(&redrawGui), &waveview) , 25);
  
  
  // setup JACK stuff
  jack_client_t* client = jack_client_open ("GuiFftAnalyser",
                                            JackNullOption,
                                            0,
                                            0 );
  
  ringbuffer = jack_ringbuffer_create( 10 * jack_get_sample_rate(client) * sizeof(float));
  
  // register the process callback
  jack_set_process_callback  (client, process , 0);
  
  // register two ports, one input one output, both of AUDIO type
  inputPort = jack_port_register ( client,
                                    "input",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsInput,
                                    0 );
  
  // activate the client, ie: enable it for processing
  jack_activate(client);
  
  mainLoop.run(window);
  
  // tell JACK to stop processing the client
  jack_deactivate(client);
  
  // close the client
  jack_client_close(client);
  
  return 0;
}
