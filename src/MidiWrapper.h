/**********************************************************************/
/*! \class MidiWrapper
    \brief An abstract base class for realtime MIDI input/output.

    This class provides a layer over RtMidi to adapt handling for
	compiling a plugin for Unity3d

    RtMidi WWW site: http://music.mcgill.ca/~gary/rtmidi/

    RtMidi: realtime MIDI i/o C++ classes
    Copyright (c) 2003-2014 Gary P. Scavone

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    Any person wishing to distribute modifications to the Software is
    asked to send the modifications to the original developer so that
    they can be incorporated into the canonical version.  This is,
    however, not a binding provision of this license.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/**********************************************************************/

#ifndef MIDIWRAPPER_H
#define MIDIWRAPPER_H

#ifndef EXPORT_DLL
	#define EXPORT_DLL __declspec(dllexport) 
#endif


#include "RtMidi.h"
//#include <sstream>
//#include <iostream>
//#include <cstring>
//#include <string>
#include <vector>

//define EXPORT_API __declspec(dllexport)

extern "C" {
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Messages structures

	//Midi notes messages (note on and note off)
	typedef struct MidiNoteMessage{
		int code = 0;
		int id = 0;
		int velocity = 0;
		//unsigned char code;
		//unsigned char id;
		//unsigned char velocity;
		double timestamp = 0.0;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MIDI Initialization & status
	/**
	 * sets up the midi connection.
	 * by default will connect to the first available port (and try to be non virtual)
	 * if port is a negative value (or if there are no ports available) will try to connect to a virtual device
	 * if an id is given will try to connect to the given port number if fails connects to the first available port
	 *
	 * returns true if manages to correclty initialize input AND output, false otherwise
	 * as bool is not blittable by .Net interface, then 0 == false, true otherwise
	 **/
	EXPORT_DLL int startMidi(int port=-1);
	/**
	 * sets up the midi connection to the given deviceName if exists.
	 *
	 * returns true if manages to correclty initialize input AND output, false otherwise
	 * will connect to the especified port or will return false
	 **/
	//int startMidiByName(std::string deviceName);
	
	/**
	 * closes and cleans up midi connection.
	 *
	 * returns true if all OK, false false otherwise
	 **/
	EXPORT_DLL int closeMidi();
	/**
	 * returns true if the midi connections (I/O) are open. false false otherwise (error, disconnect or something else)
	 **/
	EXPORT_DLL int isConnected();

	/**
	* returns true if the midi connections (I/O) are open. false false otherwise (error, disconnect or something else)
	**/
	EXPORT_DLL int ìsOutputOpen();
	EXPORT_DLL int ìsInputOpen();


	/**
	 * Callback that will get called when a message arrives
	 * this is best sent immediately after initialization in order to avoid 
	 **/
	//int setCallback(FOO* callback);
	
	/**
	 * Returns the name of the device that is connected to
	 * default index is -1 and this means that it will look for the currently opened port
	 **/
	//c string type (is a null terminated char * for compatibility)
	EXPORT_DLL const char* __cdecl getDeviceName(int portIndex=-1);

	EXPORT_DLL unsigned int getInPortCount();
	EXPORT_DLL const char* __cdecl getInPortName(unsigned int port);

	EXPORT_DLL unsigned int getOutPortCount();
	EXPORT_DLL const char* __cdecl getOutPortName(unsigned int port);


	/**
	 * volume: value [0-127]
	 * sets the active device volume 
	 **/
	//TODO
	//void setDeviceVolume(int volume);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MIDI Intput

	/**
	 * returns the current velocity of the given note
	 **/
	EXPORT_DLL int noteStatus(int noteId);
	/**
	 * must receive an int vector of 128 elements, each bucket correspnds to the note id
	 * each one contains the velocity and timestamp of the last event on that note
	 **/
	//TODO adapt to C from vector to array... ??
	EXPORT_DLL void notesStatus(std::vector<int> *notes);
	/**
	 * must receive an int vector of 128 elements, each bucket correspnds to the note id
	 * each bucket will be filled with the last timestamp value of an event, -1 if none yet
	 **/
	//TODO adapt to C from vector to array... ??
	EXPORT_DLL void notesStatusTimestamps(std::vector<double> *notes);

	/**
	 * fill the input an bool vector of 3 elements (MUST BE), each bucket correspnds to the pedal id
	 * each contains a boolean field with true if active, and false if inactive
	 **/
	//TODO
	//void pedalsStatus(std::vector<bool> *pedals);
	/**
	 * must receive an int vector of 128 elements, each bucket correspnds to the note id
	 * each bucket will be filled with the last timestamp value of an event, -1 if none yet
	 **/
	//TODO
	//void pedalsStatusTimestamps(std::vector<double> *pedals);
	
	//! Fill the user-provided vector with the data bytes for the next available MIDI message in the input queue and return the event delta-time in seconds.
    /*!
    This function returns immediately whether a new message is
    available or not.  A valid message is indicated by a non-zero
    vector size.  An exception is thrown if an error occurs during
    message retrieval or an input connection was not previously
    established.
    */
	//double getMessage( std::vector<unsigned char> *message );
	
	/**
	 * returns the next message in the queue that affects a note
	 * this is a destructive read (the message read will be popped from the queue)
	 **/
	EXPORT_DLL MidiNoteMessage getNextNoteMessage(void);

	/**
	* fills the given message with the next message in the queue.
	* the next message in the queue that affects a note
	* this is a destructive read (the message read will be popped from the queue)
	**/
	EXPORT_DLL void __cdecl fillWithNextNoteMessage(MidiNoteMessage &message);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MIDI Output
	
	/**
	 * id: midi id of the note to turn on [0-127]
	 * velocity: [0-127]
	 * channel default 0
	 **/
	EXPORT_DLL void noteOn(unsigned char id, unsigned char velocity, int channel=0);
	/**
	 * id: midi id of the note to turn off
	 * velocity: [0-127]
	 * channel default 0
	 **/
	EXPORT_DLL void noteOff(unsigned char  id, int channel=0);
	/**
	 * pedalNumber: the number of the pedal (piano), there are 3 pedals counted from left to right 1,2,3
	 * channel default 0
	 **/
	//TODO later 
	//void pedalOn(int pedalNumber, int channel=0);
	/**
	 * pedalOff: the number of the pedal (piano), there are 3 pedals counted from left to right 1,2,3
	 * channel default 0
	 **/
	//TODO later 
	//void pedalOff(int pedalNumber, int channel=0);
	/**
	 * code: midi message code
	 * data[]: array of bytes with the data
	 * channel default 0
	 **/
	//void sendMessage(int code, int id, std::vector<unsigned char> *data, int channel=0);

	//! Immediately send a single message out an open MIDI output port.
	/*!
	  An exception is thrown if an error occurs during output or an
	  output connection was not previously established.
	*/
	//void sendMessage( std::vector<unsigned char> *message );
	///////////////////////////////////////////////////////////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Non extern functions, that later will need to either be adapted for extern c or discarded

//unsigned int getInPortCount();
//std::string getInPortName(int port);

//unsigned int getOutPortCount();
//std::string getOutPortName(int port);

///////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //MIDIWRAPPER