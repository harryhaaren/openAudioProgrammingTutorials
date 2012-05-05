

#include "sila.h"

/**********************************************************************/

static void set_default_value(LADSPA_PortRangeHintDescriptor hdescriptor,
                        float lower_bound, float upper_bound, float *dest) {
    float default_value = 0.0;
    if (LADSPA_IS_HINT_HAS_DEFAULT(hdescriptor)) {
        switch (hdescriptor & LADSPA_HINT_DEFAULT_MASK) {
         case LADSPA_HINT_DEFAULT_MINIMUM:
            default_value = lower_bound;
            break;
         case LADSPA_HINT_DEFAULT_LOW:
            if (LADSPA_IS_HINT_LOGARITHMIC(hdescriptor)) {
                default_value = exp(log(lower_bound) * 0.75 + log(upper_bound) * 0.25);
            } else {
                default_value = lower_bound * 0.75 + upper_bound * 0.25;
            }
            break;
         case LADSPA_HINT_DEFAULT_MIDDLE:
            if (LADSPA_IS_HINT_LOGARITHMIC(hdescriptor)) {
                default_value = exp(log(lower_bound) * 0.5 + log(upper_bound) * 0.5);
            } else {
                default_value = lower_bound * 0.5 + upper_bound * 0.5;
            }
            break;
         case LADSPA_HINT_DEFAULT_HIGH:
            if (LADSPA_IS_HINT_LOGARITHMIC(hdescriptor)) {
                default_value = exp(log(lower_bound) * 0.25 + log(upper_bound) * 0.75);
            } else {
                default_value = lower_bound * 0.25 + upper_bound * 0.75;
            }
            break;
         case LADSPA_HINT_DEFAULT_MAXIMUM:
            default_value = upper_bound;
            break;
         case LADSPA_HINT_DEFAULT_0:
            default_value = 0.0;
            break;
         case LADSPA_HINT_DEFAULT_1:
            default_value = 1.0;
            break;
         case LADSPA_HINT_DEFAULT_100:
            default_value = 100.0;
            break;
         case LADSPA_HINT_DEFAULT_440:
            default_value = 440.0;
            break;
        }
        *dest = default_value;
    }
}

/**********************************************************************/

class Slider : public Gtk::HScale {
 public:
    Slider(int port);
 private:
    void on_value_changed();
    float *dest;
};

Slider::Slider(int port) {
    LADSPA_PortRangeHint hint = l_h.l_p.descriptor->PortRangeHints[port];
    LADSPA_PortRangeHintDescriptor hdescriptor = hint.HintDescriptor;
    float lower_bound = hint.LowerBound;
    float upper_bound = hint.UpperBound;
    

    dest = &l_h.l_p.cpv[port];

    if (LADSPA_IS_HINT_SAMPLE_RATE(hdescriptor)) {
        lower_bound *= l_h.get_samplerate();
        upper_bound *= l_h.get_samplerate();
    }
    if (LADSPA_IS_HINT_LOGARITHMIC(hdescriptor))
    {
      if (lower_bound < FLT_EPSILON)
          lower_bound = FLT_EPSILON;
    }

    if (LADSPA_IS_HINT_BOUNDED_BELOW(hdescriptor) &&
        LADSPA_IS_HINT_BOUNDED_ABOVE(hdescriptor)) {
        set_range(lower_bound, upper_bound);
    } else if (LADSPA_IS_HINT_BOUNDED_BELOW(hdescriptor)) {
        set_range(lower_bound, 1.0);
    } else if (LADSPA_IS_HINT_BOUNDED_ABOVE(hdescriptor)) {
        set_range(0.0, upper_bound);
    } else {
        set_range(-1.0, 1.0);
    }

    if (LADSPA_IS_HINT_TOGGLED(hdescriptor)) {
        set_range(0.0, 1.0);
        set_increments(1.0, 1.0);
    }

    if (LADSPA_IS_HINT_INTEGER(hdescriptor)) {
        set_increments(1.0, 1.0);
        set_digits(0);
    } else {
        set_increments(0.05, 0.1);
        set_digits(2);
    }
    set_default_value(hdescriptor, lower_bound, upper_bound, dest);
   // here we can set initial values if we wish like that
   /* if(port == 13 && l_h.l_p.descriptor->UniqueID == 34049) {
        *dest = 2990.7;
    } else if (port == 12 && l_h.l_p.descriptor->UniqueID == 33918) {
        *dest = 3556.56;
    }*/
    set_value(*dest);
}

