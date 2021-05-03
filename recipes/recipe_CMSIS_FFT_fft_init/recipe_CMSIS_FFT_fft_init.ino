#include "Arduino.h"
#include "arm_math.h"

#include "ard_supers/avr/dtostrf.h"

// FFT settings
// admissible rfft lengths as well as some documentation are available at
// https://www.keil.com/pack/doc/CMSIS/DSP/html/group__RealFFT.html#gac5fceb172551e7c11eb4d0e17ef15aa3
// fft number of samples, i.e. length
constexpr int SAMPLES  = 64; 
constexpr uint8_t forward_fft = 0;
constexpr uint8_t backward_fft = 1;

// parameters for generating the test signal
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

/*
EXAMPLE OF OUTPUT

booted
illustrate rfft using CMSIS

input signal
ind 0000 :      -0.00000004
ind 0001 :      -0.38268349
ind 0002 :      -0.70710683
ind 0003 :      -0.92387956
ind 0004 :      -1.00000000
ind 0005 :      -0.92387950
ind 0006 :      -0.70710671
ind 0007 :      -0.38268331
ind 0008 :       0.00000013
ind 0009 :       0.38268358
ind 0010 :       0.70710689
ind 0011 :       0.92387962
ind 0012 :       1.00000000
ind 0013 :       0.92387944
ind 0014 :       0.70710665
ind 0015 :       0.38268325
ind 0016 :      -0.00000022
ind 0017 :      -0.38268363
ind 0018 :      -0.70710695
ind 0019 :      -0.92387962
ind 0020 :      -1.00000000
ind 0021 :      -0.92387944
ind 0022 :      -0.70710659
ind 0023 :      -0.38268316
ind 0024 :       0.00000031
ind 0025 :       0.38268372
ind 0026 :       0.70710701
ind 0027 :       0.92387968
ind 0028 :       1.00000000
ind 0029 :       0.92387938
ind 0030 :       0.70710653
ind 0031 :       0.38268307
ind 0032 :      -0.00000039
ind 0033 :      -0.38268381
ind 0034 :      -0.70710707
ind 0035 :      -0.92387968
ind 0036 :      -1.00000000
ind 0037 :      -0.92387938
ind 0038 :      -0.70710647
ind 0039 :      -0.38268301
ind 0040 :       0.00000048
ind 0041 :       0.38268387
ind 0042 :       0.70710713
ind 0043 :       0.92387974
ind 0044 :       1.00000000
ind 0045 :       0.92387933
ind 0046 :       0.70710641
ind 0047 :       0.38268292
ind 0048 :      -0.00000057
ind 0049 :      -0.38268396
ind 0050 :      -0.70710719
ind 0051 :      -0.92387974
ind 0052 :      -1.00000000
ind 0053 :      -0.92387927
ind 0054 :      -0.70710635
ind 0055 :      -0.38268283
ind 0056 :       0.00000066
ind 0057 :       0.38268405
ind 0058 :       0.70710725
ind 0059 :       0.92387980
ind 0060 :       1.00000000
ind 0061 :       0.92387927
ind 0062 :       0.70710629
ind 0063 :       0.38268277
done writing input

start init fast struct
done init fast struct
start take FFT
done take FFT

output signal
ind 0000 :       0.00000118
ind 0001 :      -0.00000001
ind 0002 :       0.00000044
ind 0003 :       0.00000051
ind 0004 :       0.00000029
ind 0005 :       0.00000123
ind 0006 :       0.00000028
ind 0007 :       0.00000299
ind 0008 :      -0.00001240
ind 0009 :      31.99999809
ind 0010 :       0.00000037
ind 0011 :      -0.00000388
ind 0012 :       0.00000038
ind 0013 :      -0.00000207
ind 0014 :       0.00000043
ind 0015 :      -0.00000136
ind 0016 :       0.00000010
ind 0017 :      -0.00000147
ind 0018 :       0.00000033
ind 0019 :      -0.00000092
ind 0020 :       0.00000032
ind 0021 :      -0.00000082
ind 0022 :       0.00000042
ind 0023 :      -0.00000062
ind 0024 :      -0.00000002
ind 0025 :      -0.00000158
ind 0026 :       0.00000036
ind 0027 :      -0.00000055
ind 0028 :       0.00000029
ind 0029 :      -0.00000053
ind 0030 :       0.00000034
ind 0031 :      -0.00000047
ind 0032 :       0.00000011
ind 0033 :      -0.00000012
ind 0034 :       0.00000040
ind 0035 :      -0.00000037
ind 0036 :       0.00000041
ind 0037 :      -0.00000034
ind 0038 :       0.00000039
ind 0039 :      -0.00000022
ind 0040 :       0.00000050
ind 0041 :       0.00000033
ind 0042 :       0.00000032
ind 0043 :      -0.00000022
ind 0044 :       0.00000038
ind 0045 :      -0.00000027
ind 0046 :       0.00000038
ind 0047 :      -0.00000018
ind 0048 :       0.00000060
ind 0049 :      -0.00000004
ind 0050 :       0.00000027
ind 0051 :      -0.00000002
ind 0052 :       0.00000032
ind 0053 :      -0.00000013
ind 0054 :       0.00000034
ind 0055 :      -0.00000008
ind 0056 :       0.00000095
ind 0057 :       0.00000072
ind 0058 :       0.00000042
ind 0059 :      -0.00000008
ind 0060 :       0.00000041
ind 0061 :       0.00000002
ind 0062 :       0.00000024
ind 0063 :       0.00000001
done writing output

 */
