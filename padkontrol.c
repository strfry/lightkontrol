/****************************************************************************

  2012 Jonathan Sieber: modified for mpd and light control automation

  Korg padKontrol native mode reference code and demo.
  
  (C)2009 Stephen Mosher
  Contact: smosher_@hotmail.com

To build:

  Make sure the portmidi development library (eg. portmidi-dev) is
  installed, link against it:
  
  gcc native_demo.c -o native_demo -lportmidi

  This code was written and tested on Linux, but it should be portable.

Running:

  Run without args for usage, and to list devices. First arg is input
  device, second is output.  Ex:
  
          ...
          Device: # 8 (output) 'padKONTROL MIDI 1'
          Device: # 9 (input ) 'padKONTROL MIDI 1'
          Device: #10 (output) 'padKONTROL MIDI 2'
          Device: #11 (input ) 'padKONTROL MIDI 2'
          Device: #12 (input ) 'padKONTROL MIDI 3'
  
  I need to use "padKONTROL MIDI 2", so I invoke like so:
  
    ./native_demo 11 10

*****************************************************************************

Demo functions:
  Full report from the device on all pads/buttons/knobs/etc.
  Face buttons toggle flashing on/off
  Pads flash when triggered (1~8 shortest flash, 9~16 longest)
  Simple 7seg messages on knobs and xy pad.
  
TODO:
  Add a function for manipulating individual 7seg LEDs, including the
  decimals.
  
Related materials:
  
  Page related to this code:
  http://comichunter.net/nowhere/pK_native/
  
  Korg pK SysEx manual (elsewhere referred to as 'the PDF'):
  http://www.thecovertoperators.org/uploads/PadKONTROL%20imp.pdf
  (via) http://www.thecovertoperators.org/
  
  Hap's guide (very handy companion):
  http://www.mediafire.com/file/lkmwfajtjzg/padkontrol-native-mode.pdf
  (via) http://www.korgforums.com/forum/phpBB2/viewtopic.php?t=28030

Interesting functions:
  - The start_native() function has notes about entering native mode.
  - The native_buttonstate() and native_ledmsg() are all that's needed to
  update the lights.
  - The print_...msg() functions call all of the light updates in addition
  to printing received messages.
  - The poll_native() function classifies all received messages.
  
A note on the code:
  This code is meant to be a handy reference and as a result good coding
  style and good practices have occasionally been sacrificed in favour of
  brevity and information locality, and in the interest of getting it
  finished quickly.  You are free to reuse any code found in this source
  file.
  
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <portmidi.h>
#include <porttime.h>
#include <string.h>
#include <unistd.h>

// define this to see (most) raw messages (sent and received):
#undef BUGZ

// sleep time in usec, dramatically reduces CPU usage
#define POLLING_USLEEP	1000

#include "padkontrol.h"


const char *FaceButtonStrings[] = {
  "Scene Button",
  "Message Button",
  "Setting Button",
  "Note/CC Button",
  "MIDI CH Button",
  "SW Type Button",
  "Rel. Val. Button",
  "Velocity Button",
  "Port Button",
  "Fixed Velocity Button",
  "Prog. Change Button",
  "X Button",
  "Y Button",
  "Knob 1 Assign Button",
  "Knob 2 Assign Button",
  "Pedal Button",
  "Roll Button",
  "Flam Button",
  "Hold Button",
  "UNKNOWN BUTTON",
  "XY Pad"
};

void sysex_print(char *buf) {
  int i;
  printf("Sending SysEx message:");
  for(i=0;;i++) {
    if(!(i%4)) printf("\n\t");  
    printf("%02hhx ", buf[i]);
    
    if(buf[i] == (char)0xf7) break;
  }
  printf("\n");
  
  return;
}

void pk_buttonstate(PadKontrol* pad, ButtonLightSpec button, ButtonState state)
{
  char buffer[] = {0xf0, 0x42, 0x40, 0x6e, 0x08, 0x01, 0x00, 0x00, 0xf7};
  
  buffer[6] = button;
  buffer[7] = state;
  
#ifdef BUGZ
  sysex_print(buffer);
#endif

  Pm_WriteSysEx(pad->midi_out, 0, buffer);
}

void pk_ledmsg(PadKontrol* pad, char abc[3], char flash) {
  char led[] = {
    0xf0, 0x42, 0x40, 0x6e, 
    0x08, 0x22, 0x04, 0x00, 
    0x29, 0x29, 0x29, 0xf7
  };
  
  // this accepts ascii codes from 0x20~0x7f according to the pdf.
  // note: any character in this range that it cannot print will be converted to space
  led[7] = flash & 0x1;
  led[8] = abc[0];
  led[9] = abc[1];
  led[10] = abc[2];

#ifdef BUGZ  
  sysex_print(led);
#endif
  
  Pm_WriteSysEx(pad->midi_out, 0, led);
}

// note: msg must be pre-allocated
// (this function is needlessly complicated for personal reasons)
PmError read_native(PmStream *midi, char *msg) {
  PmError c;
  PmEvent buf;
  int header=0, body=0, complete=0;
  
  while(c=Pm_Read(midi, &buf, 1) >0) {
    if(c<0) return c;
#ifdef BUGZ
    printf("buffer: 0x%08lx\n", buf.message);
#endif
    if(buf.message == 0x6e4042f0) header=1; // device header
    if((buf.message == 0xf7) && body) { 
      complete = 1; 
      break;
    }
    
    if(buf.message & 0x08) { // this is actually part of the device header
      body=1;
      msg[0] = (char)(buf.message>>8);
      msg[1] = (char)(buf.message>>16);
      msg[2] = (char)(buf.message>>24);
    }
  }
  
  return complete;
}

void print_nativestatemsg(char *msg) {
  printf("Native mode is now %s\n", msg[2]==0x03?"enabled":"disabled");
  
  return;
}

void print_datadumpmsg(char *msg) {
  if(msg[1] == 0x00 || msg[1] == 0x01) {
    // packet comm
    printf("Packet %hhd received, %s\n", msg[1], msg[2]?"Err":"OK");
  } else {
    // data dump: not used here
    printf("Data dump with params: 0x%02hhx 0x%02hhx\n", msg[1], msg[2]);
  }
  
  return;
}


void print_unknownmsg(char *msg) {
  printf("Unknown command: 0x%02hhx(0x%02hhx, 0x%02hhx)\n", msg[0], msg[1], msg[2]);
  
  return;
}

enum {
  NativeMessage_NativeState	= 0x40,
  NativeMessage_DataDump	= 0x5f,
  
  NativeMessage_Pad	= 0x45,
  NativeMessage_Face	= 0x48,
  NativeMessage_Knob	= 0x49,
  NativeMessage_Encoder = 0x43,
  NativeMessage_XY	= 0x4b
} NativeMessage;

void poll_native(PadKontrol* pad) {
  char msg[3];
  PmError c;
  int lights[0x13];
  
  memset(&lights, 0, sizeof(lights));
  
  for(;;) {
    c=read_native(pad->midi_in, msg);
    
    if(!c) {
      usleep(POLLING_USLEEP);
      continue;
    }
    
    if(c<0) return;	// error condition
    
    switch(msg[0]&0xff) {
      case NativeMessage_NativeState:
        print_nativestatemsg(msg);
        break;
      case NativeMessage_DataDump:
        print_datadumpmsg(msg);
        break;
        
      case NativeMessage_Pad:
        pad->padmsg(pad, msg[1] & 0x3f, msg[1] & 0x40, msg[2]);
        break;
        
      case NativeMessage_Face:
        pad->facemsg(pad, msg[1], msg[2]);
        break;
      case NativeMessage_Knob:
        pad->knobmsg(pad, msg[1], msg[2]);
        break;
      case NativeMessage_Encoder:
        // note: 0x01=inc, 0x7f=dec. rshift converts these to false, true
        pad->encodermsg(pad, msg[2]>>1);
        break;
      case NativeMessage_XY:
        pad->xymsg(pad, msg[1], msg[2]);
        break;
      default:
        print_unknownmsg(msg);
        break;
    }
  }
  
  return;
}

void start_native(PmStream *out) {
  /**************************************************************************
  The only hairy part of setting up native mode is the packet communications
  messages.  These messages init native mode according to tables at the end
  of the PDF.  I've documented them a little here to help people understand
  what's going on.  The sections marked data are from the tables at the end.
  
  Packet comm. #1 specifies the global midi channel, which real-time events
  to transmit to the host, and the midi channels and controller numbers for
  those events (which I gather are still sent, but on a different port,
  while in native mode.)
  
  Packet comm. #2 specifies the initial states for all lights, with the
  exception of the decimals in the 7seg display.
  **************************************************************************/
  
  // native mode in request (section 3-1, fig 15, pg. 6)
  char native_start[] = {
    0xf0, 0x42, 0x40, 0x6e, 0x08, 
    0x00, 0x00, 0x01, 
    0xf7};
    
  
  // packet comm. #1 (section 3-1, fig 17, pg. 6)
  // see table 3 for the packet format 
  // this is needed to init the pK. for communicating
  char native_comms[] = {
    0xf0, 0x42, 0x40, 0x6e, 0x08, 
    0x3f, 0x2a, 0x00, // 0x3f = packet comm, 0x2a = msg len (41 bytes) +1, 0x00 = comm #1
    0x00, 0x05, 0x05, 0x05, 0x7f, 0x7e, 0x7f, 0x7f, 0x03, 0x0a, 0x0a, 0x0a, 0x0a,	// data (  13 bytes)
    0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x01, 0x02, // data (+ 14 bytes)
    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, // data (+ 14 bytes = 41 bytes)
    0xf7}; // EOX

  // packet comm. #2 (section 3-1, fig 19, pg. 7)
  // see table 4 for the packet format    
  // turns off all lights.
  char native_lightsoff[] = {
    0xf0, 0x42, 0x40, 0x6e, 0x08, 
    0x3f, 0x0a, 0x01, 	          // 0x3f = packet comm, 0x0a = msg len (9 bytes) +1, 0x01 = comm #2
    0x00, 0x00, 0x00, 0x00, 	  // data (  4 bytes)
    0x00, 0x00, 0x29, 0x29, 0x29, // data (+ 5 bytes = 9 bytes)
    0xf7}; // EOX
  
  Pm_WriteSysEx(out, 0, native_start);
  Pm_WriteSysEx(out, 0, native_comms);
  Pm_WriteSysEx(out, 0, native_lightsoff);
  
  return;
}



