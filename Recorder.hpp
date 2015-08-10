#pragma once
#include "Track.hpp"
#include "Clip.hpp"

class Recorder
{
private:
  Clip *active_clip = nullptr;
  Track *active_track = nullptr;

public:
  Clip *getClip()
  {
    return active_clip;
  }

  void setClip(Clip &clip)
  {
    active_clip = &clip;
  }

  void clearClip()
  {
    active_clip = nullptr;
  }

  void setTrack(Track &track)
  {
    active_track = &track;
  }

  Track *getTrack()
  {
    return active_track;
  }

  void addDevice(MidiDevice &device)
  {
    thread t(Recorder::read, ref(*this), ref(device));
    t.detach();
  }

  static void read(Recorder &recorder, MidiDevice &device)
  {
    while ( true )
    {
      shared_ptr<Event> event { device.read() };

      if ( recorder.active_track != nullptr )
        recorder.active_track->send(*event);

      if ( recorder.active_clip != nullptr &&
           recorder.active_clip->getState() == ClipState::ON &&
           ( typeid(*event) == typeid(NoteOnEvent) ||
             typeid(*event) == typeid(NoteOffEvent) ) )
        recorder.active_clip->addEvent(move(event));
    }
  }
};
