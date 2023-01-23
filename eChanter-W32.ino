//See LICENSE for usage/copyright information -----------------------------------------------------------------------------
// Poll sensors to determine touches. Using touches, determine a musical not to be played, then play a WAV sample of that note via I2S.
// Configure system paprmeters via embedded (WiFi) web server.
// [Optional] emulate a wind instrument by implementing a pressure sensor. Poll the pressure sensor, turning audio on/off based
//      on a pressure threshold.
//
// I2S playback: Play a WAV file from memory, stored in files within this directory.
// 		This version supports mono or stereo sound but must be 16 bit sound, any sample speed
// 		Will display the wav file stats before playing
//
//------------------------------------------------------------------------------------------------------------------------

/* eChanter-W32 using Adafruit ESP32 Feather
 * 		Note -> GPIO - > touch channel
 * 		--  		4    	touch0 (also A5)
 * 		LA  		2	    touch2
 * 		B   		15		touch3
 * 		C   		13   	touch4
 * 		D   		12   	touch5
 * 		E   		14   	touch 6
 * 		F   		27   	touch 7
 * 		HG    		32   	touch 8
 * 		HA    		33   	touch 9
 *
 * [Optional] PRessure sensor analog output
 *          36/A4
 *
 * I2S Audio
 * 		LRC left/right clock	25
 * 		BCLK bit clock			26
 * 		DIN data in/out			22
 *
 * 21 PCM Audio output
 *		OUT						21
 *
 * Default touch values
 * 		 6		touch with pressure
 *  	12		full touch
 *  	20		light touch
 *		50 		no touch
 *
*/


//------------------------------------------------------------------------------------------------------------------------
// Includes
#include "xt_i2s.h" 		      	// I2S related code
#include "ghb_samples.h"			// The Wav file of samples, to be stored in memory, should be in folder with this file

//------------------------------------------------------------------------------------------------------------------------
// Defines
#define ENABLE_DRONE false					// use drone samples

#define NO_TOUCH_VAL 50
#define LIGHT_TOUCH_VAL 20
#define FULL_TOUCH_VAL 12
#define Heavy_TOUCH_VAL 6


//------------------------------------------------------------------------------------------------------------------------
//  Global Variables/objects
int num_gpio = 8;
int gpio[] = {33,32,27,14,12, 4 /*13*/,15,2,};  // gpio 4 should be 13, but 13 is damaged on dev board
char note_names [] = {'A', 'G', 'F', 'E', 'D', 'C', 'B', 'a'}; // low 'g' implied by all sensors touched

bool drone = false;
int touchVal = 5000;
int note = 0;   //hg by default
int prev_note = 0;

TaskHandle_t playSampleTask;

struct WavHeader_Struct WH_HA, WH_HG, WH_F, WH_E, WH_D, WH_C, WH_B, WH_LA, WH_LG;


void setup() {
    const unsigned char *WavFile=ghb_ha; //WavData16BitStereo;
    Serial.begin(115200);
    memcpy(&WH_HA,ghb_ha,44);                     // Copy the header part of the wav data into our structure
    memcpy(&WH_HG,ghb_hg,44);                     // Copy the header part of the wav data into our structure
    memcpy(&WH_F,ghb_f,44);                     // Copy the header part of the wav data into our structure
    memcpy(&WH_E,ghb_e,44);                     // Copy the header part of the wav data into our structure
    memcpy(&WH_D,ghb_d,44);                     // Copy the header part of the wav data into our structure
    memcpy(&WH_C,ghb_c,44);                     // Copy the header part of the wav data into our structure
    memcpy(&WH_B,ghb_b,44);                     // Copy the header part of the wav data into our structure
    memcpy(&WH_LA,ghb_la,44);                     // Copy the header part of the wav data into our structure
    memcpy(&WH_LG,ghb_lg,44);                     // Copy the header part of the wav data into our structure
    WavHeader = &WH_HA;
    DumpWAVHeader(WavHeader);                          // Dump the header data to serial, optional!
    if(ValidWavData(WavHeader))
    {
      i2s_driver_install(i2s_num, &i2s_config, 0, NULL);        // ESP32 will allocated resources to run I2S
      i2s_set_pin(i2s_num, &pin_config);                        // Tell it the pins you will be using
      i2s_set_sample_rates(i2s_num, WavHeader->SampleRate);      //set sample rate
      TheData=WavFile+44;                                       // set to start of data
    }
    else      // end code here
      while(true);

    xTaskCreatePinnedToCore(
                    playSample,       /* Task function. */
                    "playSample",     /* name of task. */
                    10000,            /* Stack size of task */
                    NULL,             /* parameter of the task */
                    0,                /* priority of the task */
                    &playSampleTask,  /* Task handle to keep track of created task */
                    0);               /* pin task to core 0 */

}

void loop()
{

play = true;
  bool note_found = false;

  for (int i=0; i<num_gpio; i++) {
  	touchVal = touchRead(gpio[i]);
//Serial.println(touchVal);
	  if (touchVal > LIGHT_TOUCH_VAL) {
		  note_found = true;
		  note = i;
     break;
	  }

  }

  if (!note_found) note = 8; // ie low g

  if (note != prev_note) {

    play = false;

    prev_note = note;
//    memcpy(&WavHeader,ghb_lg,44); TheData=ghb_lg+44;

	  switch (note) {
		case 0: WavHeader = &WH_HA; TheData=ghb_ha+44;
				break;
		case 1:  WavHeader = &WH_HG; TheData=ghb_hg+44;
				break;
		case 2:  WavHeader = &WH_F; TheData=ghb_f+44;
				break;
		case 3:  WavHeader = &WH_E; TheData=ghb_e+44;
				break;
		case 4:  WavHeader = &WH_D; TheData=ghb_d+44;
				break;
		case 5:  WavHeader = &WH_C; TheData=ghb_c+44;
				break;
		case 6:  WavHeader = &WH_B; TheData=ghb_b+44;
				break;
		case 7:  WavHeader = &WH_LA; TheData=ghb_la+44;
				break;
		case 8:  WavHeader = &WH_LG; TheData=ghb_lg+44;
				break;
		default:  WavHeader = &WH_HA; TheData=ghb_ha+44;
				break;
	  }

    DataIdx=0;

    play = true;
  
  }

}
