#include "Arduino.h"
#include "kiss_fft.h"

// time base properties
size_t data_len = 64;
float dt_seconds = 0.5f;
float df_hz = 1.0f / dt_seconds;

// signal properties
float period_second = 12.0f;
float frequency_hz = 1.0f / period_second;
float amplitude = 2.0f;
float two_pi = 6.283185f;
float omega = two_pi * frequency_hz;

// our objects for taking fft: data in and out, config
kiss_fft_cfg my_cfg;
kiss_fft_cpx * data_time_domain;
kiss_fft_cpx * data_freq_domain;

bool is_inverse_fft = false;

// a bit of tooling
void print_vect(kiss_fft_cpx * data, size_t data_len, byte type);

void setup(){
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println(F("------ booted ------"));

  // --------------------------------------------------------------
  // generate the signal
  
  data_time_domain  = new kiss_fft_cpx[data_len];
  data_freq_domain = new kiss_fft_cpx[data_len];
  
  for (size_t ind=0; ind<data_len; ind++){
    data_time_domain[ind].r = amplitude * sin(omega * ind * dt_seconds);
    data_time_domain[ind].i = 0.0f;
  }

  // --------------------------------------------------------------
  // perform the fft
  
  is_inverse_fft = 0;
  my_cfg = kiss_fft_alloc(data_len, is_inverse_fft, NULL, NULL);
  
  kiss_fft(my_cfg, data_time_domain, data_freq_domain);

  print_vect(data_time_domain, data_len, 0);
  print_vect(data_freq_domain, data_len, 1);

  // --------------------------------------------------------------
  // erase the real data

  for (size_t ind=0; ind<data_len; ind++){
    data_time_domain[ind].r = 0.0f;
    data_time_domain[ind].i = 0.0f;
  }

  // --------------------------------------------------------------
  // perform the ifft
  is_inverse_fft = 1;
  my_cfg = kiss_fft_alloc(data_len, is_inverse_fft, NULL, NULL);

  kiss_fft(my_cfg, data_freq_domain, data_time_domain);

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
  for (size_t ind=0; ind<data_len; ind++){
    Serial.print(F("ind : ")); Serial.print(ind); Serial.print(axis_label); Serial.print(ind * dt_seconds); Serial.print(F(" | f.r = ")); Serial.print(data_time_domain[ind].r); Serial.print(F(" | f.i = ")); Serial.print(data_time_domain[ind].i); Serial.println();
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
