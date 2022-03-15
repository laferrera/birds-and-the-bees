#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisysp;
using namespace daisy;

#define NUM_OF_OSCILLATORS 8
#define AUDIO_BLOCK_SIZE 48
float notes[] = {220.0f, 440.0f, 220.0f, 329.63f, 261.63f, 293.66f, 329.63f, 349.23f};
int notesIndex = 0;

int PIN_MUX_SEL_0 = 17;
int PIN_MUX_SEL_1 = 18;
int PIN_MUX_SEL_2 = 19;

int PIN_ADC_POT_MUX1 = 15;
int PIN_ADC_POT_MUX2 = 16;
AnalogControl knob_[24];

static DaisySeed hw;
static VariableShapeOscillator osc[NUM_OF_OSCILLATORS];
// static Oscillator osc[NUM_OF_OSCILLATORS];
static MoogLadder filterLeft;
static MoogLadder filterRight;
static Adsr envelope;
static Port port;

float outputVolume = 0.75f;
float portamento = 0.0f;
float swarmPreset = 0.0f;       // what is this...
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
Switch button1;

void SetupOsc(float samplerate) {
  for (int i = 0; i < NUM_OF_OSCILLATORS; i++) {
    // osc[i].Init(samplerate);
    // osc[i].SetWaveform(osc[i].WAVE_SIN);
    // osc[i].SetFreq(250);
    // osc[i].SetAmp(0.5);

    osc[i].Init(samplerate);
    osc[i].SetWaveshape(0.0f);
    osc[i].SetPW(0.5f);
    osc[i].SetSync(true);
  }
}

static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t size) {

  
  for (size_t i = 0; i < size; i += 2) {
    float sigL = 0;
    float sigR = 0;
    // knob = 0;
    knob = hw.adc.GetFloat(0);
    button1.Debounce();
    bool gateState = button1.Pressed();
    if(button1.RisingEdge()) {
      notesIndex = (notesIndex + 1) % 8;
    }
    float note = notes[notesIndex];
    note = port.Process(note);
    float waveshape = knob;
  
    for (int i = 0; i < NUM_OF_OSCILLATORS; i++) {
      float newFreq = note + (0.2f * i);
      osc[i].SetFreq(newFreq);
      osc[i].SetSyncFreq(newFreq);
      osc[i].SetWaveshape(waveshape);
      // 0% means 0.125 * sig for both sides
      // 50% meant 0.1875 sig primary,  0.0625 sig secondary | 0.125f
      // 100% means 0.25 * sig for primary side, 0% for secondary

      float primaryChannelRatio = (stereoSpread * 0.125f + 0.125f);
      float secondaryChannelRatio = (0.125f - (stereoSpread * 0.125f));
      float oscOutput = osc[i].Process();
      if (i % 2 == 0) {
        sigL += primaryChannelRatio * oscOutput;
        sigR += secondaryChannelRatio * oscOutput;
      } else {
        sigL += secondaryChannelRatio * oscOutput;
        sigR += primaryChannelRatio * oscOutput;
      }
    }
    float envAmp = envelope.Process(gateState);
    sigL *= envAmp * outputVolume;
    sigR *= envAmp * outputVolume;
    // left out
    out[i] = sigL;

    // right out
    out[i + 1] = sigR;
  }
}

void setupMuxAdc(void) {
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

  hw.adc.Init(adc_cfg, 2);
  hw.adc.Start();
  hw.StartLog(false);

  //   // Order of pots on the hardware connected to mux.
  //   //   for(size_t i = 0; i < 24; i+=3)
  for (size_t i = 0; i < 16; i += 2) {
    knob_[i].Init(hw.adc.GetMuxPtr(0, i), hw.AudioCallbackRate());
    knob_[i + 1].Init(hw.adc.GetMuxPtr(1, i + 1), hw.AudioCallbackRate());
    // knob_[i + 2].Init(hw.adc.GetMuxPtr(2, i + 2), AudioCallbackRate());
  }
}

void setupNormalAdc(void) {
  AdcChannelConfig adcConfig;
  adcConfig.InitSingle(hw.GetPin(15));
  hw.adc.Init(&adcConfig, 1);
  hw.adc.Start();
}

int main(void) {
  // initialize hw hardware and oscillator daisysp module
  float sample_rate;
  hw.Configure();
  hw.Init();
  hw.SetAudioBlockSize(AUDIO_BLOCK_SIZE);
  sample_rate = hw.AudioSampleRate();

  button1.Init(hw.GetPin(1), 1000); // i think this has to tbe a purple pinout
  setupNormalAdc();
  

  // AdcChannelConfig adcConfig;
  // adcConfig.InitSingle(hw.GetPin(15));
  // hw.adc.Init(&adcConfig, 1);
  // hw.adc.Start();

  //   AdcChannelConfig adc[4];
  //   adc[0].InitSingle(hw.GetPin(15));
  //   adc[1].InitSingle(hw.GetPin(16));
  //   adc[2].InitSingle(hw.GetPin(17));
  //   adc[3].InitSingle(hw.GetPin(18));
  //   hw.adc.Init(adc, 4); // my Daisyhw instance is called hw
  //   hw.adc.Start();

  // Set parameters for oscillator
  SetupOsc(sample_rate);
  envelope.Init(sample_rate, AUDIO_BLOCK_SIZE);
  envelope.SetAttackTime(0.5f);
  envelope.SetReleaseTime(10.5f);

  port.Init(sample_rate, 0.05f);


      // start callback
  hw.StartAudio(AudioCallback);

  while (1) {
    knob = hw.adc.GetFloat(0);
    hw.DelayMs(100);
  }
}
