
#include <iostream>
#include <gtkmm.h>
#include <string>

#include <flowcanvas.hpp>
#include <ganv/ganv.hpp>

// Author: Harry van Haaren, 2012
// Compile: g++ -Wall -I/usr/include/ganv -I/usr/include/flowcanvas flowcanvas.cpp `pkg-config gtkmm-2.4 ganv-1 flowcanvas --cflags --libs`

using namespace std;

class UI : public Ganv::Canvas
{
  public:
    UI()
      : Ganv::Canvas(800,600)
    {
      box1 = new Ganv::Module( *this, "source", 10, 20 );
      box2 = new Ganv::Module( *this, "destination", 30, 80 );
      
      box1Out = new Ganv::Port( *box1, "out", false, 0x244678FF);
      box1Out = new Ganv::Port( *box2, "in", true, 0x244678FF);
      
      signal_connect.connect    (sigc::mem_fun(this, &UI::nodeConnect    ));
      signal_disconnect.connect (sigc::mem_fun(this, &UI::nodeDisconnect ));
    }
    
    void nodeConnect(Ganv::Node* port1, Ganv::Node* port2)
    {
      // don't need to keep the pointer, we disconnect using both "ends"
      new Ganv::Edge(*this, port1, port2, 0x244678FF);
    }
    void nodeDisconnect(Ganv::Node* port1, Ganv::Node* port2)
    {
      remove_edge(port1, port2);
    }
  
  private:
    Ganv::Module* box1;
    Ganv::Module* box2;
    
    Ganv::Port* box1Out;
    Ganv::Port* box2In;
};

int main(int argc, char**argv)
{
  
  Glib::thread_init();
  Gtk::Main kit(argc, argv);
  
  UI ui;
  
  Gtk::Window window;
  
  window.add( ui.widget() );
  
  window.show_all();
  
  kit.run( window );
  
  return 0;
  
}
