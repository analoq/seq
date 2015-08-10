#pragma once
#include <list>
#include <memory>
#include <cmath>
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
    shared_ptr<Event> event;
  };

  int time = 0;
  int quantize_time = 6;
  int length_beats;
  list<TimedEvent> events;
  list<TimedEvent>::iterator it;

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
    int event_time = time;
    if ( typeid(*event) == typeid(NoteOnEvent) )
      event_time = round(time / static_cast<double>(quantize_time))
                   * quantize_time;
    if ( event_time >= length_beats * TICKS_PER_BEAT )
    {
      event_time -= length_beats * TICKS_PER_BEAT;
      events.push_front( TimedEvent{event_time, event} );
    }
    else
      events.insert(it, TimedEvent{event_time, event} );
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

  void tick(MidiDevice &device, int channel)
  {
    if ( state == ON || state == TURNING_OFF )
    {
      while ( it != events.end() && time >= (*it).time )
      {
        device.write(channel, *(*it).event);
        it ++;
      }
    }

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

