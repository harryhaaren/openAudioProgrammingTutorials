

#include "freqview.hpp"

GFreqView::GFreqView()
{
  // Gives "Exposure" events to the widget, we need the for when we want
  // to redraw the widget!
  add_events(Gdk::EXPOSURE_MASK);
  
  // set default widget size
  set_size_request( 400, 100 );
  
  dataSize = 128;
  
  fft =  kiss_fftr_alloc( dataSize * 2, 0, 0, 0 );
}


void GFreqView::draw(int nframes, float* data)
{
  for (int i = 0; i < nframes; i++)
  {
    // copy the data from the ring buffer to the vector in the widget
    sample.push_back( *data++ );
  }
  
  // limit the size of the sample being shown
  while( sample.size() > 256 )
    sample.erase (sample.begin(),sample.begin() + 1);
  
  // do the frequency analysis
  cpx_buf = copycpx(array,size);
  
  cpx_buf = (kiss_fft_cpx*) KISS_FFT_MALLOC ( sizeof(kiss_fft_cpx) * dataSize);
  
  // create scalar data holder
  kiss_fft_scalar zero;
  
  // initialize it to all zero's
  memset( &zero, 0, sizeof(zero) );
  
  for(i=0; i<nframe ; i++)
  {
    mat2[i].r = mat[i];
    mat2[i].i = zero;
  }
  
  // data transformation done!
  
  
  
  
  // force our program to redraw the entire widget.
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
  {
    Gdk::Rectangle r(0, 0, get_allocation().get_width(),get_allocation().get_height());
    win->invalidate_rect(r, false);
  }
}

bool GFreqView::on_expose_event(GdkEventExpose* event)
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

