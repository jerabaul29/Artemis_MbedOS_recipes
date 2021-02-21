#include "Arduino.h"
#include "kiss_fft.h"

// on my Artemis board with core 1.2.1
#include "ard_supers/avr/dtostrf.h"

// time base properties
// the highest frequency for which get some information is the Nyquist frequency, i.e. df_hz / 2
// the frequency resolution is related to the length of the signal: freq_resolution = 2.0 * 1.0 / (data_len * dt_seconds) // TODO: check if a -1 somewhere
size_t data_len = 1024;
float dt_seconds = 0.1f;
float df_hz = 1.0f / dt_seconds;

// signal properties
// if the data duration is not a multiple of the period, energy will be distributed a bit around the peak
// for example: period 5.0f
// if the data duration is an integer multiple of the period, energy will be onlly at the mode (with uncertainty the rounding errors)
// for example: period 8.0f
// the spreading effect can be attenuated by using some windowing (eg hamming window).
float period_second = 4.0f;
float frequency_hz = 1.0f / period_second;
float amplitude = 2.0f;
float two_pi = 6.283185f;
float omega = two_pi * frequency_hz;

unsigned long millis_fft_start;

// our objects for taking fft: data in and out, config
kiss_fft_cfg my_cfg_fft;
kiss_fft_cfg my_cfg_ifft;
kiss_fft_cpx * data_time_domain;
kiss_fft_cpx * data_freq_domain;

// a bit of tooling
void print_vect(kiss_fft_cpx * data, size_t data_len, byte type, bool flag_pure_csv=false);
constexpr size_t format_buff_len {16};
char format_buff[format_buff_len];
float hamming_coeff_amplitude_compensated(size_t crrt_ind, size_t total_fft_len);
float energy_content(kiss_fft_cpx data, size_t data_len);
void apply_fft_scaling_sqrtN(kiss_fft_cpx * data, size_t data_len, byte type);
void apply_hamming(kiss_fft_cpx * data, size_t data_len);

void setup(){
  Serial.begin(230400);
  delay(10);
  Serial.println();
  Serial.println(F("------------------------------------- booted -------------------------------------"));

  // --------------------------------------------------------------
  // a bit of preparation

  my_cfg_fft  = kiss_fft_alloc(data_len, 0, NULL, NULL);
  my_cfg_ifft = kiss_fft_alloc(data_len, 1, NULL, NULL);

  Serial.print(F("we are taking FFTs of size: ")); Serial.println(data_len);
  Serial.print(F("kiss_fft_alloc allocated a memory buffer with size: ")); Serial.println(memneeded_copy);
  Serial.print(F("the data buffers for data in and out have size: ")); Serial.println(data_len * sizeof(kiss_fft_cpx));

  bool use_hamming_window {false};
  bool print_as_pure_csv  {true};
  bool print_vectors      {true};

  // --------------------------------------------------------------
  // generate the initial signal
  
  data_time_domain = new kiss_fft_cpx[data_len];
  data_freq_domain = new kiss_fft_cpx[data_len];
  
  for (size_t ind=0; ind<data_len; ind++){
    data_time_domain[ind].r = amplitude * cos(omega * ind * dt_seconds);
    data_time_domain[ind].i = 0.0f;
  }

  // --------------------------------------------------------------
  // perform the FFT

  Serial.println(F("FFT example"));

  if (print_vectors){
    print_vect(data_time_domain, data_len, 0, print_as_pure_csv);
  }

  if (use_hamming_window){
    apply_hamming(data_time_domain, data_len);
  }
    
  millis_fft_start = millis();
  kiss_fft(my_cfg_fft, data_time_domain, data_freq_domain);
  Serial.print(F("FFT took ")); Serial.print(millis()-millis_fft_start); Serial.println(F(" ms"));

  apply_fft_scaling_sqrtN(data_freq_domain, data_len, 0);

  if (print_vectors){
    print_vect(data_freq_domain, data_len, 1, print_as_pure_csv);
  }

  // --------------------------------------------------------------
  // erase the time domain data

  Serial.println(F("erase the time domain to prepare checking FFT"));

  for (size_t ind=0; ind<data_len; ind++){
    data_time_domain[ind].r = 0.0f;
    data_time_domain[ind].i = 0.0f;
  }

  // --------------------------------------------------------------
  // perform the IFFT

  Serial.println(F("IFFT example"));

  if (print_vectors){
    print_vect(data_freq_domain, data_len, 1, print_as_pure_csv);
  }

  millis_fft_start = millis();
  kiss_fft(my_cfg_ifft, data_freq_domain, data_time_domain);
  Serial.print(F("FFT took ")); Serial.print(millis()-millis_fft_start); Serial.println(F(" ms"));

  apply_fft_scaling_sqrtN(data_time_domain, data_len, 1);

  // scale on the way back
  for (size_t ind=0; ind<data_len; ind++){
    data_time_domain[ind].r /= (float)data_len;
    data_time_domain[ind].i /= (float)data_len;
  }

  if (print_vectors){
    print_vect(data_time_domain, data_len, 0, print_as_pure_csv);
  }

  Serial.println(F("done"));
}

void loop(){
  
}

// out tooling

