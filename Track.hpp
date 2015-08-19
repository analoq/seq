#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <list>
#include "MidiDevice.hpp"
#include "Clip.hpp"
#include "Event.hpp"
#include "Clocked.hpp"
#include "Plugin.hpp"

using namespace std;

class Track : public Clocked
{
private:
  uint8_t channel;
  string name;
  shared_ptr<MidiOutputDevice> device;

  list<NoteOnEvent> playing_notes;
  list<shared_ptr<Plugin>> plugins;
  vector<Clip> clips { Clip{8}, Clip{8}, Clip{8} };
public:
  Track(shared_ptr<MidiOutputDevice> d, uint8_t c, string n)
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
    return (*device).device_name;
  }

  Clip &getClip(const int index)
  {
    return clips.at(index);
  }

  const int getClipCount() const
  {
    return clips.size();
  }

  void addPlugin(shared_ptr<Plugin> plugin)
  {
    plugins.push_back(plugin);
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

  void send(Event &event)
  {
    if ( typeid(event) == typeid(NoteOnEvent) )
    {
      NoteOnEvent &note_on = dynamic_cast<NoteOnEvent &>(event);
      for ( shared_ptr<Plugin> plugin : plugins )
        plugin->process(note_on);
      if ( note_on.length )
        playing_notes.push_back(note_on);
    }
    (*device).write(channel, event);
  }

  void start()
  {
    for ( Clip &clip : clips )
      clip.start();
  }

  void stop()
  {
  }

  void tick()
  {
    // release playing notes
    for ( auto pnit = playing_notes.begin();
          pnit != playing_notes.end(); )
    {
      NoteOnEvent &playing_note = *pnit;
      playing_note.length -- ;

      if ( !playing_note.length )
      {
        NoteOffEvent note_off { shared_ptr<NoteOnEvent>(new NoteOnEvent {playing_note}) };
        send(note_off);
        pnit = playing_notes.erase(pnit);
      }
      else
        ++pnit;
    }

    function<void(Event &)> f = bind(&Track::send,
                                     this,
                                     placeholders::_1);
    for ( Clip &clip : clips )
      clip.tick( f );
  }
};

