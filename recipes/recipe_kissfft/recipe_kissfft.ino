#include "Arduino.h"
#include "kiss_fft.h"

void setup(){
  Serial.begin(115200);
  delay(10);
  Serial.println(F("booted"));

  size_t data_len = 64;
  kiss_fft_cfg my_cfg = kiss_fft_alloc(data_len, 0, NULL, NULL);

  kiss_fft_cpx * data_in  = new kiss_fft_cpx[data_len];
  kiss_fft_cpx * data_out = new kiss_fft_cpx[data_len];
  

  // put kth sample in cx_in[k].r and cx_in[k].i
  // transformed. DC is in cx_out[0].r and cx_out[0].i 
  kiss_fft(my_cfg, data_in, data_out);
}

void loop(){
  
}


/*

use:

kiss_fft_cfg cfg = kiss_fft_alloc( nfft ,is_inverse_fft ,0,0 );
    while ...
    
        ... // put kth sample in cx_in[k].r and cx_in[k].i
        
        kiss_fft( cfg , cx_in , cx_out );
        
        ... // transformed. DC is in cx_out[0].r and cx_out[0].i 

 */
