#include "terrarium.h"
#include "daisy_petal.h"
#include "vibrato.h"
#include "scale.h"
#include "tapesaturator.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

const int AUDIO_BLOCK_SIZE = 48;

DaisyPetal hw;
Led led1, led2;


bool effectBypass;
float wetMixSetting;
float wetL;

float pickupTolerance = 0.25f;
float knob1,knob2, knob3, knob4, knob5, knob6; 


// Parameter vtime, vfreq, vsend, lfo_speed, amplitude;
// why does Reverb use params?

void conditionalParamUpdate(float *myCurKnobVal, float *myLastParamSetting, float *myCurParamSetting, bool *mySettingLocked)
{
    float knobDiff = abs(*myLastParamSetting - *myCurKnobVal);
    if (*mySettingLocked)
    {
        if (knobDiff < pickupTolerance)
        {
            *mySettingLocked = false;
            // paramUpdateLedEnvelope.Trigger();
        }
    }
    else
    {
        *myCurParamSetting = *myCurKnobVal;
        *myLastParamSetting = *myCurKnobVal;
    }
}


void Controls()
{
    hw.ProcessAllControls();

    knob1 = hw.knob[Terrarium::KNOB_1].Process();
    knob2 = hw.knob[Terrarium::KNOB_2].Process();
    knob3 = hw.knob[Terrarium::KNOB_3].Process();
    knob4 = hw.knob[Terrarium::KNOB_4].Process();
    knob5 = hw.knob[Terrarium::KNOB_5].Process();
    knob6 = hw.knob[Terrarium::KNOB_6].Process();

    wetMixSetting = knob1; //      1 is MIX no mater what
    // conditionalParamUpdate(&knob2, &wowDepth, &wowDepthLast, &wowDepthLock);                                // wowDepth = knob2;


    ////////////////////
    ///   Switches    //
    ////////////////////
    if (hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        effectBypass = !effectBypass;
        led1.Set(effectBypass ? 1.0f : 0.0f);
    }

    if (hw.switches[Terrarium::SWITCH_4].Pressed())
    {
        // compBypass = true;
    }


    // led2.Set(paramUpdateLedEnvelopeSig); //this noisy af
}


////////////////////
/// start block   //
////////////////////

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    Controls();

    ////////////////////////////////////
    ///   start sample manipulation   //
    ////////////////////////////////////
    for (size_t i = 0; i < size; i++)
    {


        //---------------------//
        //  render audio      //
        //---------------------//
        wetL = in[0][i];

        //-----------------------------//
        //    set sample to output     //
        //-----------------------------//
        if (!effectBypass)
        {
            out[0][i] = wetL;
        }
        else
        {
            out[0][i] = in[0][i];
        }
    }
}


int main(void)
{

    hw.Init();
    hw.SetAudioBlockSize(AUDIO_BLOCK_SIZE); // this was 10 for a while
    float sample_rate = hw.AudioSampleRate();



    led1.Init(hw.seed.GetPin(Terrarium::LED_1), !effectBypass);
    // led2.Init(hw.seed.GetPin(Terrarium::LED_2), !dustBypass);
    led1.Update();
    led2.Update();

    hw.StartAdc();
    hw.ProcessAllControls();
    hw.StartAudio(AudioCallback);
    while (1)
    {
        // hw.DelayMs(6);
        led1.Update();
        led2.Update();
    }
}
