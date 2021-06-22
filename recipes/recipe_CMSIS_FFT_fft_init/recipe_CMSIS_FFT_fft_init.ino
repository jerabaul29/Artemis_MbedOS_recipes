#include "Arduino.h"
#include "arm_math.h"

#include "ard_supers/avr/dtostrf.h"

// FFT settings
// admissible rfft lengths as well as some documentation are available at
// https://www.keil.com/pack/doc/CMSIS/DSP/html/group__RealFFT.html#gac5fceb172551e7c11eb4d0e17ef15aa3
// fft number of samples, i.e. length
constexpr int SAMPLES  = 32; 
constexpr uint8_t forward_fft = 0;
constexpr uint8_t backward_fft = 1;

// parameters for generating the test signal
constexpr float sample_rate {SAMPLES};
constexpr float signal_frequency {8.0};
constexpr float pi {3.141592653589793};
constexpr float amplitude {1.0};
constexpr float phase { 0.125 * 2.0 * pi};
constexpr float offset {0.5f};

constexpr float nyquist_frequency {sample_rate / 2.0};
constexpr float frequency_resolution {sample_rate / SAMPLES};

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

// getting our status back
arm_status crrt_arm_status;

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
  Serial.print(F("input signal has | amplitude ")); Serial.print(amplitude); Serial.print(F(" | frequency ")); Serial.print(signal_frequency); Serial.print(F(" | phase [rad] ")); Serial.print(phase); Serial.print(F(" ie [deg] ")); Serial.print(phase * 180.0f / pi); Serial.print(F(" | offset ")); Serial.print(offset); Serial.println(F(" | duration is 1.0s"));
  Serial.print(F("FFT has Nyquist frequency ")); Serial.print(nyquist_frequency); Serial.print(F(" and frequency resolution ")); Serial.println(frequency_resolution);
  Serial.println();

  // build the input signal
  for (int i=0; i<SAMPLES; i++){
    fft_input[i] = amplitude * cos(2.0 * pi * signal_frequency * i * 1.0 / sample_rate + phase) + offset;
  }

  Serial.println(F("input signal"));
  for (int i=0; i<SAMPLES; i++){
    Serial.print(F("ind ")); serial_print_int_width_4(i);
    Serial.print(F(" time: ")); serial_print_float_width_16_prec_8(i * 1.0 / sample_rate);
    Serial.print(F(" : ")); serial_print_float_width_16_prec_8(fft_input[i]); Serial.println();
  }
  Serial.println(F("done writing input"));
  Serial.println();

  // compute a FFT
  Serial.println(F("start init fast struct"));
  crrt_arm_status = arm_rfft_fast_init_f32(&crrt_arm_rfft_fast_instance_f32, SAMPLES);                          // get ready; this is generic, but may use quite a bit of flash memory as pulling all fft twiddle factors
  // crrt_arm_status = arm_rfft_32_fast_init_f32(&crrt_arm_rfft_fast_instance_f32);                          // get ready; this is less generic, specific to the size we use, but more flash memory efficient
  Serial.print(F("done init fast struct, status: ")); Serial.println(crrt_arm_status);
  Serial.println(F("start take FFT"));
  arm_rfft_fast_f32(&crrt_arm_rfft_fast_instance_f32, fft_input, fft_output, forward_fft);    // take the FFT; no status here!
  Serial.print(F("done take FFT")); Serial.println();
  Serial.println();

  Serial.println(F("output signal"));
  // FFT(f)[i] = conj[ FFT(f)[-i] ]
  // FFT(f)[]
  Serial.print(F("ind 0000 is packing FFT[0], the constant component, which is real constant component: ")); serial_print_float_width_16_prec_8(fft_output[0]); Serial.println();
  Serial.print(F("ind 0001 is packing FFT[N/2], the nyquist frequency content,  which is real constant component: ")); serial_print_float_width_16_prec_8(fft_output[1]); Serial.println();
  for (int i=2; i<SAMPLES; i++){
    int frq_factor = (i) / 2;
    Serial.print(F("ind ")); serial_print_int_width_4(i); Serial.print(F(" | frq [Hz]: ")); serial_print_float_width_16_prec_8(frq_factor * frequency_resolution);
    Serial.print(F(" | ")); serial_print_float_width_16_prec_8(fft_output[i]); Serial.println();
  }
  Serial.println(F("done writing output"));
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
}