void pk_process(PadKontrol* pad) {  
  if (pad->midi_in == 0) {
    Pm_OpenInput(&pad->midi_in, pad->devi, NULL, 512, NULL, NULL);
  }
  if (pad->midi_out == 0) {
    Pm_OpenOutput(&pad->midi_out, pad->devo, NULL, 512, NULL, NULL, 0);
    start_native(pad->midi_out);
  }  
  
  poll_native(pad);
  return;
}

void pk_printdevicelist(int argc, char** argv)
{
    int dev_count, i;
    PmDeviceID devi, devo;
    const PmDeviceInfo *in, *out;
    
    Pm_Initialize();
    dev_count = Pm_CountDevices();

    printf("\nUsage: %s <input device id> <output device id>\n\n", argv[0]);
    printf("Listing %d devices:\n", dev_count);
    for(i=0;i<dev_count;i++) {
      devi = i;
      in = Pm_GetDeviceInfo(devi);
      if(in==NULL) {
        printf("\tDevice id (%2d) out of range\n", devi);
        break;
      }
      printf("\tDevice: #%2d (%s%s) '%s'\n",
        devi, in->input?"input ":"", in->output?"output":"", 
        in->name);
    }
}

int pk_init(PadKontrol* pad, int devi, int devo) {
  const PmDeviceInfo *in, *out;
  Pm_Initialize();
  
  pad->midi_in = 0;
  pad->midi_out = 0;
  
  pad->devi= devi;
  pad->devo = devo;
  
  Pt_Start(1,0,0);
  
  in=Pm_GetDeviceInfo(devi);
  out=Pm_GetDeviceInfo(devo);
  
  printf("Using device #%2d, '%s' for input\n", devi, in->name);
  printf("Using device #%2d, '%s' for output\n", devo, out->name);
  
  if(!out->output) {
    printf("Selected device is unsuitable for output, exiting.\n");
    return -1;
  }
  
  if(!in->input) {
    printf("Selected device is unsuitable for input, exiting.\n");
    return -1;
  }
  
  
}

void pk_deinit(PadKontrol* pad)
{
  Pm_Terminate();
}

