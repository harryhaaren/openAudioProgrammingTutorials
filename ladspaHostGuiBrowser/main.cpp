


#include "sila.h"

LadspaHost l_h;


int main(int argc, char **argv) {
    
    Gtk::Main main(argc, argv);

    l_h.m_window = NULL;
    l_h.l_b = new LadspaBrowser();
    l_h.sila_start(argc, argv);
    
    Gtk::Main::run();

    delete l_h.l_b;
    delete l_h.m_window;

    return 0;
}
