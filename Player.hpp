#pragma once
#include <cmath>
#include <thread>
#include <chrono>
#include <unordered_set>

#include "Clocked.hpp"

class Player
{
private:
  double BPM = 120.0;
  bool playing = false;
  unordered_set<shared_ptr<Clocked>> clock_receivers;
 
public:
  void addClockReceiver(shared_ptr<Clocked> receiver)
  {
    clock_receivers.emplace(receiver);
  }

  double getBPM() const
  {
    return BPM;
  }

  void setBPM(double bpm)
  {
    BPM = bpm;
  }

  void start()
  {
    for ( shared_ptr<Clocked> receiver : clock_receivers )
      receiver->start();

    playing = true;
    thread t(Player::process, ref(*this));
    t.detach();
  }

  void stop()
  {
    playing = false;
    for ( shared_ptr<Clocked> receiver : clock_receivers )
      receiver->stop();
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

      for ( shared_ptr<Clocked> receiver : player.clock_receivers )
        receiver->tick();

      this_thread::sleep_for(tick_time + start_time -
                             chrono::high_resolution_clock::now());
    }
  }
};
