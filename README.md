# rtmidi-c-wrapper
A C wrapper layer for RtMidi. I did this to target Unity platform as I'm playing with Unity and my electric piano

This library uses RtMidi as a base and creates a façade in C to be used from any source that needs native C code interface.
The intention is to use it with Unity3D but may be useful for other things.

For information on RtMidi see:

https://www.music.mcgill.ca/~gary/rtmidi/

### Basic usage:

```
	//Environment
	setupEnv(); //MANDATORY CALL BEFORE ANYTHING ELSE
	////////////////////////////////
	//Input handling
	///////////////////////////////
	//Input Setup
	createInput();
	//checking how many input ports there are
	unsigned int i_ports = getInPortCount();
	//opening an input midi port
	unsigned int port_num = 0;
	openInputPort(port_num);
	//checking if it's open
	isInputPortOpen();
	//getting input port names
	unsigned char[512] name; 
	getInputPortName(name,port_num);
	///Getting the messages is a bit more complex and needs bit shifting:
	long msg = getNextMessageAsLong();
	//bytes are ordered from the highest significant byte to the lowest, mainly the messages are noteOn and noteOff
	//the same can be obtained as an unsigned int, the message is maximum 4 bytes
	unsigned int msgi = getNextMessageAsUInt();
	
	
	////////////////////////////////
	//Output Handling
	////////////////////////////////
	output handling is almost the same a sinput handling
	createOutput();
	openOutputPort(int port = 0);
	int is_out_open = isOutputPortOpen();
	unsigned int o_count = getOutPortCount();
	getOutputPortName(name,port_num);
	//MIDI Output, for notes there is direcly a handler because it is my main goal
	void noteOn(byte id, byte velocity, int channel);
	//and
	noteOff(byte  id, int channel);
	
	//closing ports
	closeInputPort();
	closeOutputPort();
	
	//termination
	destroyInput();
	destroyOutput();
	cleanupInputEnv();

```

For more examples soon I'll be uploading the complete C# Unity wrapper I'm using

Please feel free to improve the wrapper and ask for a pull request.

If you use and improve the wrapper please be kind to share the improvements with me so I can merge them.

