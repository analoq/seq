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

  Session(Player &p, Recorder &r) : player{p}, recorder{r}
  {
    recorder.setTrack(&player.getTrack(0));
  }

  static Session jsonFactory(string path)
  {
    // parse json
    Json::Value root;
    ifstream doc {path, ifstream::binary };
    doc >> root;

    // get devices
    map<string, shared_ptr<MidiDevice>> devices;
    for ( Json::Value node : root["devices"] )
    {
      string device_name { node.asString() };
      devices[device_name] = shared_ptr<MidiDevice>(new MidiDevice {device_name});
    }

    // get input devices
    Recorder recorder;
    for ( Json::Value node : root["input_devices"] )
    {
      string device_name { node.asString() };
      recorder.addDevice( devices[device_name] );
    }

    // get clock devices
    Player player;
    for ( Json::Value node : root["clock_devices"] )
    {
      string device_name { node.asString() };
      player.addClockDevice( devices[device_name] );
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

      Track track { devices[device_name], channel, name };
      track.setPatch(msb, lsb, program);
      track.setVolume(volume);
      player.addTrack( track );
    }

    return Session { player, recorder };
  }
};

