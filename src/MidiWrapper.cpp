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


#include "MidiWrapper.h"
#include <queue>
#include <string>

//define EXPORT_API __declspec(dllexport)

//holders
RtMidiIn  *midiin = NULL;
RtMidiOut *midiout = NULL;
//name holders for temporary passing (buffers ...)
char in_name[1024];
char out_name[1024];
//device status
//std::string deviceName = "NONAME";
//unsigned int portUsed = -1;

//midi notes status (velocity)
std::vector<int> notesStatusVector;

//midi notes last event received (timestamp in unix time)
std::vector<double> notesStatusTimestampsVector;
/*
//for pedal status .... TODO later
std::vector<bool> pedalsStatus;
for(int i=0;i<3;i++){
	pedalsStatus.push_back(false);
}
*/
//midi notes last event received (timestamp in unix time)
std::queue<MidiNoteMessage> notesMessagesQueue;
//buffer for all midi messages that arrive
std::queue < std::vector< unsigned char > > messagesQueue;

unsigned int MAX_QUEUED_MESSAGES = 1000;

//midi messages status for note on and note off
int NOTE_ON_MESSAGE = 144;
int NOTE_OFF_MESSAGE = 128;


int MIDI_STATUS_VECTORS_SIZE = 130; //max is 128 but this way I keep the real midi number for the indices and also an extra value just in case

