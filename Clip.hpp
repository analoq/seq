#pragma once
#include <list>
#include <memory>
#include <cmath>
#include <unordered_map>
#include <functional>
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
  int time = 0;
  int quantize_time = TICKS_PER_BEAT / 4;
  int length_beats;
  list<shared_ptr<NoteOnEvent>> events;
  list<shared_ptr<NoteOnEvent>>::iterator it;

  list<NoteOnEvent> playing_notes;
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
      shared_ptr<NoteOnEvent> note_on(dynamic_pointer_cast<NoteOnEvent>(event));

      int event_time = quantize_time * 
                       round(time / static_cast<double>(quantize_time));
      if ( event_time >= length_beats * TICKS_PER_BEAT )
      {
        event_time -= length_beats * TICKS_PER_BEAT;
        note_on->time = event_time;
        events.push_front(note_on);
      }
      else
      {
        note_on->time = event_time;
        it = events.insert(it, note_on);
        ++ it;
      }
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

  void tick(function<void(const Event &)> callback)
  {
    // play notes
    if ( state == ON || state == TURNING_OFF )
    {
      while ( it != events.end() && time >= (*it)->time )
      {
        callback(**it);
        playing_notes.push_back(**it);
        ++ it;
      }
    }

    // release playing notes
    for ( auto pnit = playing_notes.begin();
          pnit != playing_notes.end(); )
    {
      NoteOnEvent &playing_note = *pnit;
      playing_note.length -- ;

      if ( !playing_note.length )
      {
        callback(NoteOffEvent(shared_ptr<NoteOnEvent>(new NoteOnEvent {playing_note})) );
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

