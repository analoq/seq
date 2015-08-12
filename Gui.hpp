#pragma once
#include <thread>
#include <ncurses.h>

#include "Session.hpp"

using namespace std;

class Gui
{
private:
  Session &session;

public:
  Gui(Session &s) : session(s)
  {
  }

  ~Gui()
  {
    endwin();
  }

  void run()
  {
    bool done = false;

    initscr();
    noecho();
    raw();
    curs_set(0);
    keypad(stdscr, true);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);

    refresh();

    thread t { Gui::update, ref(session) };
    t.detach();

    while ( !done )
    {
      switch ( getch() )
      {
        case '1':
          session.recorder.clearClip();
          session.recorder.setTrack(&session.player.getTrack(0));
          break;
        case '2':
          session.recorder.clearClip();
          session.recorder.setTrack(&session.player.getTrack(1));
          break;
        case '3':
          session.recorder.clearClip();
          session.recorder.setTrack(&session.player.getTrack(2));
          break;

        case 'q':
          session.player.getTrack(0).getClip(0).toggle();
          session.recorder.setTrack(&session.player.getTrack(0));
          session.recorder.setClip(session.player.getTrack(0).getClip(0));
          break;
        case 'w':
          session.player.getTrack(1).getClip(0).toggle();
          session.recorder.setTrack(&session.player.getTrack(1));
          session.recorder.setClip(session.player.getTrack(1).getClip(0));
          break;
        case 'e':
          session.player.getTrack(2).getClip(0).toggle();
          session.recorder.setTrack(&session.player.getTrack(2));
          session.recorder.setClip(session.player.getTrack(2).getClip(0));
          break;

        case 'a':
          session.player.getTrack(0).getClip(1).toggle();
          session.recorder.setTrack(&session.player.getTrack(0));
          session.recorder.setClip(session.player.getTrack(0).getClip(1));
          break;
        case 's':
          session.player.getTrack(1).getClip(1).toggle();
          session.recorder.setTrack(&session.player.getTrack(1));
          session.recorder.setClip(session.player.getTrack(1).getClip(1));
          break;
        case 'd':
          session.player.getTrack(2).getClip(1).toggle();
          session.recorder.setTrack(&session.player.getTrack(2));
          session.recorder.setClip(session.player.getTrack(2).getClip(1));
          break;

        case 'z':
          session.player.getTrack(0).getClip(2).toggle();
          session.recorder.setTrack(&session.player.getTrack(0));
          session.recorder.setClip(session.player.getTrack(0).getClip(2));
          break;
        case 'x':
          session.player.getTrack(1).getClip(2).toggle();
          session.recorder.setTrack(&session.player.getTrack(1));
          session.recorder.setClip(session.player.getTrack(1).getClip(2));
          break;
        case 'c':
          session.player.getTrack(2).getClip(2).toggle();
          session.recorder.setTrack(&session.player.getTrack(2));
          session.recorder.setClip(session.player.getTrack(0).getClip(2));
          break;

        case KEY_BACKSPACE:
          if ( session.recorder.getClip() )
            session.recorder.getClip()->erase(); 
          break;

        case 27:
          done = true;
          break;
      }
    }
  }

  static void update(Session &session)
  {
    WINDOW *session_win { newwin(5, COLS, 0, 0) };
    wbkgd(session_win, COLOR_PAIR(1));
    box(session_win, 0, 0);
    mvwprintw(session_win, 1, 1, "Tempo:");

    vector<vector<WINDOW *>> clip_windows;
    const int clip_height = 7;
    for ( int j = 0; j < session.player.getTrackCount(); j ++ )
    {
      WINDOW *track_win { newwin(5, 25, 5, 25*j) };
      box(track_win, 0, 0);
      mvwprintw(track_win, 1, 1, "Track: %-12s", session.player.getTrack(j).getName().c_str() );
      mvwprintw(track_win, 2, 1, "Device: %-12s", session.player.getTrack(j).getDeviceName().c_str() );
      mvwprintw(track_win, 3, 1, "Channel: %-2d", session.player.getTrack(j).getChannel() );
      wrefresh(track_win);

      vector<WINDOW *> col_windows;
      for ( int i = 0; i < session.player.getTrack(j).getClipCount(); i ++ )
      {
        WINDOW *win = newwin(clip_height, 25, 10 + i*clip_height, 25*j);
        box(win, 0, 0);
        wrefresh(win);
        col_windows.push_back(win);
      }
      clip_windows.push_back(col_windows);
    }

    while ( true )
    {
      mvwprintw(session_win, 1, 8, "%f", session.player.getBPM());
      wrefresh(session_win);

      for ( int j = 0; j < session.player.getTrackCount(); j ++ )
      {
        for ( int i = 0; i < clip_windows[j].size(); i ++ )
        {
          const Clip &clip = session.player.getTrack(j).getClip(i);
          WINDOW *win = clip_windows.at(j).at(i);

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
      }

      this_thread::sleep_for(chrono::milliseconds(100));
    }          
  }
};

