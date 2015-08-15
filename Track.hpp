#pragma once
#include <vector>
#include <memory>
#include <functional>
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

  void send(const Event &event) const
  {
    (*device).write(channel, event);
  }

  void start()
  {
    for ( Clip &clip : clips )
      clip.start();
  }

  void tick()
  {
    function<void(const Event &)> f = bind(&Track::send,
                                           *this,
                                           placeholders::_1);
    for ( Clip &clip : clips )
      clip.tick( f );
  }
};

