#pragma once
#include "Event.hpp"

class Plugin
{
public:
  virtual void process(NoteOnEvent &note_on) {};
  virtual void tick(function<void(Event &)> callback) {};
};

class TransposePlugin : public Plugin
{
  int transpose = 0;

  void process(NoteOnEvent &note_on)
  {
    note_on.note += transpose;
  }

  void tick(function<void(const Event &)> callback)
  {
  }
};
