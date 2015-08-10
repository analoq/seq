run: seq
	./seq

seq: main.cpp MidiDevice.hpp Event.hpp Clip.hpp Track.hpp Player.hpp Recorder.hpp Session.hpp
	g++ -std=c++11 main.cpp -lasound -lncurses -o seq
