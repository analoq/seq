#pragma once
#include "Event.hpp"

class Plugin
{
public:
  virtual void process(NoteOnEvent &note_on) {};
};

class TransposePlugin : public Plugin
{
  int transpose = 0;

  void process(NoteOnEvent &note_on)
  {
    note_on.transpose = transpose;
  }
};
