#include <stdio.h>
#include <mpd/client.h>

#include "padkontrol.h"

int radio_mode = 0;

void set_radiomode(PadKontrol* pk, int enable)
{
    radio_mode = enable != 0;
    pk_buttonstate(pk, ButtonLightSpec_Port, enable ? ButtonState_On : ButtonState_Off);
    
    int i;
    for (i = 0; i < 16; i++) {
        pk_buttonstate(pk, ButtonLightSpec_Pad01 + i, enable ? ButtonState_On : ButtonState_Off);
    }
    
}

void pad_callback(PadKontrol* pk, int pad, int on, int vel)
{
  if (radio_mode) {
  
    set_radiomode(pk, 0);
    struct mpd_connection* conn = mpd_connection_new("mpd.lan", 6600, 0);
    
//    mpd_run_save(conn, "default");
    
    mpd_run_clear(conn);
    
    mpd_run_load(conn, "radio");
    mpd_run_play_pos(conn, pad);
    
    mpd_connection_free(conn);
    
    
    return;
  }
//  if(on) native_buttonstate(pad, pad<8?ButtonState_Flash:ButtonState_FlashLong, midi);

  if (on) {
  #define min(X, Y)  ((X) < (Y) ? (X) : (Y))
 #define max(X, Y)  ((X) < (Y) ? (Y) : (X))
    static int r, g, b, gnome;
    if (pad == 0) {
      r = 0; g = 0; b = 0; gnome = 0;
    } else if (pad == 4) {
      r = 255; g = 255; b = 255; gnome = 255;
    }
    
    vel /= 4;

    if (pad == 1) r = max(0, r - vel);
    if (pad == 2) g = max(0, g - vel); 
    if (pad == 3) b = max(0, b - vel); 
    if (pad == 5) r = min(255, r + vel);
    if (pad == 6) g = min(255, g + vel);
    if (pad == 7) b = min(255, b + vel);
  
    if (pad == 9) {
        http_request("http://mpd.lan/rainbow.php?enable=1");
        return;
    }
    
    if (pad == 10) gnome = 0; 
    if (pad == 11) gnome = 255; 

    if (pad == 8) system("ssh miau@mpd");
     
    omlumlum_set_color(r, g, b, gnome);
  }  
  
  printf("Pad #%02hhd is %s, velocity %03hhd\n", pad +1, on?"down":"up", vel);
  
  return;
}

void face_callback(PadKontrol* pad, int button, int down)
{
  if (down) {
    struct mpd_connection* conn = mpd_connection_new("mpd.lan", 6600, 0);

    if (button == 0x0b) {
        mpd_run_previous(conn);
    } else if (button == 0x0c) {
        mpd_run_next(conn);
    } else if (button == 0x0f) {
    
      struct mpd_status* status = mpd_run_status(conn);

      if (MPD_STATE_STOP == mpd_status_get_state(status)) {
        mpd_run_play(conn);
      } else {
        mpd_run_toggle_pause(conn);
      }
      mpd_status_free(status);
      
    } else if (button == 0x08) {
        set_radiomode(pad, !radio_mode);
    }

    
    mpd_connection_free(conn);
  }
  
  const char *string;

  if(button <= 0x12) {
    string = FaceButtonStrings[button];
  } else if(button == 0x20) {
    // 0x20 is the XY pad up/down state
    string = FaceButtonStrings[0x14];
  } else {
    // nothing else is specified
    string = FaceButtonStrings[0x13];
  }
  
  printf("%s (%02hhx) is %s\n", string, button, down?"down":"up");
}


void encoder_callback(PadKontrol* pad, int dec) {
  struct mpd_connection* conn = mpd_connection_new("mpd.lan", 6600, 0);

  struct mpd_status* status = mpd_run_status(conn);

  int volume = mpd_status_get_volume(status);
  if (volume > 0 && volume < 100) {
    volume += dec ? -1 : 1;
    mpd_run_set_volume(conn, volume);

    char buf[4];
    snprintf(buf, 4, "%d", volume);
    pk_ledmsg(pad, buf, 0);
  }

  mpd_status_free(status);
  mpd_connection_free(conn);
}

void xy_callback(PadKontrol* pad, int x, int y)
{
}

void knob_callback(PadKontrol* pad, int knob, int value)
{
}



/*
    void (*padmsg)(PadKontrol*, int pad, int on, int vel);
    void (*facemsg)(PadKontrol*, int button, int down);
    void (*knobmsg)(PadKontrol*, int knob, int value);
    void (*encodermsg)(PadKontrol*, int dec);
    void (*xymsg)(PadKontrol*, int x, int y);
*/

int main(int argc, char** argv)
{
    pk_printdevicelist(argc, argv);
    
    PadKontrol pad;
    
    pad.padmsg = pad_callback;
    pad.facemsg = face_callback;
    pad.encodermsg = encoder_callback;
    pad.knobmsg = knob_callback;
    pad.xymsg = xy_callback;
    
    pk_init(&pad, 5, 4);
    pk_process(&pad);
    pk_deinit(&pad);
}
