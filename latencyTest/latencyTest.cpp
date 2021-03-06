// latencyTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <jack/jack.h>
#include <jack/transport.h>
#include <jack/midiport.h>

typedef jack_default_audio_sample_t sample_t;
jack_client_t *client;
jack_port_t *input_port; //input is wave
jack_port_t *output_port; //out is midi
const char * client_name = "latencyTest";
unsigned long sr;
long sleep_duration = 20000;
long note_on_duration = 1000;
int on_off=0;
long keep_nframes=0;
int proc_cnt=0;
float threshold =0.01;

struct log_struct {
	char type;
	DWORD val;
};
struct log_struct mylog[1024];
int logindx;
void logmsg(unsigned char type, DWORD val) {
	mylog[logindx].type = type;
	mylog[logindx].val = val;
	logindx++;
	if (logindx >= 1024) {
		logindx -= 1024;
	}
}
int process(jack_nframes_t nframes, void *arg)
{
	int i, j;
	sample_t *in, *out;
	static int prevent_chatter_tick_cnt = 0;
	jack_midi_event_t out_event;
	unsigned char *buff;
	in = (sample_t *)jack_port_get_buffer(input_port, nframes);
	void* port_buf = jack_port_get_buffer(output_port, nframes);
	jack_midi_clear_buffer(port_buf);
	DWORD tick_cnt;
	
	/*memset(buffer, 0, nframes*sizeof(jack_default_audio_sample_t));*/
	proc_cnt++;
	tick_cnt = GetTickCount();
	for (i = 0; i < nframes; i++) {
		if (keep_nframes <= 0) {
			//toggle on off;
			buff = jack_midi_event_reserve(port_buf, i, 3);
			if (on_off == 1) {
				on_off = 0;
				keep_nframes = sleep_duration*sr / 1000;
				buff[2] = 0;   //velocity
				buff[1] = 60;  //note number 
				buff[0] = 0x81; //noteoff
				logmsg('p', (DWORD)proc_cnt);
				logmsg('t', (DWORD)tick_cnt);
				logmsg('f', (DWORD)nframes);
				logmsg('m', (DWORD)(buff[2] << 16) | (buff[1] << 8) | buff[0]);
			}
			else {
				on_off = 1;
				keep_nframes = note_on_duration * sr/ 1000;
				buff[2] = 127; //velocity
				buff[1] = 60;  //note number 
				buff[0] = 0x91; //noteon
				logmsg('p', (DWORD)proc_cnt);
				logmsg('t', (DWORD)tick_cnt);
				logmsg('f', (DWORD)nframes);
				logmsg('m', (DWORD)(buff[2] << 16) | (buff[1] << 8) | buff[0]);
			}
		}
		if (in[i] >0.001) {
			//printf("%f", in[i]);
		}
		if (in[i] > threshold) {
			if (tick_cnt - prevent_chatter_tick_cnt > 5000) {
				logmsg('p', (DWORD)proc_cnt);
				logmsg('t', (DWORD)tick_cnt);
				logmsg('f', (DWORD)nframes);
				logmsg('i', 0);
			}
			prevent_chatter_tick_cnt = tick_cnt;
		}
		keep_nframes--;
	}
	return 0;
}
int main() 
{
	jack_status_t status;
	ZeroMemory(mylog, sizeof(mylog));
	if ((client = jack_client_open(client_name, JackNoStartServer, &status)) == 0) {
		fprintf(stderr, "jack server not running?\n");
		return 1;
	}
	jack_set_process_callback(client, process, 0);
	input_port = jack_port_register(client, "audio_in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	output_port = jack_port_register(client, "midi_out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

	sr = jack_get_sample_rate(client);

	if (jack_activate(client)) {
		fprintf(stderr, "cannot activate client");
		return 1;
	}

	Sleep(30000);//30秒スリープ
	jack_deactivate(client);
		//print all log;
	int i;
	for (i = 0; i < 1024; i++) {
		if (mylog[i].type != 0) {
			printf("%c:%08lx\n", mylog[i].type, mylog[i].val);
		}
	}
	

	return 0;
}