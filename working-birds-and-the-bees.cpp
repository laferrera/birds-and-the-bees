#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  hw;
static Oscillator osc;
const int AUDIO_BLOCK_SIZE = 48;
float knob = 0.5f;

static void AudioCallback(AudioHandle::InterleavingInputBuffer in, 
            AudioHandle::InterleavingOutputBuffer out, 
            size_t size) {
float sig;

for (size_t i = 0; i < size; i += 2) {
  knob = hw.adc.GetFloat(0);
  osc.SetFreq(knob * 220.0f + 220.0f);

  sig = osc.Process();

  // left out
  out[i] = sig;

  // right out
  out[i + 1] = sig;
  }
}

int main(void) {
  // initialize seed hardware and oscillator daisysp module
  float sample_rate;
  hw.Configure();
  hw.Init();
//   hw.SetAudioBlockSize(AUDIO_BLOCK_SIZE);
  sample_rate = hw.AudioSampleRate();

  AdcChannelConfig adcConfig;
  adcConfig.InitSingle(hw.GetPin(15));
  hw.adc.Init(&adcConfig, 1);
  hw.adc.Start();

//   AdcChannelConfig adc[4];
//   adc[0].InitSingle(hw.GetPin(15));
//   adc[1].InitSingle(hw.GetPin(16));
//   adc[2].InitSingle(hw.GetPin(17));
//   adc[3].InitSingle(hw.GetPin(18));
//   hw.adc.Init(adc, 4); // my DaisySeed instance is called hw
//   hw.adc.Start();

  // Set parameters for oscillator
  osc.Init(sample_rate);
  osc.SetWaveform(osc.WAVE_SIN);
  osc.SetFreq(250);
  osc.SetAmp(0.5);

  // start callback
  
  hw.StartAudio(AudioCallback);

  while (1) {
    knob = hw.adc.GetFloat(0);
    hw.DelayMs(100);
  }
}