void Slider::on_value_changed() {
    *dest = get_value();
}

/**********************************************************************/

class TButton : public Gtk::ToggleButton {
 public:
    TButton(int port);
 private:
    void on_toggled();
    float *dest;
};

TButton::TButton(int port) {
    LADSPA_PortRangeHint hint = l_h.l_p.descriptor->PortRangeHints[port];
    LADSPA_PortRangeHintDescriptor hdescriptor = hint.HintDescriptor;
    set_label(l_h.l_p.descriptor->PortNames[port]);

    float default_value;
    dest = &l_h.l_p.cpv[port];

    if (LADSPA_IS_HINT_HAS_DEFAULT(hdescriptor)) {
        switch (hdescriptor & LADSPA_HINT_DEFAULT_MASK) {
         case LADSPA_HINT_DEFAULT_0:
            default_value = 0.0;
            set_active(false);
            break;
         case LADSPA_HINT_DEFAULT_1:
            default_value = 1.0;
            set_active(true);
            break;
         default:
            default_value = 0.0;
            set_active(false);
            break;
        }
        *dest = default_value;
    }
}

void TButton::on_toggled() {
    if(get_active()) *dest = 1.0;
    else *dest = 0.0;
}

/**********************************************************************/

class Spiner : public Gtk::SpinButton {
 public:
    Spiner(int port);
 private:
    void on_value_changed();
    float *dest;
};

Spiner::Spiner(int port) {
    LADSPA_PortRangeHint hint = l_h.l_p.descriptor->PortRangeHints[port];
    LADSPA_PortRangeHintDescriptor hdescriptor = hint.HintDescriptor;
    float lower_bound = hint.LowerBound;
    float upper_bound = hint.UpperBound;

    dest = &l_h.l_p.cpv[port];

    if (LADSPA_IS_HINT_SAMPLE_RATE(hdescriptor)) {
        lower_bound *= l_h.get_samplerate();
        upper_bound *= l_h.get_samplerate();
    }
    if (LADSPA_IS_HINT_LOGARITHMIC(hdescriptor))
    {
      if (lower_bound < FLT_EPSILON)
          lower_bound = FLT_EPSILON;
    }

    if (LADSPA_IS_HINT_BOUNDED_BELOW(hdescriptor) &&
        LADSPA_IS_HINT_BOUNDED_ABOVE(hdescriptor)) {
        set_range(lower_bound, upper_bound);
    } else if (LADSPA_IS_HINT_BOUNDED_BELOW(hdescriptor)) {
        set_range(lower_bound, 1.0);
    } else if (LADSPA_IS_HINT_BOUNDED_ABOVE(hdescriptor)) {
        set_range(0.0, upper_bound);
    } else {
        set_range(-1.0, 1.0);
    }

    set_increments(1.0, 1.0);
    set_digits(0);

    set_default_value(hdescriptor, lower_bound, upper_bound, dest);
   // here we can set initial values if we wish like that
   /* if(port == 13 && l_h.l_p.descriptor->UniqueID == 34049) {
        *dest = 2990;
    } else if (port == 12 && l_h.l_p.descriptor->UniqueID == 33918) {
        *dest = 3556;
    }*/
    set_value(*dest);
}

void Spiner::on_value_changed() {
    *dest = get_value();
}

/**********************************************************************/

class ToolBar : public Gtk::Toolbar {
 public:
    ToolBar();
 private:
    bool is_active;
    Gtk::ToolButton * toolbutton;
    Gtk::Menu *menumono;
    Gtk::Menu *menustereo;
    Gtk::Menu *menumisc;
    Gtk::MenuItem* menuitem;
    void on_browser();
    void on_close();
    void on_stop();
    void on_play();
    void on_pause();
    void on_menu_popup(Gtk::Menu *menu);
    void load_plug(Glib::ustring label);
    typedef std::list<Glib::ustring>::iterator _iterator;
};

