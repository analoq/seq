#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <list>
#include "MidiDevice.hpp"
#include "Clip.hpp"
#include "Event.hpp"

class Track
{
private:
  uint8_t channel;
  string name;
  shared_ptr<MidiDevice> device;

  vector<Clip> clips { Clip{8}, Clip{8}, Clip{8} };
  list<TimedNoteOnEvent> playing_notes;

public:
  Track(shared_ptr<MidiDevice> d, uint8_t c, string n)
    : device(d), channel(c), name(n)
  {
  }

  string getName() const
  {
    return name;
  }

  uint8_t getChannel() const
  {
    return channel;
  }

  string getDeviceName() const
  {
    return (*device).device_id;
  }

  Clip &getClip(const int index)
  {
    return clips.at(index);
  }

  const int getClipCount() const
  {
    return clips.size();
  }

  void setPatch(const uint8_t msb, const uint8_t lsb, const uint8_t program)
  {
    (*device).write(channel, PatchEvent {msb, lsb, program});
  }

  void setVolume(const uint8_t volume)
  {
    (*device).write(channel,
                    ControlEvent { ControlEvent::Controller::VOLUME,
                                   volume });
  }

  void send(const Event &event)
  {
    (*device).write(channel, event);
  }

  void sendNote(const TimedNoteOnEvent &event)
  {
    send(event);
    playing_notes.push_back(event);
  }

  void start()
  {
    for ( Clip &clip : clips )
      clip.start();
  }

  void tick()
  {
    // release playing notes
    for ( auto it = playing_notes.begin();
          it != playing_notes.end(); )
    {
      TimedNoteOnEvent &playing_note = *it;
      playing_note.length -- ;

      if ( !playing_note.length )
      {
        (*device).write( channel, NoteOffEvent { playing_note.note } );
        it = playing_notes.erase(it);
      }
      else
        ++it;
    }

    function<void(const TimedNoteOnEvent &)> f = bind(&Track::sendNote,
                                                      this,
                                                      placeholders::_1);
    for ( Clip &clip : clips )
      clip.tick( f );
  }
};

