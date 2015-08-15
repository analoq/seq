#pragma once
#include <iostream>
using namespace std;

struct Event
{
  virtual ~Event()
  {
  }
};

struct TimedEvent
{
  int time;

  TimedEvent(int t) : time{t}
  {
  }

  virtual ~TimedEvent()
  {
  }
};

struct NoteOnEvent : Event
{
  uint8_t note;
  uint8_t velocity;

  NoteOnEvent(uint8_t n, uint8_t v)
    : note{n}, velocity{v}
  {
  }

  NoteOnEvent(const NoteOnEvent &note_on)
    : note{note_on.note}, velocity{note_on.velocity}
  {
  }
};

struct TimedNoteOnEvent : NoteOnEvent, TimedEvent
{
  int length;

  TimedNoteOnEvent(int t, NoteOnEvent &note_on)
    : NoteOnEvent{note_on}, TimedEvent{t}, length{0}
  {
  }
};

struct NoteOffEvent : Event
{
  uint8_t note;

  NoteOffEvent(uint8_t n) : note{n}
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
