
// Contact: Harry van Haaren <harryhaaren@gmail.com>
// Compile: g++ writingSoundfileToDisk.cpp `pkg-config --cflags --libs sndfile`

#include <cmath>
#include <iostream>
#include <sndfile.hh>

int main()
{
  const int format=SF_FORMAT_WAV | SF_FORMAT_FLOAT;
  const int channels=1;
  const int sampleRate=44100;
  const char* outfilename="foo.wav";
  
  // open a handle to a file, using the parameters defined above
  SndfileHandle outfile(outfilename, SFM_WRITE, format, channels, sampleRate);
  
  // prepare an array of 3 seconds long, based on the samplerate
  int size = sampleRate*3;
  float sample[size];
  
  int frequency = 1500; // <-- frequency of the wave generated
  
  for (int i=0; i < size; i++)
  {
    // write each sample, the math here is generating a sin wave
    sample[i]=sin( float(i) / size * M_PI * frequency);
  }
  
  // tell our file handle that we want to write "size" amount of data
  // from "the-address" of the "sample" array, starting at "0" 
  outfile.write(&sample[0], size);
  
  return 0;
}
