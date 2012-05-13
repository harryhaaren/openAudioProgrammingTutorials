

#include "freqview.hpp"

using namespace std;

#define SWAP(a,b)tempr=(a);(a)=(b);(b)=tempr;
//tempr is a variable from our FFT function

GFreqView::GFreqView()
{
  // Gives "Exposure" events to the widget, we need the for when we want
  // to redraw the widget!
  add_events(Gdk::EXPOSURE_MASK);
  
  // set default widget size
  set_size_request( 100, 500 );
  
  fftDataCounter = 0;
  fftData.clear();
}


void GFreqView::draw(int nframes, float* data)
{
  cout << "GFreqView::draw() now!" << flush;
  
  for (int i = 0; i < nframes; i++)
  {
    // always copy the audio data, we'll use only what we need later!
    fftData.push_back( *data ); // real     (even)
    fftData.push_back( 0 );     // complex  ( odd)
    fftDataCounter++;
    
    data++; // move to next sample in input buffer
  }

  // force our program to redraw the entire widget.
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
  {
    Gdk::Rectangle r(0, 0, get_allocation().get_width(),get_allocation().get_height());
    win->invalidate_rect(r, false);
  }
  
  return;
}


// FFT algorithm & tutorial taken from here:
// http://www.codeproject.com/Articles/9388/How-to-implement-the-FFT-algorithm
void GFreqView::performFFT(float data[], unsigned long number_of_complex_samples, int isign)
{
    //variables for trigonometric recurrences
    unsigned long n,mmax,m,j,istep,i;
    double wtemp,wr,wpr,wpi,wi,theta,tempr,tempi;
    
    double pi = 3.1415;
    
    
    //      Bit Reversal
    //the complex array is real+complex so the array 
    //as a size n = 2* number of complex samples
    // real part is the data[index] and the complex part is the data[index+1]
    n=number_of_complex_samples * 2;
    
    //binary inversion (note that 
    //the indexes start from 1 witch means that the
    //real part of the complex is on the odd-indexes
    //and the complex part is on the even-indexes
    j=1;
    for (i=1;i<n;i+=2) { 
        if (j > i) {
            
            //swap the real part
            //SWAP(data[j],data[i]); 
            float tmp = data[j];
            data[j] = data[i];
            data[i] = tmp;
            
            
            //swap the complex part
            //SWAP(data[j+1],data[i+1]);
            tmp = data[j+1];
            data[j+1] = data[i+1];
            data[i+1] = tmp;
        }
        m=n/2;
        while (m >= 2 && j > m) {
            j -= m;
            m = m/2;
        }
        j += m;
    }
    
    //  End Bit Reversal
    
    //Danielson-Lanzcos FFT routine 
    mmax=2;
    //external loop
    while (n > mmax)
    {
        istep = mmax<<  1;
        theta=sin(2*pi/mmax);
        wtemp=sin(0.5*theta);
        wpr = -2.0*wtemp*wtemp;
        wpi=sin(theta);
        wr=1.0;
        wi=0.0;
        //internal loops
        for (m=1;m<mmax;m+=2) {
            for (i= m;i<=n;i+=istep) {
                j=i+mmax;
                tempr=wr*data[j-1]-wi*data[j];
                tempi=wr*data[j]+wi*data[j-1];
                data[j-1]=data[i-1]-tempr;
                data[j]=data[i]-tempi;
                data[i-1] += tempr;
                data[i] += tempi;
            }
            wr=(wtemp=wr)*wpr-wi*wpi+wr;
            wi=wi*wpr+wtemp*wpi+wi;
        }
        mmax=istep;
    }
    
}

bool GFreqView::on_expose_event(GdkEventExpose* event)
{
  
  cout << "FreqView::onExposeEvent() " << endl;
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
      
      // perform the FFT after some checks, fftDataCounter = window size
      fftDataCounter = 512;
      
      if ( fftData.size() < fftDataCounter * 2 )
      {
        std::cout << " fftData < 1024, returning!" << endl;
        return true;
      }
      
      std::vector<float> plotData;
      
      for ( int i = 0; i < fftDataCounter * 2; i++)
      {
        plotData.push_back( fftData.at(i) );
      }
      
      std::cout << "Performing FFT routine NOW, fftDataCounter = " << fftDataCounter << flush;
      // now perform the FFT, and the output will be written to fftData
      performFFT( &plotData[0], fftDataCounter, 1 );
      std::cout << "  FFT finished!" << endl;
      
      
      // interpret FFT data, get peaks from complex numbers
      // complex amplitude = square root of the square of the real plus the square of the complex.
      float max = 0.f;
      int bin = -1;
      
      for ( int i = 0; i < fftDataCounter; i += 2) // += 2 for Real & Complex numbers stored in one array
      {
        float real = sqrt ( plotData[i] * plotData[i] );
        float complex = plotData[i+1] * plotData[i+1];
        
        if ( real + complex > max )
        {
          max = real + complex;
          bin = i; // again, real & complex in one array, so divide it!
        }
      }
      
      cout << "Max amp from draw = " << max << "  in bin number " << bin << " so freq = " << bin * 43 << endl;
      
      
      fftData.clear();
      fftDataCounter = 0;
      
      
      // draw line on widget to show current bin tracking
      cr->move_to( 0, ySize * (512.f / bin)  ); // top
      cr->line_to( xSize, ySize * (512.f / bin)  ); // bottom
      cr -> set_source_rgb (0.0,1.0,1.0);
      cr->stroke();
      
      
      
      
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

