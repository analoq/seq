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
  int time = 0;
  int quantize_time = TICKS_PER_BEAT / 4;
  int length_beats;
  list<TimedNoteOnEvent> events;
  list<TimedNoteOnEvent>::iterator it;

  unordered_map<uint8_t, TimedNoteOnEvent &> open_notes;

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
        open_notes.emplace(note_on.note, events.front());
      }
      else
      {
        it = events.emplace(it, event_time, note_on);
        open_notes.emplace(note_on.note, *it);
        ++ it;
      }
    }
    else if ( typeid(*event) == typeid(NoteOffEvent) )
    {
      NoteOffEvent &note_off(*dynamic_pointer_cast<NoteOffEvent>(event));
      open_notes.erase((*note_off.note_on).note);
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
      while ( it != events.end() && time >= (*it).time )
      {
        callback(*it);
        ++ it;
      }
    }

    // increment open notes
    for_each(open_notes.begin(), open_notes.end(),
         [](pair<const uint8_t, TimedNoteOnEvent &> &open_note_pair)
         {
            open_note_pair.second.length ++;
         }
      );

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

