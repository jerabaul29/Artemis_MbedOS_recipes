// with inspiration from https://os.mbed.com/users/martinsimpson/code/CMSIS_FFT_mbed_os_DAC//file/05e2c9ca68e2/main.cpp/
// note: may be worth considering using this library instead: https://github.com/kosme/arduinoFFT

#define ARM_MATH_CM4  // The apollo3 is a cortex M4

#include "mbed.h"
#include "arm_math.h"

/* FFT settings */
constexpr int SAMPLES  = 512;           /* 256 real party and 256 imaginary parts */
constexpr int FFT_SIZE = SAMPLES / 2;   /* FFT size is always the same size as we have samples, so 256 in our case */
constexpr uint8_t forward_fft = 0;
constexpr uint8_t backward_fft = 0;

// Global variables for taking FFT
float32_t fft_input[SAMPLES];
float32_t fft_output[FFT_SIZE];

void setup() {

  // compute a FFT
  arm_rfft_fast_instance_f32 crrt_arm_rfft_fast_instance_f32;
  arm_rfft_fast_init_f32(&crrt_arm_rfft_fast_instance_f32, FFT_SIZE);                         // get ready
  arm_rfft_fast_f32(&crrt_arm_rfft_fast_instance_f32, fft_input, fft_output, forward_fft);    // take the FFT
}

void loop() {
  // put your main code here, to run repeatedly:

}
