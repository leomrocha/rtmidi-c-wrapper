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

//define EXPORT_API __declspec(dllexport)

//holders
RtMidiIn  *midiin = 0;
RtMidiOut *midiout = 0;

//device status
//std::string deviceName = "NONAME";
unsigned int portUsed = -1;

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
		unsigned int nBytes = message->size();
		for ( unsigned int i=0; i<nBytes; i++ ){
			unsigned char bi = message->at(i);
			nMessage.push_back(bi);
			std::cout << "Byte " << i << " = " << (int)bi << ", ";

		}
		////////////////////////////////////
		//deal with noteOn and noteOff messages and push them in the
		//if there is a message and is actually a noteON or noteOFF
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
		messagesQueue.push(nMessage);

	}

	//midi port opening and selection (from the ones available)
	bool chooseMidiPort( RtMidi *rtmidi, int port)
	{
		//port <0 signifies open the virtual port as none is available
		//port == 0 signifies open the first port available
		//let's say by default there are no ports available so will open virtual port
		bool portsAvailable = false;
		int nPorts = rtmidi->getPortCount();
		//no available ports
		//if ( nPorts == 0){
		//	portsAvailable = false;
		//}
		if ( nPorts > 0 ) {
			portsAvailable = true;
			if(port>=nPorts){
				port = nPorts-1;
			}
			//actually open the port
			rtmidi->openPort( port );
			portUsed = port;
		}
		if ( port < 0  || !portsAvailable) {
			rtmidi->openVirtualPort();
		}
	  //TODO make something to deal with errors here
	  return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MIDI Initialization & status
	int startMidi(int port){
		int success = 1; //true;
		try {
			//setup variables, mandatory before anything or the libray will give runtime exceptions
			for(int i=0; i<MIDI_STATUS_VECTORS_SIZE; i++){
				notesStatusVector.push_back(0);
			}
			for(int i=0; i<MIDI_STATUS_VECTORS_SIZE; i++){
				notesStatusTimestampsVector.push_back(0.0);
			}

			// RtMidiIn constructor ... exception possible
			midiin = new RtMidiIn();
			if ( chooseMidiPort( midiin, port ) == false ){ success = 0; } //failure == success = false

			// Set our callback function.  This should be done immediately after
			// opening the port to avoid having incoming messages written to the
			// queue instead of sent to the callback function.
			midiin->setCallback( &incallback );

			// Don't ignore sysex, timing, or active sensing messages.
			//midiin->ignoreTypes( false, false, false );

			// RtMidiOut constructor ... exception possible
			midiout = new RtMidiOut();
			if ( chooseMidiPort( midiout, port ) == false ){ success = 0; }

			if (!success){ closeMidi(); }

		} catch ( RtMidiError &error ) {
			error.printMessage();
		}
		//catch ( ... ) {
			//some bigger error here
		//}
		return success;
	}

	int closeMidi(){
		if (midiin != NULL){
			delete midiin;
		}
		if (midiout != NULL){
			delete midiout;
		}
		return 1; //true;
	}

	int isConnected(){
		int status = 0; //false;
		if(midiin != NULL && midiout != NULL){
			status = midiin->isPortOpen() && midiout->isPortOpen();
		}
		return status;
	}

	const char* getDeviceName(int portIndex){
		int port = portIndex;
		if(portIndex<0){
			port = portUsed;
		}
		if( port < 0){
			return "";
		}
		std::string portName = midiin->getPortName(portIndex);
		return portName.c_str();
	}

	
	unsigned int getInPortCount(){
		return midiin->getPortCount();
	}
	const char* getInPortName(unsigned int port){
		if(midiin != NULL && midiin->getPortCount() > port){
			std::string name = midiin->getPortName(port);
			//std::cout << " [DEBUG] Input Port #" << port+1 << ": " << name << '\n';
			//std::cout << " [DEBUG] C STR Input Port #" << port+1 << ": " << name.c_str() << '\n';
			return name.c_str();
		}
		return "";
	}

	unsigned int getOutPortCount(){
		return midiout->getPortCount();
	}

	const char* getOutPortName(unsigned int port){
		if(midiout != NULL && midiout->getPortCount() > port){
			std::string name = midiout->getPortName(port);
			return name.c_str();
		}
		return "";
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MIDI Input

	int noteStatus(int noteId){
		return notesStatusVector[noteId];
	}

	void notesStatus(std::vector<int> *notes){
		//check that *notes is not null
		if(notes == NULL){
			return;
		}
		//clear and copy the current vector
		notes->clear();
		//1 to 128 index, this keeps the real id
		for(int i=1; i<MIDI_STATUS_VECTORS_SIZE-1;i++){
			notes->push_back(notesStatusVector[i]);
		}
	}

	void notesStatusTimestamps(std::vector<double> *notes){
		//check that *notes is not null
		if(notes == NULL){
			return;
		}
		//clear and copy the current vector
		notes->clear();
		//1 to 128 index, this keeps the real id
		for(int i=1; i<MIDI_STATUS_VECTORS_SIZE-1;i++){
			notes->push_back(notesStatusTimestampsVector[i]);
		}
	}
	
	//get next message  TODO make it C compatible somehow
	//std::vector< unsigned char > getNextMessage(){
	//	std::vector< unsigned char > lm = messagesQueue.front();
	//	messagesQueue.pop();
	//	return lm;
	//}
	//get all messages in the queue

	//get next noteOn or noteOff message
	MidiNoteMessage getNextNoteMessage(void) {
		//get reference
		if(notesMessagesQueue.size() > 0){
			MidiNoteMessage nm = notesMessagesQueue.front();
			//erase it
			notesMessagesQueue.pop();
			return nm;
		}
		//do something to return an empty message
		MidiNoteMessage nm;

		return nm;
	}
	//get all messages on the notes queue

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


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MIDI Output

	void noteOn(unsigned char id,unsigned char velocity, int channel){
		//channel not used yet
		std::vector<unsigned char> message;
		// Note On: 144, note id, velocity
		message.push_back(NOTE_ON_MESSAGE);
		message.push_back(id);
		message.push_back(velocity);
		midiout->sendMessage( &message );
	}
	
	void noteOff(unsigned char id, int channel){
		std::vector<unsigned char> message;
		// Note Off: 128, note id, velocity
		message.push_back(NOTE_OFF_MESSAGE);
		message.push_back(id);
		message.push_back(0);
		midiout->sendMessage( &message );	
	}
}