extern "C" {

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Helper functions
	//callback that processes the input messages keeping track of the status of the notes and the input queue
	void incallback( double deltatime, std::vector< unsigned char > *message, void * /*userData*/){
	  
		if(messagesQueue.size() > MAX_QUEUED_MESSAGES){
			messagesQueue.pop();
		}
		//new message for keeping record
		std::vector< unsigned char > nMessage;

		//TODO keep track of the notes status
		//unsigned int nBytes = message->size();
		size_t nBytes = message->size();
		for ( unsigned int i=0; i<nBytes; i++ ){
			unsigned char bi = message->at(i);
			nMessage.push_back(bi);
			//std::cout << "Byte " << i << " = " << (int)bi << ", ";

		}
		////////////////////////////////////
		//deal with noteOn and noteOff messages and push them in the
		//if there is a message and is actually a noteON or noteOFF
		//todo add other things as the pedals
		if ( nBytes > 0 && (message->at(0) == 144 || message->at(0) == 128)){
			try{

				//queueing the message
				MidiNoteMessage msg;
				msg.code = message->at(0);
				msg.id = message->at(1);
				msg.velocity = message->at(2);
				msg.timestamp = deltatime;
				notesMessagesQueue.push(msg);
				//changing the status of the notes vector
				notesStatusVector[msg.id] = msg.velocity ;
				notesStatusTimestampsVector[msg.id] = deltatime; //change it for absolute timestamp ... TODO

				//std::cout << "stamp = " << deltatime << std::endl;
			}catch(std::exception e){
				//TODO do something here
			}

		}
		////////////////////////////////////
		//all the messages types are stored as they came in here
		messagesQueue.push(nMessage);

	}

	//midi port opening and selection (from the ones available)
	bool chooseMidiPort( RtMidi *rtmidi, int port)
	{
		//
		if (rtmidi == NULL) {
			return false;
		}
		//port <0 signifies open the virtual port as none is available
		//port == 0 signifies open the first port available
		//let's say by default there are no ports available so will open virtual port
		
		bool portsAvailable = false;
		try {
			int nPorts = rtmidi->getPortCount();
			//no available ports
			//if ( nPorts == 0){
			//	portsAvailable = false;
			//}
			if (nPorts > 0) {
				portsAvailable = true;
				if (port >= nPorts) {
					//port = nPorts-1;
					port = 0;
				}
				//actually open the port
				rtmidi->openPort(port);
				//portUsed = port;
			}
			if (port < 0 || !portsAvailable) {
				rtmidi->openVirtualPort();
			}
		}
		catch (...) {
			portsAvailable = false;
		}
	  //TODO make something to deal with errors here
	  return portsAvailable;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MIDI Initialization, Finalization & Status

	EXPORT_DLL void setupEnv() {
		//setup variables, mandatory before anything or the libray will give runtime exceptions
		for (int i = 0; i<MIDI_STATUS_VECTORS_SIZE; i++) {
			notesStatusVector.push_back(0);
		}
		for (int i = 0; i<MIDI_STATUS_VECTORS_SIZE; i++) {
			notesStatusTimestampsVector.push_back(0.0);
		}
	}

	EXPORT_DLL int createInput() {
		int ret = 1;
		try { 
			if (midiin != NULL) { ret = destroyInput(); }
			midiin = new RtMidiIn();
		} catch(...){ ret = 0; }
		return ret;
	}

	EXPORT_DLL void cleanupInputEnv() {
		//there is this swap idiom but not clear() function
		notesMessagesQueue.swap(std::queue<MidiNoteMessage>());
		messagesQueue.swap(std::queue < std::vector< unsigned char > >());
		//then the new queue gets out of scope and "gc" takes its place
		notesStatusVector.clear();
		notesStatusTimestampsVector.clear();
	}

	EXPORT_DLL int destroyInput() {
		int ret = 1;
		try {
			if (midiin != NULL) {
				if (midiin->isPortOpen()) { midiin->closePort(); }
				delete midiin;
				midiin = NULL;
			}
		} catch (...) { ret = 0; }
		return ret;
	}
	
	EXPORT_DLL int openInputPort(int port) {
		int ret = 0;
		if(chooseMidiPort(midiin, port) == true) {
			midiin->setCallback(&incallback);
			ret = 1;
		}
		return ret;
	}

	EXPORT_DLL void closeInputPort() {
		if (midiin != NULL && midiin->isPortOpen()) {
			midiin->closePort();
		}
	}

	EXPORT_DLL int isInputPortOpen() {
		int ret = 0;
		if (midiin != NULL) { ret = (midiin->isPortOpen() ? 1 : 0); }
		return ret;
	}

	EXPORT_DLL unsigned int getInPortCount() {
		unsigned int ret = 0;
		if (midiin != NULL) { ret = midiin->getPortCount(); }
		return ret;
	}

	EXPORT_DLL void getInputPortName(char* name, unsigned int port) {
		if (midiin != NULL && port>=0 && midiin->getPortCount() > port) {
			std::string pname = midiin->getPortName(port);
			const char* cname = pname.c_str();
			strcpy_s(name, pname.size() + 1, cname);
		}
		//nothing
	}

	EXPORT_DLL char * getInputPortNamePtr(unsigned int port) {
		if (midiin != NULL && port >= 0 && midiin->getPortCount() > port) {
			std::string pname = midiin->getPortName(port);
			const char* cname = pname.c_str();
			strcpy_s(in_name, pname.size() + 1, cname);
			
		}
		else {
			strcpy_s(in_name, 6 + 1, "NONAME");
		}
		return in_name;
	}

	EXPORT_DLL int createOutput() {
		int ret = 1;
		try {
			if (midiout != NULL) { ret = destroyOutput(); }
			midiout = new RtMidiOut();
		}
		catch (...) { ret = 0; }
		return ret;
	}

	EXPORT_DLL int destroyOutput() {
		int ret = 1;
		try {
			if (midiout != NULL) {
				if (midiout->isPortOpen()) { midiout->closePort(); }
				delete midiout;
				midiout = NULL;
			}
		}
		catch (...) { ret = 0; }
		return ret;
	}

	EXPORT_DLL int isOutputPortOpen() {
		int ret = 0;
		if (midiout != NULL) { ret = (midiout->isPortOpen() ? 1 : 0); }
		return ret;
	}

	EXPORT_DLL unsigned int getOutPortCount() {
		unsigned int ret = 0;
		if (midiout != NULL) { ret = midiout->getPortCount(); }
		return ret;
	}

	EXPORT_DLL int openOutputPort(int port) {
		int ret = 0;
		(chooseMidiPort(midiout, port) == true) ? ret = 1 : ret = 0;
		return ret;
	}

	EXPORT_DLL void closeOutputPort() {
		if (midiout != NULL && midiout->isPortOpen()) {
			midiout->closePort();
		}
	}

	EXPORT_DLL void getOutputPortName(char* name, unsigned int port) {
		if (midiout != NULL && port >= 0 && midiout->getPortCount() > port) {
			std::string pname = midiout->getPortName(port);
			const char* cname = pname.c_str();
			strcpy_s(name, pname.size() + 1, cname);
		}
		//nothing
	}

	EXPORT_DLL char * getOutputPortNamePtr(unsigned int port) {
		if (midiin != NULL && port >= 0 && midiin->getPortCount() > port) {
			std::string pname = midiout->getPortName(port);
			const char* cname = pname.c_str();
			strcpy_s(out_name, pname.size() + 1, cname);

		}
		else {
			strcpy_s(out_name, 6 + 1, "NONAME");
		}
		return out_name;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// MIDI Input

	EXPORT_DLL MidiNoteMessage getNextMessageStruct() {
		if (notesMessagesQueue.size() > 0) {
			MidiNoteMessage nm = notesMessagesQueue.front();
			//erase it
			notesMessagesQueue.pop();
			return nm;
		}
		MidiNoteMessage em;
		return em;
	}

	//get next noteOn or noteOff message
	void fillWithNextNoteMessage(MidiNoteMessage &message) {
		//get reference
		if (notesMessagesQueue.size() > 0) {
			MidiNoteMessage nm = notesMessagesQueue.front();
			//erase it
			notesMessagesQueue.pop();

			message.code = nm.code;
			message.id = nm.id;
			message.velocity = nm.velocity;
			message.timestamp = nm.timestamp;
		}
	}

	EXPORT_DLL long getNextMessageAsLong() {
		long ret = 0x00000000;
		if (notesMessagesQueue.size() > 0) {
			/*
			//directly from bytes
			std::vector< unsigned char > bm = messagesQueue.front();
			messagesQueue.pop();
			if (bm.size() <= 8) {
				for (int i = 7, j = 0; i >= 0, j < bm.size(); i--, j++) {
					ret = ret | bm[j] << 8 * i;
				}
			}*/
			//from MidiNoteMessage
			MidiNoteMessage nm = notesMessagesQueue.front();
			//erase it
			notesMessagesQueue.pop();

			ret = nm.code << 8*7;
			ret = ret | nm.id << 8*6;
			ret = ret | nm.velocity << 8*5;
			//nm.timestamp; // ignore timestamp for the moment
		}
		return ret;
	}

	EXPORT_DLL unsigned int getNextMessageAsUInt() {
		long ret = 0x00000000;
		if (notesMessagesQueue.size() > 0) {
			//from MidiNoteMessage
			MidiNoteMessage nm = notesMessagesQueue.front();
			//erase it
			notesMessagesQueue.pop();
			//Warning with shift might fail with other compilers ... ?? need to previously cast the original before shifting
			ret = nm.code << 8 *3;
			ret = ret | nm.id << 8 * 2;
			ret = ret | nm.velocity << 8 * 1;
			//nm.timestamp; // ignore timestamp for the moment
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// MIDI Output

	//WARNING, might need to use: uint64_t
	//EXPORT_DLL void sendLimitedMessage(unsigned long data, int nBytes, int channel) {
	EXPORT_DLL void sendLimitedMessage(unsigned int data, int nBytes, int channel) {
		//refuse to work with invalid data
		//if (!data || nBytes < 3 || nBytes > 8) {
		if (!data || nBytes < 3 || nBytes > 4) {
			return;
		}
		//all the other things, is assummed that the developer knows midi protocol and nothing will be broken...
		std::vector<unsigned char> message;
		for (int i = 0; i < nBytes; i++) {
			//get byte
			unsigned char b = data & 0xFFFF; 
			//add to message
			message.push_back(b);
			data >> 8; //drop the byte we just read
		}
		//TODO verify
		
		midiout->sendMessage(&message);
	}

	EXPORT_DLL void noteOn(unsigned char id, unsigned char velocity, int channel) {
		//channel not used yet
		std::vector<unsigned char> message;
		// Note On: 144, note id, velocity
		message.push_back(NOTE_ON_MESSAGE);
		message.push_back(id);
		message.push_back(velocity);
		midiout->sendMessage(&message);
	}

	EXPORT_DLL void noteOff(unsigned char id, int channel) {
		std::vector<unsigned char> message;
		// Note Off: 128, note id, velocity
		message.push_back(NOTE_OFF_MESSAGE);
		message.push_back(id);
		message.push_back(0);
		midiout->sendMessage(&message);
	}
}


RtMidi* getMidiIn() {
	return midiin;
}
RtMidi* getMidiOut() {
	return midiout;
}