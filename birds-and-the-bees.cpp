#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

#define NUM_OF_OSCILLATORS 8
#define AUDIO_BLOCK_SIZE 48

int PIN_MUX_SEL_0 = 17;
int PIN_MUX_SEL_1 = 18;
int PIN_MUX_SEL_2 = 19;

int PIN_ADC_POT_MUX1 = 15;
int PIN_ADC_POT_MUX2 = 16;
AnalogControl knob_[24];

static DaisySeed hw;
static VariableShapeOscillator osc[8];
static MoogLadder filterLeft;
static MoogLadder filterRight;
static Adsr envelope;

float outputVolume = 0.75f;
float portamento = 0.0f;
float swarmPreset = 0.0f; // what is this...
float swarmParcialRange = 0.0f; // what is that...
float cutoffHz = 100.0f;
float resonance = 0.0f;
float filterSwarmSpread = 0.5; // what is dis...
float enveloptToCutoff = 0.0f;
float overdrive = 0.0f;
float tune = 0.0f; // in cents?
float attackMs = 0.0f;
float decayMs = 0.0f;
float sustain = 0.0f; 
float releaseMs = 0.0f;
int octave = 2;
float stereoSpread = 0.5f;

float pitchRibbon = 0.0f;
float cutoffRibbon = 0.0f;



float knob = 0.5f;

void SetupOsc(float samplerate) {
  for (int i = 0; i < NUM_OF_OSCILLATORS; i++) {
    osc[i].Init(samplerate);
    // osc[i].SetAmp(0.125f);
  }
}

static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t size) {
  float sigL;
  float sigR;

  for (size_t i = 0; i < size; i += 2) {
    // knob = 0;
    knob = hw.adc.GetMuxFloat(0,1);
    float knob2 = hw.adc.GetMuxFloat(1, 1);
    for (int i = 0; i < NUM_OF_OSCILLATORS; i++) {
      osc[i].SetFreq(knob * 220.0f + 220.0f);

      // 0% means 0.125 * sig for both sides
      // 50% meant 0.1875 sig primary,  0.0625 sig secondary | 0.125f
      // 100% means 0.25 * sig for primary side, 0% for secondary
      float primaryChannelRatio = (stereoSpread * 0.125f + 0.125f);
      float secondaryChannelRatio = (0.125f - (stereoSpread * 0.125f));

      if (i % 2 == 0) {
        sigL += primaryChannelRatio * osc[i].Process();
        sigR += secondaryChannelRatio * osc[i].Process();
      }
      else {
        sigL += secondaryChannelRatio * osc[i].Process();
        sigR += primaryChannelRatio * osc[i].Process();
      }
    }

      // left out
      out[i] = sigL;

      // right out
      out[i + 1] = sigR;
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
    for (size_t i = 0; i < 16; i += 2) {
      knob_[i].Init(hw.adc.GetMuxPtr(0, i), hw.AudioCallbackRate());
      knob_[i + 1].Init(hw.adc.GetMuxPtr(1, i + 1), hw.AudioCallbackRate());
      // knob_[i + 2].Init(hw.adc.GetMuxPtr(2, i + 2), AudioCallbackRate());
    }

  // Set parameters for oscillator
  SetupOsc(sample_rate);

  // start callback

  hw.StartAudio(AudioCallback);

  while (1) {
    knob = hw.adc.GetFloat(0);
    hw.DelayMs(100);
  }
}
