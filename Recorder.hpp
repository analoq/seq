#pragma once
#include <memory>
#include "Track.hpp"
#include "Clip.hpp"

class Recorder
{
private:
  Clip *active_clip;
  Track *active_track;

  list<shared_ptr<MidiDevice>> devices;
public:
  Recorder() : active_clip{nullptr}, active_track{nullptr}
  {
  }

  Clip *getClip() const
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

  void setTrack(Track *track)
  {
    active_track = track;
  }

  Track *getTrack() const
  {
    return active_track;
  }

  void addDevice(shared_ptr<MidiDevice> device)
  {
    devices.push_back(device);
  }

  void start()
  {
    for ( shared_ptr<MidiDevice> device : devices )
    {
      thread t(Recorder::read, ref(*this), device);
      t.detach();
    }
  }

  static void read(Recorder &recorder, shared_ptr<MidiDevice> device)
  {
    while ( true )
    {
      shared_ptr<Event> event { (*device).read() };

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
