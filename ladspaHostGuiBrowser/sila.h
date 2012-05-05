
#pragma once

#ifndef _SILA_H
#define _SILA_H

#include <jack/jack.h>
#include <ladspa.h>

#include <gtkmm.h>

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <list>

/*****************************************************************************/

template <class T>
inline std::string to_string(const T& t) {
    std::stringstream ss;
    ss << t;
    return ss.str();
}

/*****************************************************************************/

class LadspaBrowser : public Gtk::Window {
private:

    class LadspaHandle {
     public:
        Glib::ustring libname;
        Glib::ustring labelname;
        Glib::ustring UnicID;
    };

    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
     public:
        ModelColumns() { add(name);
                         add(label);}
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> label;
    };

    ModelColumns columns;
    LadspaHandle plhandle;
    Gtk::Dialog window;
    Gtk::VBox topBox;
    Gtk::Button b1;
    Gtk::Image stck1;
    Gtk::Image stck2;
    Gtk::ScrolledWindow scrollWindow;
    Gtk::ScrolledWindow detailWindow;
    Gtk::TextView m_TextView;
    Gtk::VPaned m_pane;
    Glib::RefPtr<Gtk::TextBuffer> m_refTextBuffer;
    Gtk::TreeView treeview;
    Glib::RefPtr<Gtk::TreeStore> model;
    void on_pressed1_event();
    void on_response(int);
    void on_cursor_changed();
    void create_list();
    void fill_buffers(Glib::ustring);
    Glib::ustring analysePlugin(const char * pcPluginFilename, 
        const char * pcPluginLabel, const int bVerbose);
    void unloadLADSPAPluginLibrary(void * pvLADSPAPluginLibrary);
    void * loadLADSPAPluginLibrary(const char * pcPluginFilename);
    void * dlopenLADSPA(const char * pcFilename, int iFlag);
    void on_treeview_row_activated(const Gtk::TreeModel::Path& path, 
        Gtk::TreeViewColumn*);

public:
    void show_browser(Glib::ustring *plfile, int *plid);
    explicit LadspaBrowser();
    ~LadspaBrowser();
}; 

/*****************************************************************************/

class LadspaHost {
 private:

    class LadspaPlug {
     public:
        int n_audio_in;
        int n_audio_out;
        int cnp;
        float *cpv;
        const LADSPA_Descriptor *descriptor;
        LADSPA_Handle lhandle;
        void *PluginHandle;
    };

    bool go_on;
    bool pause;
    bool ex;
    bool cmd_parser( int argc, char **argv, char **plfile,
               int *plid, Glib::ustring *error);
    bool init_jack(Glib::ustring *error);
    bool load_ladspa_plugin(char *PluginFilename, int id, Glib::ustring *error);
    bool sila_try(int argc, char **argv);
    bool on_delete_event(GdkEventAny*);
    int SR;
    int BZ;
    jack_client_t *jack_client;
    Gtk::Window *create_widgets();
    void file_browser(char **plfile, int *plid);
    std::string client_name;
    jack_port_t **ports;
    void get_connections();
    static int compute_callback(jack_nframes_t nframes, void *arg);
    static int buffer_size_callback(jack_nframes_t nframes, void *arg);
    static gboolean buffer_changed(void* arg);
    static void shut_down_callback(void *arg);

 public:
    std::list<Glib::ustring> plug_mono_list;
    std::list<Glib::ustring> plug_stereo_list;
    std::list<Glib::ustring> plug_misc_list;
    LadspaPlug l_p;
    LadspaBrowser *l_b;
    Gtk::Window *m_window;
    void set_ex(bool e_) {ex = e_;}
    void set_pause(bool pa_) {pause = pa_;}
    bool get_pause() {return pause;}
    int get_samplerate() {return SR;}
    void set_go_on(bool go_) {go_on = go_;}
    void sila_start(int argc, char **argv);
    void jack_cleanup();
    void activate_jack();
    void deactivate_jack();
};
extern LadspaHost l_h;

/*****************************************************************************/

#endif // _SILA_H
