

#ifndef GWAVEVIEW
#define GWAVEVIEW

#include <vector>
#include <gtkmm.h>

class GWaveView : public Gtk::DrawingArea
{
  public:
    GWaveView();
    
    void draw(const std::vector<float>& inSample);

  protected:
    int width;
    int height;
    
    // keep track of if we have the data from the sample
    bool copiedSample;
    
    // keep a *copy* of the vector
    std::vector<float> sample;
    
    // this is the GTK "draw" function, it's where we'll draw lines onto the widget
    bool on_expose_event(GdkEventExpose* event);
};

#endif // GWAVEVIEW

