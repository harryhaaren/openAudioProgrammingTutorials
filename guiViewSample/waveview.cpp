

#include "waveview.hpp"

GWaveView::GWaveView()
{
  // Gives "Exposure" events to the widget, we need the for when we want
  // to redraw the widget!
  add_events(Gdk::EXPOSURE_MASK);
  
  // set default widget size
  set_size_request( 400, 100 );
  
  // set our flag so we copy the data when we call draw()
  copiedSample = false;
}


void GWaveView::draw(const std::vector<float>& inSample)
{
  if ( !copiedSample )
  {
    sample = inSample;
    copiedSample = true;
  }
  
  // force our program to redraw the entire widget.
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
  {
    Gdk::Rectangle r(0, 0, get_allocation().get_width(),get_allocation().get_height());
    win->invalidate_rect(r, false);
  }
}

bool GWaveView::on_expose_event(GdkEventExpose* event)
{
    // This is where we draw on the window
    Glib::RefPtr<Gdk::Window> window = get_window();
    
    if(window)    // Only run if Window does exist
    {
      Gtk::Allocation allocation = get_allocation();
      width = allocation.get_width();
      height = allocation.get_height();
      
      // coordinates for the center of the window
      int xc, yc;
      xc = width / 2;
      yc = height / 2;
      
      Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
      
      // clip to the area indicated by the expose event so that we only redraw
      // the portion of the window that needs to be redrawn
      cr->rectangle(event->area.x, event->area.y,event->area.width, event->area.height);
      cr->clip();
      
      int x = 0;
      int y = 0;
      int xSize = width;
      int ySize = height;
      
      // works but a bit simple
      cr -> move_to( x        , y         );
      cr -> line_to( x + xSize, y         );
      cr -> line_to( x + xSize, y + ySize );
      cr -> line_to( x        , y + ySize );
      cr -> close_path();
      
      // Draw outline shape
      cr -> set_source_rgb (0.1,0.1,0.1);
      cr -> fill();
      
      
      // don't draw every sample
      int sampleCountForDrawing = -1;
      
      float currentTop = 0.f;
      float previousTop = 0.f;
      float currentSample = 0.f;
      
      // loop for drawing each Point on the widget.
      for (long index=0; index <(long)sample.size(); index++ )
      {
        // get the current sample
        float currentSample = sample.at(index);
        
        if ( currentSample > 0 && currentTop < currentSample )
        {
          currentTop = currentSample;
        }
        sampleCountForDrawing--;
        
        if ( sampleCountForDrawing < 0 )
        {
          float drawSample = currentTop;
          
          int xCoord = x + ( xSize * ( float(index) / sample.size() ) );
          
          cr->move_to( xCoord, y + (ySize/2) - (previousTop * ySize )  ); // top
          cr->line_to( xCoord, y + (ySize/2) + (drawSample  * ySize )  ); // bottom
          
          sampleCountForDrawing = 15;
          previousTop = drawSample;
          currentTop = 0;
        }
        
      }
      
      cr -> set_source_rgb (1.0,1.0,1.0);
      cr->stroke();
    }
  return true;
} // on_expose_event()

