

#ifndef PUREDATAHOST
#define PUREDATAHOST

#include <iostream>
#include <vector>
#include <math.h>
#include <cstring>

class PureDataHost
{
  public:
    PureDataHost(int nframes, int samplerate);
    
    void printInfo();
    void openFile(std::string openFile);
    
    void noteOn(int c, int p, int v);
    
    void process(float *L, float *R, int nframes);
  
  protected:
    int nframes;
    int samplerate;
    
    bool fileOpen;
    std::string filename;
    
    // static methods for PD output
    static void pdprint(const char *s);
    static void pdnoteon(int ch, int pitch, int vel);
};

#endif
