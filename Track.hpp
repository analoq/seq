#pragma once
#include <vector>
#include "MidiDevice.hpp"
#include "Clip.hpp"
#include "Event.hpp"

class Track
{
private:
  MidiDevice &device;
  uint8_t channel;
  string name;

  vector<Clip> clips { Clip{8}, Clip{8}, Clip{8} };

public:
  Track(MidiDevice &d, uint8_t c, string n)
    : device(d), channel(c), name(n)
  {
  }

  string getName()
  {
    return name;
  }

  Clip &getClip(const int index)
  {
    return clips.at(index);
  }

  const int getClipCount()
  {
    return clips.size();
  }

  void setPatch(const uint8_t msb, const uint8_t lsb, const uint8_t program)
  {
    device.write(channel, PatchEvent {msb, lsb, program});
  }

  void send(const Event &event) const
  {
    device.write(channel, event);
  }

  void start()
  {
    for ( Clip &clip : clips )
      clip.start();
  }

  void tick()
  {
    for ( Clip &clip : clips )
      clip.tick(device, channel);
  }
};

