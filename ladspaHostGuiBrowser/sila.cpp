

#include "sila.h"

/*****************************************************************************/

int LadspaHost::compute_callback(jack_nframes_t nframes, void *arg) {
    LadspaHost& self = *static_cast<LadspaHost*>(arg);

    if(!self.pause) {
        for (int i = 0; i < static_cast<int>(self.l_p.descriptor->PortCount); i++) {
            if (LADSPA_IS_PORT_AUDIO(self.l_p.descriptor->PortDescriptors[i])) {
                self.l_p.descriptor->connect_port (self.l_p.lhandle, i,
                    (float *)jack_port_get_buffer(self.ports[i], nframes));
            }
        }

        self.l_p.descriptor->run(self.l_p.lhandle, nframes);
    } else { //pause
        float *inports[self.l_p.n_audio_in];
        float *outports[self.l_p.n_audio_out];
        int in = 0;
        int out = 0;
 
        for (int i = 0; i < static_cast<int>(self.l_p.descriptor->PortCount); i++) {
            if (LADSPA_IS_PORT_AUDIO(self.l_p.descriptor->PortDescriptors[i]) &&
                LADSPA_IS_PORT_INPUT(self.l_p.descriptor->PortDescriptors[i])) {
                inports[in] = (float *)jack_port_get_buffer(self.ports[i], nframes);
                in++;
            } else if (LADSPA_IS_PORT_AUDIO(self.l_p.descriptor->PortDescriptors[i]) &&
                LADSPA_IS_PORT_OUTPUT(self.l_p.descriptor->PortDescriptors[i])) {
                outports[out] = (float *)jack_port_get_buffer(self.ports[i], nframes);
                out++;
            }
        }
        if (out>0&&in>0) {
            if (out <= in) {
                for(int i=0;i<out;i++) {
                    (void)memcpy(outports[i], inports[i], sizeof(float)*nframes);
                }
            } else if (out > in) {
                int j = 0;
                for(int i=0;i<out;i++) {
                    (void)memcpy(outports[i], inports[j], sizeof(float)*nframes);
                    j++;
                    if(j>=in)j=0;
                } 
            }
        } else if (out>0) {
            for(int j=0;j<out;j++) {
                (void)memset(outports[j], 0.0, sizeof(float)*nframes);
            }
        }
    }
    return 0;
}

/*****************************************************************************/

gboolean LadspaHost::buffer_changed(void* arg) {
    LadspaHost& self = *static_cast<LadspaHost*>(arg);
    self.set_ex(false);
    self.set_go_on(true);
    self.jack_cleanup();
    delete self.m_window;
    self.m_window = NULL;
    self.sila_start(0,NULL);
    return false;
}

/*****************************************************************************/

int LadspaHost::buffer_size_callback(jack_nframes_t nframes, void *arg) {
    LadspaHost& self = *static_cast<LadspaHost*>(arg);
    int _bz = jack_get_buffer_size(self.jack_client);
    if (_bz != self.BZ) {
        self.set_pause(true);
        g_idle_add(buffer_changed,(void*)arg);
    }
    return 0;
}

/*****************************************************************************/

void LadspaHost::shut_down_callback(void *arg) {
    LadspaHost& self = *static_cast<LadspaHost*>(arg);
    delete [] self.l_p.cpv;
    delete [] self.ports;
    fprintf(stderr,"Exit: JACK shut us down\n");
    if(!Gtk::Main::instance()->level()) {
        delete self.l_b;
        delete self.m_window;
        exit(1);
    } else {
        Gtk::Main::quit();
    }
}

/*****************************************************************************/

void LadspaHost::deactivate_jack() {
    get_connections();
    jack_deactivate(jack_client);
}

/*****************************************************************************/

