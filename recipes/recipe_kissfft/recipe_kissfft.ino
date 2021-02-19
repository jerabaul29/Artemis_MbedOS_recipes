#include "Arduino.h"
#include "kiss_fft.h"

// on my Artemis board with core 1.2.1
#include "ard_supers/avr/dtostrf.h"

// time base properties
size_t data_len = 64;
float dt_seconds = 0.5f;
float df_hz = 1.0f / dt_seconds;

// signal properties
// if the data duration is not a multiple of the period, energy will be distributed a bit around the peak
// for example: period 5.0f
// if the data duration is an integer multiple of the period, energy will be onlly at the mode (with uncertainty the rounding errors)
// for example: period 8.0f
// the spreading effect can be attenuated by using some windowing (eg Hanning window).
float period_second = 8.0f;
float frequency_hz = 1.0f / period_second;
float amplitude = 2.0f;
float two_pi = 6.283185f;
float omega = two_pi * frequency_hz;

// our objects for taking fft: data in and out, config
kiss_fft_cfg my_cfg_fft;
kiss_fft_cfg my_cfg_ifft;
kiss_fft_cpx * data_time_domain;
kiss_fft_cpx * data_freq_domain;

// a bit of tooling
void print_vect(kiss_fft_cpx * data, size_t data_len, byte type);
constexpr size_t format_buff_len {16};
char format_buff[format_buff_len];

void setup(){
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println(F("------ booted ------"));

  my_cfg_fft  = kiss_fft_alloc(data_len, 0, NULL, NULL);
  my_cfg_ifft = kiss_fft_alloc(data_len, 1, NULL, NULL);

  // --------------------------------------------------------------
  // generate the signal
  
  data_time_domain  = new kiss_fft_cpx[data_len];
  data_freq_domain = new kiss_fft_cpx[data_len];
  
  for (size_t ind=0; ind<data_len; ind++){
    data_time_domain[ind].r = amplitude * cos(omega * ind * dt_seconds);
    data_time_domain[ind].i = 0.0f;
  }

  // --------------------------------------------------------------
  // perform the fft
    
  kiss_fft(my_cfg_fft, data_time_domain, data_freq_domain);

  print_vect(data_time_domain, data_len, 0);
  print_vect(data_freq_domain, data_len, 1);

  // --------------------------------------------------------------
  // erase the time domain data

  for (size_t ind=0; ind<data_len; ind++){
    data_time_domain[ind].r = 0.0f;
    data_time_domain[ind].i = 0.0f;
  }

  // --------------------------------------------------------------
  // perform the ifft

  kiss_fft(my_cfg_ifft, data_freq_domain, data_time_domain);

  // scale on the way back
  for (size_t ind=0; ind<data_len; ind++){
    data_time_domain[ind].r /= (float)data_len;
    data_time_domain[ind].i /= (float)data_len;
  }

  print_vect(data_time_domain, data_len, 0);
  print_vect(data_freq_domain, data_len, 1);

  Serial.println(F("done"));
}

void loop(){
  
}

// type: 0 for time domain, 1 for frequency domain
void print_vect(kiss_fft_cpx * data, size_t data_len, byte type){
  Serial.println(F("------------------------------------"));
  
  char * axis_label;
  if (type == 0){
    Serial.println(F("time domain vector"));
    axis_label = " | t = ";
  }
  else{
    Serial.println(F("freq domain vector"));
    axis_label = " | f = ";
  }

  float axis_coord;

  // note: this logics is really the indication of the convention used for reporting the fft
  for (size_t ind=0; ind<data_len; ind++){
    if (type == 0){
      axis_coord = ind * dt_seconds;
    }
    else{
      if (ind <= data_len / 2){
        axis_coord = ind * df_hz / data_len;
      }
      else{
        axis_coord = (-(float)data_len + (float)ind) * df_hz / data_len;
      }
    }
    Serial.print(F("ind : "));
    snprintf(format_buff, format_buff_len, "%04i", ind);
    Serial.print(format_buff);
    Serial.print(axis_label);
    //snprintf(format_buff, format_buff_len, "%+05.2f", axis_coord);  // no snprintf for %f in arduino...
    dtostrf(axis_coord, 12, 4, format_buff);
    Serial.print(format_buff);
    Serial.print(F(" | f.r = "));
    dtostrf(data[ind].r, 12, 4, format_buff);
    Serial.print(format_buff);
    Serial.print(F(" | f.i = "));
    dtostrf(data[ind].r, 12, 4, format_buff);
    Serial.print(format_buff);
    Serial.println();
  }
  Serial.println(F("------------------------------------"));
}


/*

use:

kiss_fft_cfg cfg = kiss_fft_alloc( nfft ,is_inverse_fft ,0,0 );
    while ...
    
        ... // put kth sample in cx_in[k].r and cx_in[k].i
        
        kiss_fft( cfg , cx_in , cx_out );
        
        ... // transformed. DC is in cx_out[0].r and cx_out[0].i 

 */