// type: 0 for time domain, 1 for frequency domain
void print_vect(kiss_fft_cpx * data, size_t data_len, byte type, bool flag_pure_csv){
  Serial.println(F("------------------------------------"));
  
  __FlashStringHelper const * axis_label;
  __FlashStringHelper const * axis_label_header;
  
  if (type == 0){
    Serial.println(F("time domain vector"));
    axis_label = F(" | t = ");
    axis_label_header = F("time, ");
  }
  else{
    Serial.println(F("freq domain vector"));
    axis_label = F(" | f = ");
    axis_label_header = F("freq, ");
  }

  Serial.print(F("ind, ")); Serial.print(axis_label_header); Serial.println(F("real, imaginary"));

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
    if (!flag_pure_csv){
        Serial.print(F("ind : "));
    }
    snprintf(format_buff, format_buff_len, "%04i", ind);
    Serial.print(format_buff);
    if (!flag_pure_csv){
      Serial.print(axis_label);
    }
    else{
      Serial.print(F(", "));
    }
    //snprintf(format_buff, format_buff_len, "%+05.2f", axis_coord);  // no snprintf for %f in arduino...
    dtostrf(axis_coord, 12, 4, format_buff);
    Serial.print(format_buff);
    if (!flag_pure_csv){
      Serial.print(F(" | f.r = "));
    }
    else{
      Serial.print(F(", "));
    }
    dtostrf(data[ind].r, 12, 4, format_buff);
    Serial.print(format_buff);
    if (!flag_pure_csv){
      Serial.print(F(" | f.i = "));
    }
    else{
      Serial.print(F(", "));
    }
    dtostrf(data[ind].i, 12, 4, format_buff);
    Serial.print(format_buff);
    Serial.println();
  }
  Serial.print(F("total energy content: ")); Serial.println(energy_content(data, data_len));
  Serial.println(F("------------------------------------"));
}

float hamming_coeff_amplitude_compensated(size_t crrt_ind, size_t total_fft_len){
  // implementing the hamming window as described in: https://numpy.org/doc/stable/reference/generated/numpy.hamming.html
  // using the amplitude compensation coefficient as described in: https://www.physik.uzh.ch/local/teaching/SPI301/LV-2015-Help/lvanlsconcepts.chm/Scaling_Smoothing_Windows.html, https://community.sw.siemens.com/s/article/window-correction-factors
  // windowing is a bit tricky: it changes the amplitude / energy content, and one CANNOT preserve both amplitude and total enery while applying some non trivial windowing,
  // i.e. only the Uniform window, which is equivalent to no window, has the same amplitude and energy correction factors

  // TODO: add a flag to choose which to conserve
  
  // float amplitude_scaling_coefficient = 1.85f// that is the correction so that amplitude at the peak is conserved, i.e. 1.0f / 0.54f
  float amplitude_scaling_coefficient = 1.59f;  // that is the correction so that the total energy is conserved
  
  float phase_coefficient = two_pi * (float)(crrt_ind) / (float)(total_fft_len - 1);
  float amplitude_compensated_hamming_coef = (0.54f - 0.46f * cos(phase_coefficient)) * amplitude_scaling_coefficient;

  return amplitude_compensated_hamming_coef;
}

// TODO: fixme: do the hamming computation inside the function to avoid re calculating scaling and casts all the time
void apply_hamming(kiss_fft_cpx * data, size_t data_len){
  float crrt_hamming_coeff;
  for (size_t crrt_ind=0; crrt_ind<data_len; crrt_ind++){
    crrt_hamming_coeff = hamming_coeff_amplitude_compensated(crrt_ind, data_len);
    data[crrt_ind].r *= crrt_hamming_coeff;
    data[crrt_ind].i *= crrt_hamming_coeff;
  }
}

float energy_content(kiss_fft_cpx * data, size_t data_len){
  float total_energy = 0;

  for (size_t ind=0; ind<data_len; ind++){
    total_energy += data[ind].r * data[ind].r + data[ind].i * data[ind].i;
  }

  return total_energy;
}


// apply scaling so that each FFT scales with sqrt(N), instead of N for forward FFT, and 1/N for backwards FFT which is the current convention.
// that makes energy directly conserved in Parsevals theorem instead of having a factor in difference
void apply_fft_scaling_sqrtN(kiss_fft_cpx * data, size_t data_len, byte type){
  if (type == 1){
    float scaling_factor = sqrt((float)(data_len));
    for (size_t ind=0; ind<data_len; ind++){
      data[ind].r *= scaling_factor;
      data[ind].i *= scaling_factor;
    }
  }
  else if (type == 0){
    float scaling_factor = sqrt(1.0f / (float)(data_len));
    for (size_t ind=0; ind<data_len; ind++){
      data[ind].r *= scaling_factor;
      data[ind].i *= scaling_factor;
    }
  }
}

// TODO: apply_fft_scaling_sqrtN, energy_content, etc, should directly take a my_cfg_ifft
// TODO: hamming should directly take a my_cfg_ifft
// TODO: clear unused functions
// TODO: is kiss_fft_cpx aware of its size? If yes, drop the size_t data_len arguments and use the struct value
// TODO: plot the FFTs with / without windowing, just to check

// NOTE: this code allocates; could also provide a version with static memory allocation at the start

// TODO: show how to perform all of this statically
