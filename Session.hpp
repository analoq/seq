#pragma once
#include "MidiDevice.hpp"
#include "Track.hpp"
#include "Player.hpp"
#include "Recorder.hpp"

#include <vector>

class Session
{
private:
  MidiDevice device{"hw:3,0,0"};
  MidiDevice clock_device{"hw:3,0,8"};

public:
  Track track{device, 0, "1 (Piano)"};
  Player player {105.0};
  Recorder recorder;

  Session()
  {
    track.setPatch(63, 0, 1);

    player.addClockDevice(clock_device);
    player.addTrack(track);

    recorder.addDevice(device);
    recorder.setTrack(track);
  }
};

