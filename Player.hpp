#pragma once
#include <cmath>
#include <thread>
#include <chrono>
#include <vector>
#include <list>

#include "Track.hpp"

class Player
{
private:
  double BPM = 120.0;
  bool playing = false;
  vector<Track> tracks;
  list<shared_ptr<MidiDevice>> clock_devices;
 
public:
  void addClockDevice(shared_ptr<MidiDevice> device)
  {
    clock_devices.push_back(device);
  }

  void addTrack(Track track)
  {
    tracks.push_back(track);
  }

  Track &getTrack(int index)
  {
    return tracks[index];
  }

  int getTrackCount()
  {
    return tracks.size();
  }

  double getBPM()
  {
    return BPM;
  }

  void setBPM(double bpm)
  {
    BPM = bpm;
  }

  void start()
  {
    for ( Track &track : tracks)
      track.start();
    for ( shared_ptr<MidiDevice> device : clock_devices )
      (*device).write( StartEvent{} );

    playing = true;
    thread t(Player::process, ref(*this));
    t.detach();
  }

  void stop()
  {
    playing = false;
    for ( shared_ptr<MidiDevice> device : clock_devices )
      (*device).write( StopEvent{} );
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
      for ( shared_ptr<MidiDevice> device : player.clock_devices )
        (*device).write( ClockEvent{} );

      this_thread::sleep_for(tick_time + start_time -
                             chrono::high_resolution_clock::now());
    }
  }
};
