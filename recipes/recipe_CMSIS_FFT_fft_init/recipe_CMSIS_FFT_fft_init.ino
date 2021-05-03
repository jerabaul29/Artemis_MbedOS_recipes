#include "Arduino.h"
#include "arm_math.h"

#include "ard_supers/avr/dtostrf.h"

// FFT settings
// admissible rfft lengths are available at
// https://www.keil.com/pack/doc/CMSIS/DSP/html/group__RealFFT.html#gac5fceb172551e7c11eb4d0e17ef15aa3
constexpr int SAMPLES  = 64; 
constexpr uint8_t forward_fft = 0;
constexpr uint8_t backward_fft = 1;

// parameters for the test signal
constexpr float sample_rate {SAMPLES};
constexpr float signal_frequency {4.0};
constexpr float pi {3.141592653589793};
constexpr float amplitude {1.0};

// Global variables for taking FFT: input and output
// for RFFT, there are a bit of "hacks", see the discussion at: https://github.com/ARM-software/CMSIS_5/issues/1091
// input: an array of samples, all real, nothing special, N samples [r_sample[0], r_samples[1], ...]
float32_t fft_input[SAMPLES];
// output: there is a bit of a hack here; the FT of a real signal is conjugate symmetric, i.e. Hermitian, i.e. FFT(i) = conj(FFT(N-i)), so it is enough
// to just output the coefficients from and included to 0 to N/2; in addition, FFT(0) and FFT(N/2) are real, not complex; the CMSIS RFFT takes advantage
// of that, so that the output is: [FFT(0), FFT(N/2), real(FFT(1)), img(FFT(1)), real(FFT(2)), img(FFT(2)), ...]
float32_t fft_output[SAMPLES];

// this can either be filled in the code (as is done here), or use a precomputed table (need arm_const_structs.h).
arm_rfft_fast_instance_f32 crrt_arm_rfft_fast_instance_f32;

// a bit of helper for printing stuff in a nicer format
constexpr size_t length_format_buffer {64};
char format_buffer[length_format_buffer];

void serial_print_int_width_4(unsigned int input){
  snprintf(format_buffer, length_format_buffer, "%04i", input);
  Serial.print(format_buffer);
}

void serial_print_float_width_16_prec_8(float input){ 
  dtostrf(input, 16, 8, format_buffer);
  Serial.print(format_buffer);
}

void setup() {
  Serial.begin(1000000);
  delay(100);
  
  Serial.println();
  Serial.println(F("booted"));
  Serial.println(F("illustrate rfft using CMSIS"));
  Serial.println();

  // build the input signal
  for (int i=0; i<SAMPLES; i++){
    fft_input[i] = amplitude * cos(2.0 * pi * signal_frequency * i / sample_rate + pi / 2.0);
  }

  Serial.println(F("input signal"));
  for (int i=0; i<SAMPLES; i++){
    Serial.print(F("ind ")); serial_print_int_width_4(i); Serial.print(F(" : ")); serial_print_float_width_16_prec_8(fft_input[i]); Serial.println();
  }
  Serial.println(F("done writing input"));
  Serial.println();

  // compute a FFT
  Serial.println(F("start init fast struct"));
  arm_rfft_fast_init_f32(&crrt_arm_rfft_fast_instance_f32, SAMPLES);                          // get ready
  Serial.println(F("done init fast struct"));
  Serial.println(F("start take FFT"));
  arm_rfft_fast_f32(&crrt_arm_rfft_fast_instance_f32, fft_input, fft_output, forward_fft);    // take the FFT
  Serial.println(F("done take FFT"));
  Serial.println();

  Serial.println(F("output signal"));
  for (int i=0; i<SAMPLES; i++){
    Serial.print(F("ind ")); serial_print_int_width_4(i); Serial.print(F(" : ")); serial_print_float_width_16_prec_8(fft_output[i]); Serial.println();
  }
  Serial.println(F("done writing output"));
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
}
