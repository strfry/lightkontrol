#pragma once

#include <portmidi.h>

typedef enum {
  ButtonState_Off		= 0x00,
  ButtonState_On		= 0x20,
  ButtonState_Flashing		= 0x63,
  ButtonState_Flash		= 0x41,
  // values in between specify alternate flashing durations
  ButtonState_FlashLong		= 0x5f
} ButtonState;

typedef enum {
  ButtonLightSpec_Pad01	= 0x00,
  ButtonLightSpec_Pad02,
  ButtonLightSpec_Pad03,
  ButtonLightSpec_Pad04,
  ButtonLightSpec_Pad05,
  ButtonLightSpec_Pad06,
  ButtonLightSpec_Pad07,
  ButtonLightSpec_Pad08,
  ButtonLightSpec_Pad09,
  ButtonLightSpec_Pad10,
  ButtonLightSpec_Pad11,
  ButtonLightSpec_Pad12,
  ButtonLightSpec_Pad13,
  ButtonLightSpec_Pad14,
  ButtonLightSpec_Pad15,
  ButtonLightSpec_Pad16,
  ButtonLightSpec_Scene,
  ButtonLightSpec_Message,
  ButtonLightSpec_Setting,
  ButtonLightSpec_NoteCC,
  ButtonLightSpec_MidiCH,
  ButtonLightSpec_SWType,
  ButtonLightSpec_RelVal,
  ButtonLightSpec_Vel,
  ButtonLightSpec_Port,
  ButtonLightSpec_FixedVel,
  ButtonLightSpec_ProgChange,
  ButtonLightSpec_X,
  ButtonLightSpec_Y,
  ButtonLightSpec_Knob1,
  ButtonLightSpec_Knob2,
  ButtonLightSpec_Pedal,
  ButtonLightSpec_Roll,
  ButtonLightSpec_Flam,
  ButtonLightSpec_Hold
} ButtonLightSpec;


extern const char *FaceButtonStrings[];


struct PadKontrol_s;
typedef struct PadKontrol_s PadKontrol;

struct PadKontrol_s {
    // public callbacks, must be set before calling pk_init()   
    void (*padmsg)(PadKontrol* pk, int pad, int on, int vel);
    void (*facemsg)(PadKontrol* pk, int button, int down);
    void (*knobmsg)(PadKontrol* pk, int knob, int value);
    void (*encodermsg)(PadKontrol* pk, int dec);
    void (*xymsg)(PadKontrol* pk, int x, int y);
    
    void* callback_context;
    
    // private:
    int devi, devo;
    PmStream *midi_in;
    PmStream *midi_out;

};

int pk_init(PadKontrol* pad, int devi, int devo);
void pk_process(PadKontrol* pad);
void pk_deinit(PadKontrol* pad);
void pk_printdevicelist(int argc, char** argv);

void pk_buttonstate(PadKontrol* pad, ButtonLightSpec button, ButtonState state);
void pk_ledmsg(PadKontrol* pad, char abc[3], char flash);