/*

booted
illustrate rfft using CMSIS
input signal has | amplitude 1.00 | frequency 8.00 | phase [rad] 0.79 ie [deg] 45.00 | offset 0.50 | duration is 1.0s
FFT has Nyquist frequency 16.00 and frequency resolution 1.00

input signal
ind 0000 time:       0.00000000 :       1.20710671
ind 0001 time:       0.03125000 :      -0.20710683
ind 0002 time:       0.06250000 :      -0.20710671
ind 0003 time:       0.09375000 :       1.20710695
ind 0004 time:       0.12500000 :       1.20710659
ind 0005 time:       0.15625000 :      -0.20710695
ind 0006 time:       0.18750000 :      -0.20710658
ind 0007 time:       0.21875000 :       1.20710707
ind 0008 time:       0.25000000 :       1.20710647
ind 0009 time:       0.28125000 :      -0.20710708
ind 0010 time:       0.31250000 :      -0.20710646
ind 0011 time:       0.34375000 :       1.20710719
ind 0012 time:       0.37500000 :       1.20710635
ind 0013 time:       0.40625000 :      -0.20710720
ind 0014 time:       0.43750000 :      -0.20710634
ind 0015 time:       0.46875000 :       1.20710731
ind 0016 time:       0.50000000 :       1.20710623
ind 0017 time:       0.53125000 :      -0.20710732
ind 0018 time:       0.56250000 :      -0.20710620
ind 0019 time:       0.59375000 :       1.20710742
ind 0020 time:       0.62500000 :       1.20710611
ind 0021 time:       0.65625000 :      -0.20710744
ind 0022 time:       0.68750000 :      -0.20710608
ind 0023 time:       0.71875000 :       1.20710754
ind 0024 time:       0.75000000 :       1.20710599
ind 0025 time:       0.78125000 :      -0.20710757
ind 0026 time:       0.81250000 :      -0.20710596
ind 0027 time:       0.84375000 :       1.20710766
ind 0028 time:       0.87500000 :       1.20710588
ind 0029 time:       0.90625000 :      -0.20710769
ind 0030 time:       0.93750000 :      -0.20710585
ind 0031 time:       0.96875000 :       1.20710778
done writing input

start init fast struct
done init fast struct, status: 0
start take FFT
done take FFT

output signal
ind 0000 is packing FFT[0], the constant component, which is real constant component:      16.00000000
ind 0001 is packing FFT[N/2], the nyquist frequency content,  which is real constant component:       0.00000095
ind 0002 | frq [Hz]:       1.00000000 |       0.00000096
ind 0003 | frq [Hz]:       1.00000000 |       0.00000010
ind 0004 | frq [Hz]:       2.00000000 |       0.00000100
ind 0005 | frq [Hz]:       2.00000000 |       0.00000014
ind 0006 | frq [Hz]:       3.00000000 |       0.00000105
ind 0007 | frq [Hz]:       3.00000000 |       0.00000034
ind 0008 | frq [Hz]:       4.00000000 |       0.00000115
ind 0009 | frq [Hz]:       4.00000000 |       0.00000048
ind 0010 | frq [Hz]:       5.00000000 |       0.00000135
ind 0011 | frq [Hz]:       5.00000000 |       0.00000073
ind 0012 | frq [Hz]:       6.00000000 |       0.00000182
ind 0013 | frq [Hz]:       6.00000000 |       0.00000114
ind 0014 | frq [Hz]:       7.00000000 |       0.00000297
ind 0015 | frq [Hz]:       7.00000000 |       0.00000243
ind 0016 | frq [Hz]:       8.00000000 |      11.31370068
ind 0017 | frq [Hz]:       8.00000000 |      11.31371689
ind 0018 | frq [Hz]:       9.00000000 |      -0.00000199
ind 0019 | frq [Hz]:       9.00000000 |      -0.00000245
ind 0020 | frq [Hz]:      10.00000000 |      -0.00000083
ind 0021 | frq [Hz]:      10.00000000 |      -0.00000120
ind 0022 | frq [Hz]:      11.00000000 |      -0.00000039
ind 0023 | frq [Hz]:      11.00000000 |      -0.00000075
ind 0024 | frq [Hz]:      12.00000000 |      -0.00000020
ind 0025 | frq [Hz]:      12.00000000 |      -0.00000048
ind 0026 | frq [Hz]:      13.00000000 |      -0.00000011
ind 0027 | frq [Hz]:      13.00000000 |      -0.00000036
ind 0028 | frq [Hz]:      14.00000000 |      -0.00000009
ind 0029 | frq [Hz]:      14.00000000 |      -0.00000030
ind 0030 | frq [Hz]:      15.00000000 |      -0.00000003
ind 0031 | frq [Hz]:      15.00000000 |      -0.00000018
done writing output

 */