void LadspaHost::activate_jack() {
    if (jack_activate(jack_client)) {
        delete [] l_p.cpv;
        delete [] ports;
        fprintf(stderr,"Exit: could not activate JACK processing\n");
        if(!Gtk::Main::instance()->level()) {
            delete l_b;
            delete m_window;
            exit(1);
        } else {
            Gtk::Main::quit();
        }
    } else {
        std::string portname;
        int a = 1;
        int b = 1;
        int c = 0;
        int d = 0;
        for (int i = 0; i < static_cast<int>(l_p.descriptor->PortCount); i++) {
            if (LADSPA_IS_PORT_CONTROL(l_p.descriptor->
                                   PortDescriptors[i]) &&
                LADSPA_IS_PORT_INPUT(l_p.descriptor->PortDescriptors[i])) {
                continue;
            }
            if (LADSPA_IS_PORT_INPUT(l_p.descriptor->PortDescriptors[i])) {
                portname = "SHINPUT";
                c++;
                portname += to_string(a);
                portname += "_";
                portname += to_string(c);
                while(getenv(portname.c_str()) != NULL){
                    jack_connect(jack_client, getenv(portname.c_str()), jack_port_name(ports[i]));
                    portname = "SHINPUT";
                    c++;
                    portname += to_string(a);
                    portname += "_";
                    portname += to_string(c);
                }
                c--;
                a++;
            } else if (LADSPA_IS_PORT_OUTPUT(l_p.descriptor->PortDescriptors[i])){
                portname = "SHOUTPUT";
                d++;
                portname += to_string(b);
                portname += "_";
                portname += to_string(d);
                while (getenv(portname.c_str()) != NULL) {
                    jack_connect(jack_client, jack_port_name(ports[i]), getenv(portname.c_str()));
                    portname = "SHOUTPUT";
                    d++;
                    portname += to_string(b);
                    portname += "_";
                    portname += to_string(d);
                }
                d--;
                b++;
            }
        }
    }
}

/*****************************************************************************/

bool LadspaHost::init_jack(Glib::ustring *message) {
    client_name = "sila_";
    jack_status_t jack_status;
    int i;
    unsigned long flags;
    l_p.n_audio_in = 0;
    l_p.n_audio_out = 0;
    l_p.cnp = 0;
    l_p.cpv = NULL;
    ports = NULL;

    client_name += l_p.descriptor->Label;
    jack_client = jack_client_open(client_name.c_str(),
                                          JackNullOption, &jack_status);
    if (jack_status) {
        *message = "Error: could not connect to JACK";
        return false;
    }

    try {
        ports = new jack_port_t* [l_p.descriptor->PortCount]();
        l_p.cpv = new float [l_p.descriptor->PortCount * sizeof(float)];
    } catch(...) {
        *message = "Error: could not allocate memory";
        return false;
    }

    for (i = 0; i < static_cast<int>(l_p.descriptor->PortCount); i++) {
        if (LADSPA_IS_PORT_CONTROL(l_p.descriptor->
                                   PortDescriptors[i]) &&
            LADSPA_IS_PORT_INPUT(l_p.descriptor->PortDescriptors[i])) {
            l_p.cnp++;
            continue;
        }

        if (LADSPA_IS_PORT_INPUT(l_p.descriptor->PortDescriptors[i]) &&
            LADSPA_IS_PORT_AUDIO(l_p.descriptor->PortDescriptors[i])) {
            flags = JackPortIsInput;
            l_p.n_audio_in++;
            
        } else if (LADSPA_IS_PORT_OUTPUT(l_p.descriptor->PortDescriptors[i]) &&
            LADSPA_IS_PORT_AUDIO(l_p.descriptor->PortDescriptors[i])){
            flags = JackPortIsOutput;
            l_p.n_audio_out++;
        }

        ports[i] =
            jack_port_register(jack_client,
                               l_p.descriptor->PortNames[i],
                               JACK_DEFAULT_AUDIO_TYPE,
                               flags, 0);

        if (!ports[i]) {
            delete [] l_p.cpv;
            delete [] ports;
            *message = "Error: could not register JACK ports";
            return false;
        }
    }

    if (!l_p.n_audio_in) {
        fprintf(stderr,"Warning: plugin have no input JACK port \n");
    } 
    if (!l_p.n_audio_out) {
        fprintf(stderr,"Warning: plugin have no output JACK port \n");
    }

    jack_set_process_callback(jack_client, compute_callback, this);
    jack_set_buffer_size_callback(jack_client, buffer_size_callback, this);
    jack_on_shutdown(jack_client, shut_down_callback, this);

    SR = jack_get_sample_rate(jack_client);
    BZ = jack_get_buffer_size(jack_client);

    l_p.lhandle = l_p.descriptor->instantiate(l_p.descriptor, SR);
    if (!l_p.lhandle) {
        *message = "Error: could not instantiate the plugin.";
        jack_cleanup();
        set_go_on(true);
        return false;
    }

    for (i = 0; i < static_cast<int>(l_p.descriptor->PortCount); i++) {
        if (LADSPA_IS_PORT_CONTROL(l_p.descriptor-> PortDescriptors[i]) &&
            LADSPA_IS_PORT_INPUT(l_p.descriptor->PortDescriptors[i])) {
            l_p.cpv[i] = 0.0;
            l_p.descriptor->connect_port(l_p.lhandle, i,&l_p.cpv[i]);
        }
        if (LADSPA_IS_PORT_CONTROL(l_p.descriptor-> PortDescriptors[i]) &&
            LADSPA_IS_PORT_OUTPUT(l_p.descriptor->PortDescriptors[i])) {
            l_p.cpv[i] = 0.0;
            l_p.descriptor->connect_port(l_p.lhandle, i,&l_p.cpv[i] );
        }
    }

    if (l_p.descriptor->activate) {
        l_p.descriptor->activate(l_p.lhandle);
    }

    return true;
}

