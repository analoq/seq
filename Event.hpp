#pragma once
#include <memory>

using namespace std;

struct Event
{
  int time;

  Event() : time{0}
  {
  }

  virtual ~Event()
  {
  }
};

struct NoteOnEvent : Event
{
  uint8_t note;
  uint8_t velocity;
  int length;

  NoteOnEvent(uint8_t n, uint8_t v)
    : note{n}, velocity{v}, length{0}
  {
  }
};

struct NoteOffEvent : Event
{
  shared_ptr<NoteOnEvent> note_on;

  NoteOffEvent(shared_ptr<NoteOnEvent> n) : note_on{n}
  {
  }
};

struct ControlEvent : Event
{
  enum Controller : uint8_t
  {
    MODULATION = 1,
    VOLUME = 7
  } controller;
  uint8_t value;


  ControlEvent(Controller c, uint8_t v) : controller{c}, value{v}
  {
  }
};

struct PatchEvent : Event
{
  uint8_t msb;
  uint8_t lsb;
  uint8_t patch;

  PatchEvent(uint8_t m, uint8_t l, uint8_t c) : msb{m}, lsb{l}, patch{c}
  {
  }
};

struct StartEvent : Event
{
};

struct StopEvent : Event
{
};

struct ClockEvent : Event
{
};
