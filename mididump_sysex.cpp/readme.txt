mididump_sysex.cpp

This program monitor midi input and print readable hex.

Build with Visual Studio 2017 target x86.
I have not known how toSysEx with windows API.
I find the way. It is an example program.

MIT license.

If you execute 

C:\>mididump_sysex.exe 

then print available midi input port and the number


C:\>mididump_sysex.exe [the number of midi input device]

output is something like bellow.

start_time 2017. 11. 14 11: 38 : 0252659015,03c4(MIM_LONGDATA)00000000,0103a140,00007199
SysEx=f0,55,55,55,f7,
52734109,03c4(MIM_LONGDATA)00000000,0103a140,000196e8
SysEx=f0,55,55,55,f7,
52743578,03c3(MIM_DATA)00000000,00642d90,0001bbe3
52743578,03c3(MIM_DATA)00000000,00002d80,0001bbe3
52743578,03c3(MIM_DATA)00000000,00643590,0001bbe3
52743578,03c3(MIM_DATA)00000000,00003580,0001bbe3
...

first field is GetTickCount
second is data type
3rd meaningless. This may be used if you program your own midi handler.
4th field is the most important.
   if data type == MIM_DATA then it may be MIDI short message.
   MIDI message exclude SysEx is use this format.
   
   data_type == MIM_LONG* this is the pointer of buffer(MIDI_HEADER).
5th midi device driver time.
SysEx=  SysEx (get thru MIDI_HEADER)
