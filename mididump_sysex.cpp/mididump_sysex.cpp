// mididump.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>
#pragma comment (lib, "winmm.lib")
char * msgbuffer = nullptr;
HMIDIIN hmi;
MIDIHDR mi_hdr;
void print_available_midiinput_devices()
{
	UINT num_mo;
	MIDIINCAPS caps;
	printf("midiinput devices\n");
	num_mo = midiInGetNumDevs();
	for (UINT i = 0; i < num_mo; i++) {
		MMRESULT mr;
		mr = midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
		if (mr == MMSYSERR_NOERROR) {
			printf("%d ", i);
			wprintf(L"%s\n", caps.szPname);
		}
		else {
			printf("midiInGetNumDevs error");
		}
	}
}

void CALLBACK cbMidiIn(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	DWORD tick_cnt;
	tick_cnt = GetTickCount();
	MIDIHDR hdr;
	int prepare_hdr_index;
	int unprepare_hdr_index;
	int addbuff_hdr_index;
	int i;
	char msg_type[20];
	char tmpbuff[32];
	if (wMsg == MIM_OPEN) {
		sprintf_s(msg_type, "%s", "MIM_OPEN");
	}else if (wMsg == MIM_CLOSE) {
		sprintf_s(msg_type, "%s", "MIM_CLOSE");
	}else if (wMsg == MIM_DATA) {
		sprintf_s(msg_type, "%s", "MIM_DATA");
	}else if (wMsg == MIM_LONGDATA) {
		sprintf_s(msg_type, "%s", "MIM_LONGDATA");
		hdr = (MIDIHDR)*((LPMIDIHDR)dwParam1);
		
		strncpy_s(tmpbuff, hdr.lpData, hdr.dwBytesRecorded);
		while (midiInUnprepareHeader(hmi, (LPMIDIHDR)dwParam1, sizeof(MIDIHDR)) == MIDIERR_STILLPLAYING);
		((LPMIDIHDR)dwParam1)->dwFlags = 0;
		midiInPrepareHeader(hmi, (LPMIDIHDR)dwParam1, sizeof(MIDIHDR));
		midiInAddBuffer(hmi, (LPMIDIHDR)dwParam1, sizeof(MIDIHDR));
	}else if (wMsg == MIM_ERROR) {
		sprintf_s(msg_type, "%s", "MIM_ERROR");
	}else if (wMsg == MIM_LONGERROR) {
		sprintf_s(msg_type, "%s", "MIM_LONGERROR");
		hdr = *((LPMIDIHDR)dwParam1);

		strncpy_s(tmpbuff, hdr.lpData, hdr.dwBytesRecorded);
		while (midiInUnprepareHeader(hmi, (LPMIDIHDR)dwParam1, sizeof(MIDIHDR)) == MIDIERR_STILLPLAYING);
		((LPMIDIHDR)dwParam1)->dwFlags = 0;
		midiInPrepareHeader(hmi, (LPMIDIHDR)dwParam1, sizeof(MIDIHDR));
	}else if (wMsg == MIM_MOREDATA) {
		sprintf_s(msg_type, "%s", "MIM_MOREDATA");
	}
	else {
		sprintf_s(msg_type, "%s", "MIM_??");
	}
	printf("%ld,%04x(%s)%08lx,%08lx,%08lx\n", tick_cnt, wMsg, msg_type, dwInstance, dwParam1, dwParam2);
	if (wMsg == MIM_LONGDATA || wMsg == MIM_LONGERROR) {
		printf("SysEx=");
		for (i = 0; i < hdr.dwBytesRecorded; i++) {
			printf("%hhx,", hdr.lpData[i]);
		}
		printf("\n");
	}

}
int main(int argc, char *argv[])
{
	UINT midi_id;
	MMRESULT rslt;

	SYSTEMTIME local_sys_time;
	int i;

	GetLocalTime(&local_sys_time);

	if (argc <= 1) {
		print_available_midiinput_devices();
	}
	midi_id = atoi(argv[1]);
	rslt = midiInOpen(&hmi, midi_id, (DWORD_PTR)cbMidiIn,0x0, CALLBACK_FUNCTION |MIDI_IO_STATUS);

	if (rslt != MMSYSERR_NOERROR) {
		printf("midiInOpen() error");
		return -1;
	}
	
	__try {
		msgbuffer = (char *)malloc(32);
		if (msgbuffer == nullptr) {
			printf("malloc() error");
			return -1;
		}
		ZeroMemory(msgbuffer, 32);
		mi_hdr.lpData = msgbuffer;
		mi_hdr.dwBufferLength = 32;
		mi_hdr.dwBytesRecorded = 0;
		mi_hdr.dwFlags = 0;
		//printf("&mi_hdr=%x;", &mi_hdr);
		rslt = midiInPrepareHeader(hmi, &mi_hdr, sizeof(MIDIHDR));
		if (rslt != MMSYSERR_NOERROR) {
			printf("midiInPrepareHeader() error");
			return -1;
		}
		rslt = midiInAddBuffer(hmi, &mi_hdr, sizeof(MIDIHDR));
		if (rslt != MMSYSERR_NOERROR) {
			printf("midiIAddBuffer() error");
			return -1;
		}
		printf("start_time %d. %02d. %02d %2d: %02d : %02d", local_sys_time.wYear, local_sys_time.wMonth, local_sys_time.wDay,
			local_sys_time.wHour, local_sys_time.wMinute, local_sys_time.wSecond);
		
		if (midiInStart(hmi) != MMSYSERR_NOERROR) {
			printf("midiInStart() error");
			return -1;
		}

		while (1) {
			Sleep(1000);
		}
	}
	__finally {
		midiInStop(hmi);
		midiInClose(hmi);
		if (msgbuffer != nullptr) {
			free(msgbuffer);
		}
	}
    return 0;
}

