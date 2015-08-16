#pragma once
#include <stdexcept>
#include <string>
#include <unordered_map>
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

  unordered_map<uint8_t, shared_ptr<NoteOnEvent>> open_notes;

public:
  string device_id;

  MidiDevice(string id) : device_id(id)
  {
    int err {snd_rawmidi_open(&handle_in, &handle_out, device_id.c_str(), 0)};
    if ( err )
      throw runtime_error{"snd_rawmidi_open failed for " + device_id};
  }

  ~MidiDevice()
  {
    snd_rawmidi_drain(handle_in);
    snd_rawmidi_close(handle_in);
    snd_rawmidi_drain(handle_out);
    snd_rawmidi_close(handle_out);
  }

  shared_ptr<Event> read()
  {
    uint8_t buffer_in[3];
    snd_rawmidi_read(handle_in, &buffer_in, 3);

    shared_ptr<NoteOnEvent> note_on, note_off;

    switch ( buffer_in[0] & 0xF0 )
    {
      case MIDI_NOTE_ON:
        if ( buffer_in[2] )
        {
          note_on = shared_ptr<NoteOnEvent>(new NoteOnEvent { buffer_in[1], buffer_in[2] });
          open_notes[buffer_in[1]] = note_on;
          return note_on;
        }
        else
        {
          note_on = open_notes.at(buffer_in[1]);
          open_notes.erase(buffer_in[1]);
          return shared_ptr<Event>(new NoteOffEvent { note_on });
        }

      case MIDI_NOTE_OFF:
        note_on = open_notes.at(buffer_in[1]);
        open_notes.erase(buffer_in[1]);
        return shared_ptr<Event>(new NoteOffEvent { note_on });

      case MIDI_CONTROL:
        return shared_ptr<Event>(new ControlEvent { static_cast<ControlEvent::Controller>(buffer_in[1]), buffer_in[2] });
      
      default:
        return shared_ptr<Event>(new Event {});
    }
  }

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
      (*event.note_on).note,
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
    if ( dynamic_cast<const NoteOnEvent *>(&event) )
    {
      const NoteOnEvent &note_on = static_cast<const NoteOnEvent &>(event);
      write(channel, note_on);
    }
    else if (dynamic_cast<const NoteOffEvent *>(&event) )
    {
      const NoteOffEvent &note_off = static_cast<const NoteOffEvent &>(event);
      write(channel, note_off);
    }
    else if (dynamic_cast<const ControlEvent *>(&event) )
    {
      const ControlEvent &control = static_cast<const ControlEvent &>(event);
      write(channel, control);
    }
  }
};