ToolBar::ToolBar() {

    toolbutton = Gtk::manage(new Gtk::ToolButton(Gtk::Stock::NEW));
    toolbutton->set_label("NEW");
    append(*toolbutton, sigc::mem_fun( *this, &ToolBar::on_browser));

    menumono = Gtk::manage (new Gtk::Menu);
    toolbutton = Gtk::manage(new Gtk::ToolButton());
    toolbutton->set_label("Mono");
    append(*toolbutton, sigc::bind<Gtk::Menu*>(
        sigc::mem_fun(*this, &ToolBar::on_menu_popup), menumono));

    menustereo = Gtk::manage (new Gtk::Menu);
    toolbutton = Gtk::manage(new Gtk::ToolButton());
    toolbutton->set_label("Stereo");
    append(*toolbutton, sigc::bind<Gtk::Menu*>(
        sigc::mem_fun( *this, &ToolBar::on_menu_popup), menustereo));

    menumisc = Gtk::manage (new Gtk::Menu);
    toolbutton = Gtk::manage(new Gtk::ToolButton());
    toolbutton->set_label("Misc");
    append(*toolbutton, sigc::bind<Gtk::Menu*>(
        sigc::mem_fun( *this, &ToolBar::on_menu_popup), menumisc));

    toolbutton = Gtk::manage(new Gtk::ToolButton(Gtk::Stock::MEDIA_PLAY));
    toolbutton->set_label("PLAY");
    append(*toolbutton, sigc::mem_fun( *this, &ToolBar::on_play));

    toolbutton = Gtk::manage(new Gtk::ToolButton(Gtk::Stock::MEDIA_PAUSE));
    toolbutton->set_label("PAUSE");
    append(*toolbutton, sigc::mem_fun( *this, &ToolBar::on_pause));

    toolbutton = Gtk::manage(new Gtk::ToolButton(Gtk::Stock::MEDIA_STOP));
    toolbutton->set_label("STOP");
    append(*toolbutton, sigc::mem_fun( *this, &ToolBar::on_stop));

    toolbutton = Gtk::manage(new Gtk::ToolButton(Gtk::Stock::CLOSE));
    toolbutton->set_label("CLOSE");
    append(*toolbutton, sigc::mem_fun( *this, &ToolBar::on_close));
    
    
    for (_iterator its = l_h.plug_mono_list.begin();
                         its != l_h.plug_mono_list.end(); its++) {
        std::string entry = *its;
        menuitem = Gtk::manage(new Gtk::MenuItem(entry, true));
        menuitem->set_use_underline(false);
        menumono->add(*menuitem);
        menuitem->show();
        menuitem->signal_activate().connect(
            sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &ToolBar::load_plug),
            menuitem->get_label()));
    }
    
    for (_iterator its = l_h.plug_stereo_list.begin();
                         its != l_h.plug_stereo_list.end(); its++) {
        std::string entry = *its;
        menuitem = Gtk::manage(new Gtk::MenuItem(entry, true));
        menuitem->set_use_underline(false);
        menustereo->add(*menuitem);
        menuitem->show();
        menuitem->signal_activate().connect(
            sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &ToolBar::load_plug),
            menuitem->get_label()));
    }
    
    
    for (_iterator its = l_h.plug_misc_list.begin();
                         its != l_h.plug_misc_list.end(); its++) {
        std::string entry = *its;
        menuitem = Gtk::manage(new Gtk::MenuItem(entry, true));
        menuitem->set_use_underline(false);
        menumisc->add(*menuitem);
        menuitem->show();
        menuitem->signal_activate().connect(
            sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &ToolBar::load_plug),
            menuitem->get_label()));
    }
    
    is_active = true;
}

void ToolBar::load_plug(Glib::ustring label) {
    
    int mark = label.find_first_of(' ');
    Glib::ustring lib = label.substr(1, mark-2);
    mark = label.find_last_of(' ');
    Glib::ustring ID = label.substr(mark, label.length() - mark);
    char** argv;
    argv = new char*[3];
    argv[0] = const_cast<char*>(lib.c_str());
    argv[1] = const_cast<char*>(lib.c_str());
    argv[2] = const_cast<char*>(ID.c_str());
    l_h.set_ex(false);
    l_h.set_go_on(true);
    l_h.jack_cleanup();
    delete l_h.m_window;
    l_h.m_window = NULL;
    l_h.sila_start(3,argv);
    delete []argv;
}

void ToolBar::on_browser() {
    l_h.set_ex(false);
    l_h.set_go_on(true);
    l_h.jack_cleanup();
    delete l_h.m_window;
    l_h.m_window = NULL;
    l_h.sila_start(0,NULL);
}

void ToolBar::on_close() {
    l_h.set_ex(true);
    l_h.set_go_on(false);
    delete l_h.m_window;
    l_h.m_window = NULL;
    l_h.jack_cleanup();
}

