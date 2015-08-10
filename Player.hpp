#pragma once
#include <cmath>
#include <thread>
#include <chrono>
#include <list>

#include "Track.hpp"

class Player
{
private:
  double BPM;
  bool playing = false;
  list<reference_wrapper<Track>> tracks;
  list<reference_wrapper<MidiDevice>> clock_devices;
 
public:
  Player(double b) : BPM{b}
  {
  }

  void addClockDevice(MidiDevice &device)
  {
    clock_devices.push_back(ref(device));
  }

  void addTrack(Track &track)
  {
    tracks.push_back(ref(track));
  }

  double getBPM()
  {
    return BPM;
  }  

  void start()
  {
    for ( Track &track : tracks)
      track.start();
    for ( MidiDevice &device : clock_devices )
      device.write( StartEvent{} );

    playing = true;
    thread t(Player::process, ref(*this));
    t.detach();
  }

  void stop()
  {
    playing = false;
    for ( MidiDevice &device : clock_devices )
      device.write( StopEvent{} );
  }

  static void process(Player &player)
  {
    auto tick_time = chrono::microseconds(
                       static_cast<int>(
                          round(60*micro::den / (player.getBPM()*TICKS_PER_BEAT))
                        )
                     );

    while ( player.playing )
    {
      auto start_time = chrono::high_resolution_clock::now();

      for ( Track &track : player.tracks)
        track.tick();
      for ( MidiDevice &device : player.clock_devices )
        device.write( ClockEvent{} );

      this_thread::sleep_for(tick_time + start_time -
                             chrono::high_resolution_clock::now());
    }
  }
};
