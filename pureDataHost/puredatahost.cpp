
#include "puredatahost.hpp"

extern "C"
{
#include "z_libpd.h"
}

using namespace std;

PureDataHost::PureDataHost(int inNframes, int inSamplerate)
{
  nframes = inNframes;
  samplerate = inSamplerate;
  
  fileOpen = false;
  filename = "";
  
  cout<< "Initializing PureDataHost..." << flush;
  cout<< "\tsetting up hooks & audio IO..." << flush;
  
  libpd_printhook = (t_libpd_printhook) pdprint;
  libpd_noteonhook = (t_libpd_noteonhook) pdnoteon;
  libpd_init();
  
  // init( in chans, out chans, samplerate )
  libpd_init_audio(1, 1, samplerate);
  
  
  // compute audio message [; pd dsp 1(
  
  // start_message( numEntriesInMessage )
  libpd_start_message(1);
  
  libpd_add_float(1.0f);
  libpd_finish_message("pd", "dsp");
  
  
  libpd_start_message(1);
  libpd_finish_message("loadbang", "1");
  
  cout << "Done!" << endl;
}

// output from PD
void PureDataHost::pdnoteon(int ch, int pitch, int vel)
{
  cout << "NoteOn: " << ch << " " << pitch << " " << vel << endl;
}

// input to PD
void PureDataHost::noteOn(int c, int p, int v)
{
  if ( c == 144 )
  {
    cout << "PureDataHost::noteOn("<<c<<", "<<p<<", "<<v<<")" << endl;
    libpd_noteon(c, p, v);
  }
  else if (c == 128 )
  {
    cout << "PureDataHost::noteOff("<<c<<", "<<p<<", "<<v<<")" << endl;
    libpd_noteon(c,p,0);
  }
  else
  {
    cout << "PureDataHost::controlChange("<<c<<", "<<p<<","<<v<<")" << endl;
    libpd_controlchange(c, p, v);
  }
}

// output from PD
void PureDataHost::pdprint(const char *s)
{
  cout << "Print: " << *s << endl;
}

// open a PD patch
void PureDataHost::openFile(string openFile)
{
  cout << "PureDataHost  Opening file : "<< openFile<< endl;
  
  // open patch
  void* result = libpd_openfile( openFile.c_str() , "./");
  
  if ( result != 0 )
  {
    fileOpen = true;
    cout << "File opened successfully!" << endl;
  }
  else
    cout << "ERROR LOADING PURE DATA FILE!" << endl;
}

void PureDataHost::printInfo()
{
  cout << "PureDataHost, Current Patch:" << filename << endl; ;
}

void PureDataHost::process(float *L, float *R, int nframes)
{
  if (!fileOpen)
  {
    //cout << "PureDataHost: No file open!" << endl;
    return;
  }
  
  
  // have to process at least 64 samples at a time
  if (nframes < 64)
  {
    cout << "Support for nframes < 64 not implemented yet" << endl;
    return;
  }
  
  // can process multiples of 64 at a time too,
  if (nframes % 64 != 0)
  {
    cout << "nframes is not a multiple of 64" << endl;
    return;
  }
  
  // loop over the libpd process audio function until we have "nframes" samples
  for (int i = 0; i < nframes; i += 64)
  {
    // 1 is the number of "ticks" to process
    libpd_process_float( 1, L + i, L + i );
  }
}
