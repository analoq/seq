#pragma once
#include "MidiDevice.hpp"
#include "Track.hpp"
#include "Player.hpp"
#include "Recorder.hpp"

#include <fstream>
#include <vector>
#include <map>
#include <jsoncpp/json/reader.h>

using namespace std;

class Session
{
public:
  Player player;
  Recorder recorder;
  vector<shared_ptr<Track>> tracks;

  Track &getTrack(int index)
  {
    return *tracks.at(index);
  }

  int getTrackCount() const
  {
    return tracks.size();
  }

  void loadJSON(string path)
  {
    // parse json
    Json::Value root;
    ifstream doc {path, ifstream::binary };
    doc >> root;

    // get devices
    map<string, shared_ptr<MidiInputDevice>> input_devices;
    for ( Json::ValueIterator it = root["input_devices"].begin();
          it != root["input_devices"].end();
          ++ it )
    {
      string device_name { it.key().asString() };
      string device_id { (*it).asString() };
      input_devices[device_name] = shared_ptr<MidiInputDevice>(new MidiInputDevice {device_id});

      recorder.addDevice( input_devices[device_name] );
      player.addClockReceiver( input_devices[device_name] );
    }

    map<string, shared_ptr<MidiOutputDevice>> output_devices;
    for ( Json::ValueIterator it = root["output_devices"].begin();
          it != root["output_devices"].end();
          ++ it )
    {
      string device_name { it.key().asString() };
      string device_id { (*it).asString() };
      output_devices[device_name] = shared_ptr<MidiOutputDevice>(new MidiOutputDevice {device_id});
    }

    // get clock devices
    for ( Json::Value node : root["clock_devices"] )
    {
      string device_name { node.asString() };
      player.addClockReceiver( output_devices[device_name] );
    }

    // get tempo
    double tempo = root["tempo"].asDouble();
    player.setBPM(tempo);

    // get tracks
    for ( Json::Value node : root["tracks"] )
    {
      string name { node["name"].asString() };
      string device_name { node["device"].asString() };
      uint8_t channel { static_cast<uint8_t>(node["channel"].asInt()) };
      uint8_t msb { static_cast<uint8_t>(node["msb"].asInt()) };
      uint8_t lsb { static_cast<uint8_t>(node["lsb"].asInt()) };
      uint8_t program { static_cast<uint8_t>(node["program"].asInt()) };
      uint8_t volume { static_cast<uint8_t>(node["volume"].asInt()) };

      auto track = shared_ptr<Track>(new Track {output_devices[device_name], channel, name});
      track->setPatch(msb, lsb, program);
      track->setVolume(volume);
      tracks.push_back(track);

      player.addClockReceiver( track );
    }

    recorder.setTrack(&getTrack(0));
  }
};

