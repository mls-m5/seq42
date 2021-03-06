//----------------------------------------------------------------------------
//
//  This file is part of seq42.
//
//  seq42 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  seq42 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with seq42; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------

#ifndef SEQ42_PERFORM
#define SEQ42_PERFORM

class perform;

#include "globals.h"
#include "event.h"
#include "midibus.h"
#include "midifile.h"
#include "sequence.h"
#include "track.h"
#ifndef __WIN32__
#   include <unistd.h>
#endif
#include <pthread.h>


/* if we have jack, include the jack headers */
#ifdef JACK_SUPPORT
#include <jack/jack.h>
#include <jack/transport.h>
#endif

enum mute_op
{
    MUTE_TOGGLE = -1,
    MUTE_OFF = 0,
    MUTE_ON = 1
};

class perform
{
 private:
    /* vector of sequences */
    track *m_tracks[c_max_track];

    bool m_tracks_active[ c_max_track ];
    bool m_seqlist_open;
    bool m_seqlist_raise;

    bool m_was_active_edit[ c_max_track ];
    bool m_was_active_perf[ c_max_track ];
    bool m_was_active_names[ c_max_track ];

    /* our midibus */
    mastermidibus m_master_bus;

    /* pthread info */
    pthread_t m_out_thread;
    pthread_t m_in_thread;
    bool m_out_thread_launched;
    bool m_in_thread_launched;

    bool m_running;
    bool m_inputing;
    bool m_outputing;
    bool m_looping;

    bool m_playback_mode;

    int thread_trigger_width_ms; 

    long m_left_tick;
    long m_right_tick;
    long m_starting_tick;
    
    long m_tick;
    bool m_usemidiclock;
    bool m_midiclockrunning; // stopped or started
    int  m_midiclocktick;
    int  m_midiclockpos;
    
    void set_running( bool a_running );

    void set_playback_mode( bool a_playback_mode );

    condition_var m_condition_var;


#ifdef JACK_SUPPORT
    
    jack_client_t *m_jack_client;
    jack_nframes_t m_jack_frame_current,
                   m_jack_frame_last;
    jack_position_t m_jack_pos;
    jack_transport_state_t m_jack_transport_state;
    jack_transport_state_t m_jack_transport_state_last;
    double m_jack_tick;
    
#endif
    
    bool m_jack_running;
    bool m_jack_master;

    void inner_start( bool a_state );
    void inner_stop();

 public:
    bool is_running();

    unsigned int m_key_bpm_up;
    unsigned int m_key_bpm_dn;

    unsigned int m_key_start;
    unsigned int m_key_stop;

    unsigned int m_key_loop;

    perform();
    ~perform();

    void init( void );

    void clear_all( void );
    
    void launch_input_thread( void );
    void launch_output_thread( void );
    void init_jack( void );
    void deinit_jack( void );
    
    void add_track( track *a_track, int a_pref );
    void delete_track( int a_num );

    bool is_track_in_edit( int a_num );
    int get_track_index( track *a_track );
    
    void clear_track_triggers( int a_num  );

    long get_tick( ) { return m_tick; };

    void set_left_tick( long a_tick );
    long get_left_tick( void );

    void set_starting_tick( long a_tick );
    long get_starting_tick( void );

    void set_right_tick( long a_tick );
    long get_right_tick( void );

    void move_triggers( bool a_direction );
    void copy_triggers(  );
    
    void push_trigger_undo( void );
    void pop_trigger_undo( void );
    void pop_trigger_redo( void );

    void print();

    void start( bool a_state );
    void stop();

    void start_jack();
    void stop_jack();
    void position_jack( bool a_state );

    void off_sequences( void );
    void all_notes_off( void );

    void set_active(int a_track, bool a_active);
    void set_was_active( int a_track );
    bool is_active_track(int a_track);
    bool is_dirty_perf (int a_sequence);
    bool is_dirty_names (int a_sequence);
        
    void new_track( int a_track );

    /* plays all notes to Curent tick */
    void play( long a_tick );
    void set_orig_ticks( long a_tick  );

    track *get_track( int a_trk );
    sequence *get_sequence( int a_trk, int a_seq );

    void reset_sequences( void );

    void set_bpm(int a_bpm);
    int  get_bpm( );

    void set_swing_amount8(int a_swing_amount);
    int  get_swing_amount8( );
    void set_swing_amount16(int a_swing_amount);
    int  get_swing_amount16( );

    void set_looping( bool a_looping ){ m_looping = a_looping; };
 
    void set_song_mute( mute_op op );

    mastermidibus* get_master_midi_bus( );
    
    void output_func();
    void input_func();
    
    long get_max_trigger( void );

    bool save( const Glib::ustring& a_filename );
    bool load( const Glib::ustring& a_filename );

    friend class midifile;
    friend class optionsfile;
    friend class options;

    void set_seqlist_open( bool a_edit )
    {
        m_seqlist_open = a_edit;
    };

    bool get_seqlist_open( void )
    {
        return m_seqlist_open;
    };

    void set_seqlist_raise( bool a_raise )
    {
        m_seqlist_raise = a_raise;
    };

    bool get_seqlist_raise( void )
    {
        return m_seqlist_raise;
    };

    void apply_song_transpose (void);

#ifdef JACK_SUPPORT

    friend int jack_sync_callback(jack_transport_state_t state, 
                              jack_position_t *pos, void *arg);
    friend void jack_shutdown(void *arg);
    friend void jack_timebase_callback(jack_transport_state_t state, jack_nframes_t nframes, 
                                       jack_position_t *pos, int new_pos, void *arg);
#endif
};

/* located in perform.C */
extern void *output_thread_func(void *a_p);
extern void *input_thread_func(void *a_p);



#ifdef JACK_SUPPORT

int jack_sync_callback(jack_transport_state_t state, 
					   jack_position_t *pos, void *arg);
void print_jack_pos( jack_position_t* jack_pos );
void jack_shutdown(void *arg);
void jack_timebase_callback(jack_transport_state_t state, jack_nframes_t nframes, 
                            jack_position_t *pos, int new_pos, void *arg);
#endif


#endif