/*****************************************************************************/

void LadspaHost::get_connections() {
    const char** port = NULL;
    std::string portname;
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    for (int i = 0; i < static_cast<int>(l_p.descriptor->PortCount); i++) {
        if (LADSPA_IS_PORT_CONTROL(l_p.descriptor-> PortDescriptors[i]) &&
            LADSPA_IS_PORT_INPUT(l_p.descriptor->PortDescriptors[i])) {
            continue;
        }
        if (LADSPA_IS_PORT_INPUT(l_p.descriptor->PortDescriptors[i])) {
            a++;
            if (jack_port_connected(ports[i])) {
                port = jack_port_get_connections(ports[i]);
                for ( int j = 0; port[j] != NULL; j++) {
                    portname = "SHINPUT";
                    c++;
                    portname += to_string(a);
                    portname += "_";
                    portname += to_string(c);
                    setenv(portname.c_str(),port[j],1);
                }
            }
        } else if (LADSPA_IS_PORT_OUTPUT(l_p.descriptor->PortDescriptors[i])){
            b++;
            if (jack_port_connected(ports[i])) {
                port = jack_port_get_connections(ports[i]);
                for ( int j = 0; port[j] != NULL; j++) {
                    portname = "SHOUTPUT";
                    d++;
                    portname += to_string(b);
                    portname += "_";
                    portname += to_string(d);
                    setenv(portname.c_str(),port[j],1);
                }
            }
        }
    }
    free(port);
}

/*****************************************************************************/

void LadspaHost::jack_cleanup() {
    if (jack_client) {
        if(!ex) {
            get_connections();
        }

        jack_deactivate(jack_client);

        if (l_p.descriptor->deactivate) {
            l_p.descriptor->deactivate(l_p.lhandle);
        }
        if (l_p.descriptor->cleanup) {
            l_p.descriptor->cleanup(l_p.lhandle);
        }

        for (int i = 0; i < static_cast<int>(l_p.descriptor->PortCount); i++) {
            if (LADSPA_IS_PORT_CONTROL(l_p.descriptor-> PortDescriptors[i]) &&
                LADSPA_IS_PORT_INPUT(l_p.descriptor->PortDescriptors[i])) {
                continue;
            }
            if (LADSPA_IS_PORT_INPUT(l_p.descriptor->PortDescriptors[i])) {
                jack_port_unregister(jack_client, ports[i]);
            } else if (LADSPA_IS_PORT_OUTPUT(l_p.descriptor->PortDescriptors[i])){
                jack_port_unregister(jack_client, ports[i]);
            }
        }
        
        if (l_p.PluginHandle) dlclose(l_p.PluginHandle);
        jack_client_close(jack_client);
        jack_client = NULL;
        delete [] l_p.cpv;
        delete [] ports;
    }
    if (ex) {
        Gtk::Main::quit();
    }
}

/*****************************************************************************/

