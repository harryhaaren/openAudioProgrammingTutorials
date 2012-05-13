

#ifndef GFREQVIEW
#define GFREQVIEW

#include <iostream>

#include <vector>
#include <gtkmm.h>

#include <math.h>

class GFreqView : public Gtk::DrawingArea
{
  public:
    GFreqView();
    
    void draw(int nframes, float*);

  protected:
    int width;
    int height;
    
    int  fftDataCounter;
    
    // keep a *copy* of the data in a "deque"
    std::deque<float> sample;
    
    std::deque<float> fftData;
    
    // this is our own implementation of the FFT transform
    void performFFT(float data[], unsigned long number_of_complex_samples, int isign);
    
    // this is the GTK "draw" function, it's where we'll draw lines onto the widget
    bool on_expose_event(GdkEventExpose* event);
};

#endif // GFREQVIEW

