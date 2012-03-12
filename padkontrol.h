#pragma once

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


typedef struct {
    // public callbacks, must be set before calling pk_init()   
    void (*padmsg)(void*, int pad, int on, int vel);
    void (*facemsg)(void*, int button, int down);
    void (*knobmsg)(void*, int knob, int value);
    void (*encodermsg)(void*, int dec);
    void (*xymsg)(void*, int x, int y);
    
    
    // private:
    int devi, devo;

} PadKontrol;

int pk_init(PadKontrol* pad, int devi, int devo);
void pk_process(PadKontrol* pad);
void pk_deinit(PadKontrol* pad);

void pk_setbuttonstate(ButtonLightSpec button, ButtonState state);
void pk_setledmsg(char abc[3], char flash);
