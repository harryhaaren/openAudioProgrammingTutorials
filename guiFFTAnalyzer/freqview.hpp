

#ifndef GFREQVIEW
#define GFREQVIEW

#include <vector>
#include <gtkmm.h>

#include <math.h>

// include the files for the frequency analysis
#include "kiss_fft.h"
#include "kiss_fftr.h"

class GFreqView : public Gtk::DrawingArea
{
  public:
    GFreqView();
    
    void draw(int nframes, float*);

  protected:
    int width;
    int height;
    
    // frequency analysis
    kiss_fftr_cfg fft;
    
    kiss_fft_cpx* cpx_buf;
    
    kiss_fft_cpx out_cpx[size];
    
    // keep a *copy* of the data in a "deque"
    std::deque<float> sample;
    
    // this is the GTK "draw" function, it's where we'll draw lines onto the widget
    bool on_expose_event(GdkEventExpose* event);
};

#endif // GFREQVIEW

