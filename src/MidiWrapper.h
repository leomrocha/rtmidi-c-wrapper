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
	typedef struct {
		int code = 0;
		int id = 0;
		int velocity = 0;
		//unsigned char code;
		//unsigned char id;
		//unsigned char velocity;
		double timestamp = 0.0;
	} MidiNoteMessage;

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MIDI Initialization & status
	/**
	 * Setup the environment needed for the system to work (helpers and other variables)
	 */
	EXPORT_DLL void setupEnv();

	/**
	 * sets up an input object.
	 * If exists then disconnects and deletes creating a new connection
	 * also will call cleanup (of all the temp vars and status) for the input
	 * returns 0 if failed, 1 otherwise
	 */
	EXPORT_DLL int createInput();
	
	/**
	 * Cleans all the temporary buffers and variables taht keep the current input status (queue and so on)
	 */
	EXPORT_DLL void cleanupInputEnv();
	
	/**
	  * destroys the input object (if exists)
	  */
	EXPORT_DLL int destroyInput();
	
	/**
	 * Opens the given port, by default the first one available or a virtual port.
	 */
	EXPORT_DLL int openInputPort(int port=0);
	/**
	* If there is a port open will close it
	*/
	EXPORT_DLL void closeInputPort();

	/**
 	 * if the input object exists and has an open port returns 1 (non 0), else 0
	 */
	EXPORT_DLL int isInputPortOpen();

	/**
	 * If the Input object exists will return the number of inputs available
	 */
	EXPORT_DLL unsigned int getInPortCount();
	
	/**
	* copies the given port (if exists) name in the given space
	* @param 
	*/
	EXPORT_DLL void getInputPortName(char* name, unsigned int port = 0);

	/**
	* copies the given port (if exists) name in the static space allocated (max 1024 chars)
	* and returns the pointer back to the caller
	* @param
	*/
	EXPORT_DLL char * getInputPortNamePtr(unsigned int port = 0);

	/**
	 * sets up an output object.
	 * If exists then disconnects and deletes creating a new connection
	 * returns 0 if failed, 1 otherwise
	 */
	EXPORT_DLL int createOutput();
	/**
	 * destroys the input object (if exists)
	 */
	EXPORT_DLL int destroyOutput();
	/**
	* Opens the given port, by default the first one available or a virtual port.
	*/
	EXPORT_DLL int openOutputPort(int port = 0);
	/**
	* If there is a port open will close it
	*/
	EXPORT_DLL void closeOutputPort();
	/**
	* if the output object exists and has an open port returns 1 (non 0), else 0
	*/
	EXPORT_DLL int isOutputPortOpen();
	
	/**
	 * If the Input object exists will return the number of inputs available
	 */
	EXPORT_DLL unsigned int getOutPortCount();

	/**
	* copies the given port id name to the given space
	* @param name
	*/
	EXPORT_DLL void getOutputPortName(char* name, unsigned int port = 0);

	/**
	* copies the given port (if exists) name in the static space allocated (max 1024 chars)
	* and returns the pointer back to the caller
	* @param
	*/
	EXPORT_DLL char * getOutputPortNamePtr(unsigned int port = 0);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//MIDI Input
	/**
	* returns a MidiNoteMessage to the caller
	* if the input queue is not empty will return the next one
	if input queue is EMPTY will return a zero filled message (id, code and velocity are 0)
	* this is a destructive read (the message read will be popped from the queue)
	**/
	EXPORT_DLL MidiNoteMessage __cdecl getNextMessageStruct();

	/**
	* fills the given message with the next message in the queue.
	* the next message in the queue that affects a note
	* this is a destructive read (the message read will be popped from the queue)
	**/
	EXPORT_DLL void __cdecl fillWithNextNoteMessage(MidiNoteMessage &message);

	EXPORT_DLL long getNextMessageAsLong();
	EXPORT_DLL unsigned int getNextMessageAsUInt();
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// MIDI Output
	/**
	 * Sends up to 8 bytes to the output.
	 * @param data: the input to the function, the unsigned long is treated as an 8 byte message
	 * byte 0 is the head of the message, byte 3 (7) will be the latest byte in the message
	 * @param nBytes: the number of bytes to send to the output port. [3,4]//[3-8]
	 **/
	//EXPORT_DLL void sendLimitedMessage(unsigned long data, int nBytes, int channel = 0);
	EXPORT_DLL void sendLimitedMessage(unsigned int data, int nBytes, int channel = 0);

	/**
	* id: midi id of the note to turn on [0-127]
	* velocity: [0-127]
	* channel default 0
	**/
	EXPORT_DLL void noteOn(unsigned char id, unsigned char velocity, int channel = 0);

	/**
	* id: midi id of the note to turn off
	* velocity: [0-127]
	* channel default 0
	**/
	EXPORT_DLL void noteOff(unsigned char  id, int channel = 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Non extern functions, that later will need to either be adapted for extern c or discarded

RtMidi* getMidiIn();
RtMidi* getMidiOut();

///////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //MIDIWRAPPER