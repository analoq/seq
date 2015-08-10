#pragma once
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <memory>
#include <alsa/asoundlib.h>

#include "Event.hpp"

using namespace std;

const int TICKS_PER_BEAT = 24;

class MidiDevice
{
private:
  snd_rawmidi_t *handle_in = 0;
  snd_rawmidi_t *handle_out = 0;

  enum : uint8_t
  {
    MIDI_NOTE_ON = 0x90,
    MIDI_NOTE_OFF = 0x80,
    MIDI_CONTROL = 0xB0,
  };

  void write(const uint8_t channel, const NoteOnEvent &event) const
  {
    uint8_t buffer[]
    {
      static_cast<uint8_t>(MIDI_NOTE_ON | channel),
      event.note,
      event.velocity
    };
    snd_rawmidi_write(handle_out, buffer, 3);
  }

  void write(const uint8_t channel, const NoteOffEvent &event) const
  {
    unsigned char buffer[]
    {
      static_cast<uint8_t>(MIDI_NOTE_OFF | channel),
      event.note,
      0
    };
    snd_rawmidi_write(handle_out, buffer, 3);
  }

  void write(const uint8_t channel, const ControlEvent &event) const
  {
    uint8_t buffer[]
    {
      static_cast<uint8_t>(MIDI_CONTROL | channel),
      event.controller,
      event.value
    };
    snd_rawmidi_write(handle_out, buffer, 3);
  }


public:
  MidiDevice(string device_id)
  {
    int err {snd_rawmidi_open(&handle_in, &handle_out, device_id.c_str(), 0)};
    if ( err )
      throw runtime_error{"snd_rawmidi_open failed"};
  }

  ~MidiDevice()
  {
    snd_rawmidi_drain(handle_in);
    snd_rawmidi_close(handle_in);
    snd_rawmidi_drain(handle_out);
    snd_rawmidi_close(handle_out);
  }

  shared_ptr<Event> read() const
  {
    uint8_t buffer_in[3];
    snd_rawmidi_read(handle_in, &buffer_in, 3);
    switch ( buffer_in[0] & 0xF0 )
    {
      case MIDI_NOTE_ON:
        if ( buffer_in[2] )
          return shared_ptr<Event>(new NoteOnEvent { buffer_in[1], buffer_in[2] });
        else
          return shared_ptr<Event>(new NoteOffEvent { buffer_in[1] });

      case MIDI_NOTE_OFF:
        return shared_ptr<Event>(new NoteOffEvent { buffer_in[1] });

      case MIDI_CONTROL:
        return shared_ptr<Event>(new ControlEvent { buffer_in[1], buffer_in[2] });
      
      default:
        return shared_ptr<Event>(new Event {});
    }
  }

  void write(const StartEvent &event) const
  {
    uint8_t buffer[] {0xFA};
    snd_rawmidi_write(handle_out, buffer, 1);
  }

  void write(const StopEvent &event) const
  {
    uint8_t buffer[] {0xFC};
    snd_rawmidi_write(handle_out, buffer, 1);
  }

  void write(const ClockEvent &event) const
  {
    uint8_t buffer[] {0xF8};
    snd_rawmidi_write(handle_out, buffer, 1);
  }

  void write(const uint8_t channel, const PatchEvent &event) const
  {
    uint8_t buffer[]
    {
      static_cast<uint8_t>(0xB0 | channel), // bank command
      event.msb,
      event.lsb,
      static_cast<uint8_t>(0xC0 | channel), // patch command
      event.patch
    };
    snd_rawmidi_write(handle_out, buffer, 5);
  }

  void write(const uint8_t channel, const Event &event) const
  {
    if (typeid(event) == typeid(NoteOnEvent))
    {
      const NoteOnEvent &note_on = static_cast<const NoteOnEvent &>(event);
      write(0, note_on);
    }
    else if (typeid(event) == typeid(NoteOffEvent))
    {
      const NoteOffEvent &note_off = static_cast<const NoteOffEvent &>(event);
      write(0, note_off);
    }
    else if (typeid(event) == typeid(ControlEvent))
    {
      const ControlEvent &control = static_cast<const ControlEvent &>(event);
      write(0, control);
    }
  }
};

