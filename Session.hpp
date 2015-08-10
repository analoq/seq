#include "MidiDevice.hpp"
#include "Track.hpp"
#include "Player.hpp"
#include "Recorder.hpp"

#include <ncurses.h>
#include <memory>
#include <thread>
#include <vector>

class Session
{
private:
  MidiDevice device{"hw:3,0,0"};
  MidiDevice clock_device{"hw:3,0,8"};
  Track track{device, 0, "1 (Piano)"};

  Recorder recorder;

public:
  Player player {105.0};

  Session()
  {
    track.setPatch(63, 0, 1);

    player.addClockDevice(clock_device);
    player.addTrack(track);

    recorder.addDevice(device);
    recorder.setTrack(track);
  }

  void run()
  {
    player.start();
    gui();
    player.stop();
  }

  void gui()
  {
    bool done = false;

    initscr();
    noecho();
    raw();
    curs_set(0);
    keypad(stdscr, true);
    refresh();

    thread t { Session::gui_update, ref(*this) };
    t.detach();

    while ( !done )
    {
      switch ( getch() )
      {
        case '1':
          recorder.clearClip();
          break;

        case 'q':
          track.getClip(0).toggle();
          recorder.setClip(track.getClip(0));
          break;

        case 'a':
          track.getClip(1).toggle();
          recorder.setClip(track.getClip(1));
          break;

        case 'z':
          track.getClip(2).toggle();
          recorder.setClip(track.getClip(2));
          break;

        case KEY_BACKSPACE:
          if ( recorder.getClip() )
            recorder.getClip()->erase(); 
          break;

        case 27:
          done = true;
          break;
      }
    }
    endwin();
  }

  static void gui_update(Session &session)
  {
    WINDOW *session_win { newwin(5, COLS, 0, 0) };
    box(session_win, 0, 0);
    mvwprintw(session_win, 1, 1, "Tempo:");

    WINDOW *track_win { newwin(5, 25, 5, 0) };
    box(track_win, 0, 0);
    mvwprintw(track_win, 1, 1, "Track: %-12s", session.track.getName().c_str() );
    wrefresh(track_win);

    vector<WINDOW *> clip_windows;
    const int clip_height = 7;
    for ( int i = 0; i < session.track.getClipCount(); i ++ )
    {
      WINDOW *win = newwin(clip_height, 25, 10 + i*clip_height, 0);
      box(win, 0, 0);
      wrefresh(win);
      clip_windows.push_back( win );
    }

    while ( true )
    {
      mvwprintw(session_win, 1, 8, "%f", session.player.getBPM());
      wrefresh(session_win);

      for ( int i = 0; i < clip_windows.size(); i ++ )
      {
        const Clip &clip = session.track.getClip(i);
        WINDOW *win = clip_windows.at(i);

        // state
        string state;
        switch ( clip.getState() )
        {
          case ClipState::ON:
            state = "ON";
            break;
          case ClipState::OFF:
            state = "OFF";
            break;
          case ClipState::TURNING_ON:
            state = "TURNING ON";
            break;
          case ClipState::TURNING_OFF:
            state = "TURNING OFF";
            break;
          default:
            state = "UNKNOWN";
        }
        mvwprintw(win, 1, 1, "State: %-12s", state.c_str());

        // time
        int beat { clip.getTime() / TICKS_PER_BEAT + 1 };
        mvwprintw(win, 2, 1, "Position: %-2d", beat);

        // length
        mvwprintw(win, 3, 1, "Length: %-2d", clip.getLength());

        // events
        mvwprintw(win, 4, 1, "Events: %-3d", clip.getEventCount());

        // active clip
        if ( session.recorder.getClip() == &clip )
          mvwprintw(win, 5, 1, "Active: ON ");
        else
          mvwprintw(win, 5, 1, "Active: OFF");

        wrefresh(win);
      }

      this_thread::sleep_for(chrono::milliseconds(100));
    }          
  }
};