bool LadspaHost::load_ladspa_plugin(char *PluginFilename, 
               int id, Glib::ustring *message) {

    LADSPA_Descriptor_Function phDescriptorFunction;
    const LADSPA_Descriptor * phDescriptor;
    char * pch;
    std::string path;

    pch=strchr(PluginFilename,'/');
    if(pch == NULL) {
        char * p = getenv ("LADSPA_PATH");
        if (p != NULL) {
            path = getenv ("LADSPA_PATH");
        } else {
            setenv("LADSPA_PATH","/usr/lib/ladspa",0);
            setenv("LADSPA_PATH","/usr/local/lib/ladspa",0);
            path = getenv("LADSPA_PATH");
        } 
        path += "/";
        path += PluginFilename;
        PluginFilename = const_cast<char*>(path.c_str());
    }
    
    l_p.PluginHandle = dlopen(PluginFilename, RTLD_NOW | RTLD_LOCAL);
    if (!l_p.PluginHandle) {
        *message = dlerror();
        jack_cleanup();
        set_go_on(true);
        return false;
    }

    phDescriptorFunction 
        = (LADSPA_Descriptor_Function)dlsym(l_p.PluginHandle, "ladspa_descriptor");
    if (!phDescriptorFunction) {
        *message = dlerror();
        jack_cleanup();
        set_go_on(true);
        return false;
    }

    for (int i = 0;; i++) {
        phDescriptor = phDescriptorFunction(i);
        if (!phDescriptor) {
            *message = "Error: ID not found in the given Plugin";
            jack_cleanup();
            set_go_on(true);
            return false;
        } else if (static_cast<int>(phDescriptor->UniqueID) == id) {
            l_p.descriptor = phDescriptor;
            break;
        }
    }
    return true;
}

/*****************************************************************************/

void LadspaHost::file_browser(char **plfile, int *plid) {
   *plid = 0;
    Glib::ustring pl;
    std::string path;
    l_b->show_browser(&pl,plid);
    char * p = getenv ("LADSPA_PATH");
    if (p != NULL) {
        path = getenv ("LADSPA_PATH");
    } else {
        setenv("LADSPA_PATH","/usr/lib/ladspa",0);
        setenv("LADSPA_PATH","/usr/local/lib/ladspa",0);
        path = getenv("LADSPA_PATH");
    } 
    path += "/";
    path +=  pl.c_str();    
    *plfile = const_cast<char*>(path.c_str());
}

/*****************************************************************************/

bool LadspaHost::cmd_parser(int argc, char **argv,
            char **plfile, int *plid, Glib::ustring *message) {

    if (argc >= 3) {
        *plfile = argv[1];
        *plid = atoi(argv[2]);
    } else {
        file_browser(plfile, plid);
    }

    if(*plid == 0) {
        *message = "Error: no plugin filename and no plugin ID given";
        return false;
    }
    return true;
}

/*****************************************************************************/

bool LadspaHost::on_delete_event(GdkEventAny*) {
    jack_cleanup();
    return false;
}

/*****************************************************************************/

bool LadspaHost::sila_try(int argc, char **argv) {
    Glib::ustring message;
    char *plfile;
    int plid;

    jack_client = 0;

    if(!cmd_parser(argc, argv, &plfile, &plid, &message) ||
        !load_ladspa_plugin(plfile, plid, &message) ||
        !init_jack(&message)) {
        fprintf(stderr, "%s.\n", message.c_str());
        return false;
    }
    return true;
}

/*****************************************************************************/

void LadspaHost::sila_start(int argc, char **argv) {

    if (sila_try(argc, argv)) {
        set_pause(false);
        m_window = new Gtk::Window;
        m_window->signal_delete_event().connect(
            sigc::mem_fun(*this,&LadspaHost::on_delete_event)); 
        m_window->add(*create_widgets());
        m_window->resize(500, m_window->get_height());
        m_window->set_title(client_name.c_str());
        m_window->set_position(Gtk::WIN_POS_CENTER);
        m_window->show_all_children();
        activate_jack();
        m_window->show();
        set_ex(true);
    } else if (go_on) {
        set_go_on(false);
        sila_start(0,NULL);
    } else {
        if(!Gtk::Main::instance()->level()) {
            delete l_b;
            delete l_h.m_window;
            exit(1);
        } else {
            Gtk::Main::quit();
        }
    }
}

/*****************************************************************************/
