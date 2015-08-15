#pragma once
#include <list>
#include <memory>
#include <cmath>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include "Event.hpp"

using namespace std;

enum ClipState
{
  ON,
  OFF,
  TURNING_ON,
  TURNING_OFF
};

class Clip
{
private:
  struct OpenNote
  {
    TimedNoteOnEvent &timed_event;
    int original_time;
  };

  int time = 0;
  int quantize_time = TICKS_PER_BEAT / 4;
  int length_beats;
  list<TimedNoteOnEvent> events;
  list<TimedNoteOnEvent>::iterator it;

  unordered_map<uint8_t, OpenNote> open_notes;
  list<TimedNoteOnEvent> playing_notes;

  ClipState state = OFF;

public:
  Clip(int l) : length_beats(l), it(events.begin())
  {
  }

  ClipState getState() const
  {
    return state;
  }

  int getTime() const
  {
    return time;
  }

  int getLength() const
  {
    return length_beats;
  }

  int getEventCount() const
  {
    return events.size();
  }

  void toggle()
  {
    switch ( state )
    {
      case OFF:
        state = TURNING_ON;
        break;
      case TURNING_ON:
        state = OFF;
        break;
      case ON:
        state = TURNING_OFF;
        break;
      case TURNING_OFF:
        state = ON;
        break;
    }
  }

  void erase()
  {
    it = events.end();
    events.clear();
  }

  void addEvent(shared_ptr<Event> event)
  {
    if ( typeid(*event) == typeid(NoteOnEvent) )
    {
      NoteOnEvent &note_on(*dynamic_pointer_cast<NoteOnEvent>(event));

      int event_time = quantize_time * 
                       round(time / static_cast<double>(quantize_time));
      if ( event_time >= length_beats * TICKS_PER_BEAT )
      {
        event_time -= length_beats * TICKS_PER_BEAT;
        events.emplace_front( event_time, note_on );
        open_notes.emplace(note_on.note, OpenNote { events.front(), time });
      }
      else
      {
        it = events.emplace(it, event_time, note_on);
        open_notes.emplace(note_on.note, OpenNote { *it, time });
        ++ it;
      }
    }
    else if ( typeid(*event) == typeid(NoteOffEvent) )
    {
      NoteOffEvent &note_off(*dynamic_pointer_cast<NoteOffEvent>(event));
      OpenNote &open_note = open_notes.at(note_off.note);
      open_notes.erase(note_off.note);
    }
  }

  void setLength(int l)
  {
    length_beats = l;
  }

  void start()
  {
    time = 0;
    it = events.begin();
  }

  void tick(function<void(const Event &event)> callback)
  {
    // play notes
    if ( state == ON || state == TURNING_OFF )
    {
      while ( it != events.end() && time >= (*it).time )
      {
        callback(*it);
        playing_notes.push_back(*it);
        ++ it;
      }
    }

    // increment open notes
    for_each(open_notes.begin(), open_notes.end(),
         [](pair<const uint8_t, OpenNote> &open_note_pair)
         {
            open_note_pair.second.timed_event.length ++;
         }
      );

    // release playing notes
    for ( auto pnit = playing_notes.begin();
          pnit != playing_notes.end(); )
    {
      TimedNoteOnEvent &playing_note = *pnit;
      playing_note.length -- ;
      if ( !playing_note.length )
      {
        callback( NoteOffEvent { playing_note.note } );
        pnit = playing_notes.erase(pnit);
      }
      else
        ++pnit;
    }

    // increment time
    time ++;

    if ( time >= length_beats * TICKS_PER_BEAT )
    {
      time = 0;
      it = events.begin();
      switch ( state )
      {
        case TURNING_ON:
          state = ON;
          break;
        case TURNING_OFF:
          state = OFF;
          break;
      }
    }
  }
};

