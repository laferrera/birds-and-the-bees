#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

int PIN_MUX_SEL_0 = 17;
int PIN_MUX_SEL_1 = 18;
int PIN_MUX_SEL_2 = 19;

int PIN_ADC_POT_MUX1 = 15;
int PIN_ADC_POT_MUX2 = 16;
AnalogControl knob_[24];

static DaisySeed hw;
static Oscillator osc;
const int AUDIO_BLOCK_SIZE = 48;
float knob = 0.5f;

static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t size) {
  float sig;

  for (size_t i = 0; i < size; i += 2) {
    // knob = 0;
    knob = hw.adc.GetMuxFloat(0,1);
    float knob2 = hw.adc.GetMuxFloat(1, 1);
    osc.SetFreq(knob * 220.0f + 220.0f);
    // osc.SetFreq(((knob) + (knob2)) * 220.0f + 220.0f);

    sig = osc.Process();

    // left out
    out[i] = sig;

    // right out
    out[i + 1] = sig;
  }
}

int main(void) {
  // initialize hw hardware and oscillator daisysp module
  float sample_rate;
  hw.Configure();
  hw.Init();
  //   hw.SetAudioBlockSize(AUDIO_BLOCK_SIZE);
  sample_rate = hw.AudioSampleRate();

//   AdcChannelConfig adcConfig;
//   adcConfig.InitSingle(hw.GetPin(15));
//   hw.adc.Init(&adcConfig, 1);
//   hw.adc.Start();

  //   AdcChannelConfig adc[4];
  //   adc[0].InitSingle(hw.GetPin(15));
  //   adc[1].InitSingle(hw.GetPin(16));
  //   adc[2].InitSingle(hw.GetPin(17));
  //   adc[3].InitSingle(hw.GetPin(18));
  //   hw.adc.Init(adc, 4); // my Daisyhw instance is called hw
  //   hw.adc.Start();

  // let's try to mux
  AdcChannelConfig adc_cfg[2];
//   adc_cfg[0].InitSingle(hw.GetPin(15));
    adc_cfg[0].InitMux(hw.GetPin(PIN_ADC_POT_MUX1),
                     8,
                     hw.GetPin(PIN_MUX_SEL_0),
                     hw.GetPin(PIN_MUX_SEL_1),
                     hw.GetPin(PIN_MUX_SEL_2));
    adc_cfg[1].InitMux(hw.GetPin(PIN_ADC_POT_MUX2),
                       8,
                       hw.GetPin(PIN_MUX_SEL_0),
                       hw.GetPin(PIN_MUX_SEL_1),
                       hw.GetPin(PIN_MUX_SEL_2));
  // adc_cfg[2].InitMux(hw.GetPin(PIN_ADC_POT_MUX3),
  //                    8,
  //                    hw.GetPin(PIN_MUX_SEL_0),
  //                    hw.GetPin(PIN_MUX_SEL_1),
  //                    hw.GetPin(PIN_MUX_SEL_2));

  hw.adc.Init(adc_cfg, 1);
  hw.adc.Start();
  hw.StartLog(false);

  //   // Order of pots on the hardware connected to mux.
  //   //   for(size_t i = 0; i < 24; i+=3)
    for (size_t i = 0; i < 16; i += 3) {
      knob_[i].Init(hw.adc.GetMuxPtr(0, i), hw.AudioCallbackRate());
      knob_[i + 1].Init(hw.adc.GetMuxPtr(1, i + 1), hw.AudioCallbackRate());
      // knob_[i + 2].Init(hw.adc.GetMuxPtr(2, i + 2), AudioCallbackRate());
    }

  // Set parameters for oscillator
  osc.Init(sample_rate);
  osc.SetWaveform(osc.WAVE_SIN);
  osc.SetFreq(210);
  osc.SetAmp(0.5);

  // start callback

  hw.StartAudio(AudioCallback);

  while (1) {
    knob = hw.adc.GetFloat(0);
    hw.DelayMs(100);
  }
}