void ToolBar::on_stop() {
    l_h.deactivate_jack();
    is_active = false;
}

void ToolBar::on_play() {
    l_h.set_pause(false);
    if(!is_active) {
        l_h.activate_jack();
        is_active = true;
    }
}

void ToolBar::on_pause() {
    if(l_h.get_pause()) {
        l_h.set_pause(false);
    } else {
        l_h.set_pause(true);
    }
}

void ToolBar::on_menu_popup(Gtk::Menu *menu) {
    guint32 tim = gtk_get_current_event_time();
    menu->popup(2, tim);
}

/**********************************************************************/

Gtk::Window *LadspaHost::create_widgets() {
    Gtk::VBox *main_box = Gtk::manage(new Gtk::VBox);
    main_box->pack_start(*Gtk::manage(new ToolBar()),Gtk::PACK_SHRINK);
    Gtk::VBox *controller_box = Gtk::manage(new Gtk::VBox);
    
   /* controller_box->pack_start(*Gtk::manage(new Gtk::Label(l_h.l_p.descriptor->Label,
                                Pango::ALIGN_CENTER, Pango::ALIGN_CENTER,false)),
                                Gtk::PACK_EXPAND_WIDGET);*/
    for (int i = 0; i < (int)l_h.l_p.descriptor->PortCount; i++) {
        LADSPA_PortRangeHint hint = l_h.l_p.descriptor->PortRangeHints[i];
        LADSPA_PortRangeHintDescriptor hdescriptor = hint.HintDescriptor;
        if (LADSPA_IS_PORT_INPUT(l_h.l_p.descriptor->PortDescriptors[i]) &&
            LADSPA_IS_PORT_CONTROL(l_h.l_p.descriptor->PortDescriptors[i])) {
            if (LADSPA_IS_HINT_TOGGLED(hdescriptor)) {
                Gtk::HBox *box = Gtk::manage(new Gtk::HBox);
                box->set_spacing(10);
                box->pack_start(*Gtk::manage(new TButton( i)),
                                Gtk::PACK_SHRINK);
                box->pack_start(*Gtk::manage(new Gtk::HBox), Gtk::PACK_EXPAND_WIDGET);
                controller_box->pack_start(*box, Gtk::PACK_SHRINK);
            } else if (LADSPA_IS_HINT_INTEGER(hdescriptor)) {
                Gtk::HBox *box = Gtk::manage(new Gtk::HBox);
                box->set_spacing(10);
                box->pack_start(*Gtk::manage(new Gtk::Label(l_h.l_p.descriptor->PortNames[i],
                                Pango::ALIGN_RIGHT)), Gtk::PACK_SHRINK);
                box->pack_start(*Gtk::manage(new Spiner(i)),
                                Gtk::PACK_SHRINK);
                box->pack_start(*Gtk::manage(new Gtk::HBox), Gtk::PACK_EXPAND_WIDGET);
                controller_box->pack_start(*box, Gtk::PACK_SHRINK);
            } else {
                Gtk::HBox *box = Gtk::manage(new Gtk::HBox);
                box->set_spacing(10);
                controller_box->pack_start(*Gtk::manage(new Gtk::Label(l_h.l_p.descriptor->PortNames[i],
                                Pango::ALIGN_RIGHT)), Gtk::PACK_EXPAND_WIDGET);
                box->pack_start(*Gtk::manage(new Slider(i)),
                                Gtk::PACK_EXPAND_WIDGET);
                controller_box->pack_start(*box, Gtk::PACK_SHRINK);
            }
            Gtk::HSeparator *sep = Gtk::manage(new Gtk::HSeparator);
            controller_box->pack_start(*sep, Gtk::PACK_EXPAND_WIDGET);
        }
    }
    if (l_p.cnp > 12) {
        Gtk::ScrolledWindow *scrolled_window = Gtk::manage(new Gtk::ScrolledWindow);
        scrolled_window->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
        scrolled_window->set_shadow_type(Gtk::SHADOW_NONE);
        scrolled_window->add(*controller_box);
        scrolled_window->set_size_request(400, 300);
        main_box->pack_start(*scrolled_window, Gtk::PACK_EXPAND_WIDGET);
    } else { 
        main_box->pack_start(*controller_box, Gtk::PACK_EXPAND_WIDGET);
    }
    return((Gtk::Window*)main_box);
}

/*****************************************************************************/
