run: seq
	./seq

seq: main.cpp MidiDevice.hpp Event.hpp Clip.hpp Track.hpp Player.hpp Recorder.hpp Session.hpp Gui.hpp
	g++ -std=c++11 -g main.cpp -lasound -lncurses -ljsoncpp -o seq
