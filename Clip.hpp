#pragma once
#include <list>
#include <memory>
#include <cmath>
#include <map>
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
  struct TimedEvent
  {
    int time;
    int length;
    shared_ptr<NoteOnEvent> note_on;
  };

  struct OpenNote
  {
    shared_ptr<TimedEvent> timed_event;
    int original_time;
  };

  int time = 0;
  int quantize_time = 6;
  int length_beats;
  list<shared_ptr<TimedEvent>> events;
  list<shared_ptr<TimedEvent>>::iterator it;

  map<uint8_t, OpenNote> open_notes;
  list<TimedEvent> playing_notes;

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
      shared_ptr<NoteOnEvent> note_on { dynamic_pointer_cast<NoteOnEvent>(event) };
      shared_ptr<TimedEvent> timed_event = shared_ptr<TimedEvent>(new TimedEvent { time, 0, note_on });


      (*timed_event).time = quantize_time * 
                            round(time / static_cast<double>(quantize_time));
      if ( (*timed_event).time >= length_beats * TICKS_PER_BEAT )
      {
        (*timed_event).time -= length_beats * TICKS_PER_BEAT;
        events.push_front( timed_event );
      }
      else
        events.insert(it, timed_event );

      open_notes[(*note_on).note] = OpenNote { timed_event, time };
    }
    else if ( typeid(*event) == typeid(NoteOffEvent) )
    {
      shared_ptr<NoteOffEvent> note_off { dynamic_pointer_cast<NoteOffEvent>(event) };
      OpenNote &open_note = open_notes[(*note_off).note];
      open_notes.erase((*note_off).note);
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

  void tick(shared_ptr<MidiDevice> device, int channel)
  {
    // play notes
    if ( state == ON || state == TURNING_OFF )
    {
      while ( it != events.end() && time >= (**it).time )
      {
        (*device).write(channel, *(**it).note_on);
        playing_notes.push_back(**it);
        it ++;
      }
    }

    // increment open notes
    for_each(open_notes.begin(), open_notes.end(),
         [](pair<const uint8_t, OpenNote> &open_note_pair)
         {
            (*open_note_pair.second.timed_event).length ++;
         }
      );

    // release playing notes
    for ( auto pnit = playing_notes.begin();
          pnit != playing_notes.end(); )
    {
      TimedEvent &playing_note = *pnit;
      playing_note.length -- ;
      if ( !playing_note.length )
      {
        (*device).write(channel, NoteOffEvent { (*playing_note.note_on).note } );
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

