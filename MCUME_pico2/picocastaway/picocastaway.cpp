#include "pico.h"
#include "pico/stdlib.h"

extern "C" {
  #include "iopins.h"  
  #include "emuapi.h"  
}
#include "keyboard_osd.h"

//extern "C" {
#include "emu.h"
//}
#include <stdio.h>

#include <stdio.h>
#include "pico_dsp.h"


volatile bool vbl=true;

bool repeating_timer_callback(struct repeating_timer *t) {
    if (vbl) {
        vbl = false;
    } else {
        vbl = true;
    }   
    return true;
}

PICO_DSP tft;
static int skip=0;

#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "hdmi_framebuffer.h"

int main(void) {
//    vreg_set_voltage(VREG_VOLTAGE_1_05);
//    set_sys_clock_khz(125000, true);    
//    set_sys_clock_khz(150000, true);    
//    set_sys_clock_khz(133000, true);    
//    set_sys_clock_khz(200000, true);    
//    set_sys_clock_khz(210000, true);    
//    set_sys_clock_khz(230000, true);    
//    set_sys_clock_khz(225000, truxe);

    // Overclock!
//    set_sys_clock_khz(280000, true);
    set_sys_clock_khz(260000, true); // for PSRAM tolerance 
    *((uint32_t *)(0x40010000+0x58)) = 2 << 16; //CLK_HSTX_DIV = 2 << 16; // HSTX clock/2

    // Overclock!
//    stdio_init_all();
     emu_init();

    char * filename;
#ifdef FILEBROWSER
    while (true) {      
        if (menuActive()) {
            uint16_t bClick = emu_DebounceLocalKeys();
            int action = handleMenu(bClick);
            filename = menuSelection();   
            if (action == ACTION_RUN) {
              break;    
            }
            tft.waitSync();
        }
    }
#endif
    emu_start();
    emu_Init(filename);
    tft.startRefresh();
    struct repeating_timer timer;
    add_repeating_timer_ms(25, repeating_timer_callback, NULL, &timer);    
    while (true) {
        uint16_t bClick = emu_DebounceLocalKeys();
        emu_Input(bClick);  
        emu_Step();               
    }
}

static unsigned short palette16[PALETTE_SIZE];
void emu_SetPaletteEntry(unsigned char r, unsigned char g, unsigned char b, int index)
{
    if (index<PALETTE_SIZE) {
        palette16[index]  = RGBVAL16(r,g,b);        
    }
}

void emu_DrawLinePal16(unsigned char * VBuf, int width, int height, int line) 
{
    if (skip == 0) {
         tft.writeLinePal(width,height,line, VBuf, palette16);
    }
}

void emu_DrawLine16(unsigned short * VBuf, int width, int height, int line)
{
    if (skip == 0) {
        tft.writeLine(width,height,line, VBuf);
    }
}

int emu_IsVga(void)
{
    return (tft.getMode() == MODE_VGA_320x240?1:0);
}

void emu_DrawVsync(void)
{
    skip += 1;
    skip &= VID_FRAME_SKIP;
#ifdef USE_VGA
    tft.waitSync();            
#else                      
    volatile bool vb=vbl;
    while (vbl==vb) {};
#endif  
}

/*
void emu_DrawLine8(unsigned char * VBuf, int width, int height, int line) 
{
    if (skip == 0) {
#ifdef USE_VGA                        
      tft.writeLine(width,height,line, VBuf);      
#endif      
    }
} 

void emu_DrawLine16(unsigned short * VBuf, int width, int height, int line) 
{
    if (skip == 0) {
#ifdef USE_VGA        
        tft.writeLine16(width,height,line, VBuf);
#else
        tft.writeLine(width,height,line, VBuf);
#endif        
    }
}  

void emu_DrawScreen(unsigned char * VBuf, int width, int height, int stride) 
{
    if (skip == 0) {
#ifdef USE_VGA                
        tft.writeScreen(width,height-TFT_VBUFFER_YCROP,stride, VBuf+(TFT_VBUFFER_YCROP/2)*stride, palette8);
#else
        tft.writeScreen(width,height-TFT_VBUFFER_YCROP,stride, VBuf+(TFT_VBUFFER_YCROP/2)*stride, palette16);
#endif
    }
}

int emu_FrameSkip(void)
{
    return skip;
}

void * emu_LineBuffer(int line)
{
    return (void*)tft.getLineBuffer(line);    
}
*/


#ifdef HAS_SND
#include "AudioPlaySystem.h"
AudioPlaySystem mymixer;
#define AUDIO_BUFFER_LEN  (22050/50)
void emu_sndInit() {
  tft.begin_audio(AUDIO_BUFFER_LEN*2, mymixer.snd_Mixer);
  mymixer.start();    
}

void emu_sndPlaySound(int chan, int volume, int freq)
{
  if (chan < 6) {
    mymixer.sound(chan, freq, volume); 
  }
}

void emu_sndPlayBuzz(int size, int val) {
  mymixer.buzz(size,val); 
}

#endif


