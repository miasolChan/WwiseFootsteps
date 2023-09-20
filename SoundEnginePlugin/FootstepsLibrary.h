/*
* NEMISINDO LIBRARY
*
* This file is part of the project "Procedural Audio Worlds", a collaboration between
* Nemisindo Ltd. and Epic Games Ltd., which is released under Proprietary License. All Rights Reserved.
*
* See https://nemisindo.com/ for more information.
*
* Project Manager:
* Joshua Reiss
*
* Authors:
* Selim Sheta
* Jack Walters
*
* The Nemisindo Library is a collection of classes to facilitate procedural sound generation in UE4 using a
* sample-by-sample approach. There are three main categories of classes in this library: Oscillators, Processors,
* and Envelopes. These classes all have Setter functions to alter the class parameters, as well as a
* "runtime" function that should be called once every tick to generate and process audio correctly:
* Generators : NextSample()
* Processors : ProcessSample(float sample)
* Envelopes : GetNextEnvelopePoint()
* The timer class's runtime function is CheckTime().
*
* Except for the Panner processor, all classes assume a monophonic output. For stereo sound generation, two separate
* audio streams are required within the sound generation loop. For most cases, it's fine to have one audio
* stream connected to both Left and Right channels, since Unreal has a built-in audio spacialization feature.
*/

#pragma once
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <stdexcept>

namespace nemlib
{
    const double NEM_PI = 3.14159265358979323846264338327950288;
    /*### GENERATORS ###*/

    /* Sine wave oscillator */
    class SineOsc
    {
    public:
        // Default Constructor
        SineOsc();
        // Constructor
        SineOsc(float InFrequency, int InSampleRate);
        // Destructor
        virtual ~SineOsc() {}

        // Function setting the Frequency property of the oscillator
        void SetFrequency(float InFrequency);
        // Function Calculating the value of the next sample
        float NextSample();

    protected:
        // Explicit
        float Frequency = 440.0f;
        int SampleRate = 48000;
        float Phase = 0.0f;
        float PhaseInc = 440.0f / 48000.0f;
    };

    /* Square wave oscillator */
    class SquareOsc
    {
    public:
        // Default Constructor
        SquareOsc();
        // Constructor
        SquareOsc(float InFrequency, int InSampleRate);
        // Destructor
        virtual ~SquareOsc() {}

        // Function setting the Frequency property of the oscillator
        void SetFrequency(float InFrequency);
        // Function Calculating the value of the next sample
        float NextSample();

    protected:
        // Explicit
        float Frequency = 440.0f;
        int SampleRate = 48000;
        float Phase = 0.0f;
        float PhaseInc = 440.0f / 48000.0f;
    };

    /* Sawtooth wave oscillator */
    class SawOsc
    {
    public:
        // Default Constructor
        SawOsc();
        // Constructor
        SawOsc(float InFrequency, int InSampleRate);
        // Destructor
        virtual ~SawOsc() {}

        // Function setting the Frequency property of the oscillator
        void SetFrequency(float InFrequency);
        // Function Calculating the value of the next sample
        float NextSample();

    protected:
        // Explicit
        float Frequency = 440.0f;
        int SampleRate = 48000;
        float Phase = 0.0f;
        float PhaseInc = 440.0f / 48000.0f;
    };

    /* Triangle wave oscillator */
    class TriangleOsc
    {
    public:
        // Default Constructor
        TriangleOsc();
        // Constructor
        TriangleOsc(float InFrequency, int InSampleRate);
        // Destructor
        virtual ~TriangleOsc() {}

        // Function setting the Frequency property of the oscillator
        void SetFrequency(float InFrequency);
        // Function Calculating the value of the next sample
        float NextSample();

    protected:
        // Explicit
        float Frequency = 440.0f;
        int SampleRate = 48000;
        float Phase = 0.0f;
        float PhaseInc = 440.0f / 48000.0f;
    };

    /* PWM Signal generator.
    Basic pulse width modulation, with parameters for changing the frequency and the duty cycle.*/
    class PWMOsc
    {
    public:
        // Default Constructor
        PWMOsc();
        // Constructor
        PWMOsc(float InFrequency, int InSampleRate, float InDutyCycle);
        // Destructor
        virtual ~PWMOsc() {}

        // Function setting the Frequency property of the oscillator
        void SetFrequency(float InFrequency);

        // Function setting the Duty Cycle of the oscillator
        void SetDutyCycle(float InDutyCycle);

        // Function Calculating the value of the next sample
        float NextSample();

    protected:
        float Frequency = 440.0f;
        int SampleRate = 48000;
        float Phase = 0.0f;
        float PhaseInc = 440.0f / 48000.0f;
        float DutyCycle = 0.5f;
    };

    /* White noise generator
    Generates White noise by creating random samples using srand. It has no parameters.*/
    class WhiteNoiseGen
    {
    public:
        // Default Constructor
        WhiteNoiseGen();
        // Constructor
        WhiteNoiseGen(int InSampleRate);
        // Destructor
        virtual ~WhiteNoiseGen() {}

        // Function Calculating the value of the next sample
        float NextSample();

    protected:
        int SampleRate = 48000;
    };

    /* Pink noise generator
    Generates Pink noise by filtering white noise. It has no parameters.
    Based on Paul Kellet's method : https://www.firstpr.com.au/dsp/pink-noise/#Filtering */
    class PinkNoiseGen
    {
    public:
        // Default Constructor
        PinkNoiseGen();
        // Constructor
        PinkNoiseGen(int InSampleRate);
        // Destructor
        virtual ~PinkNoiseGen() {}

        // Function Calculating the value of the next sample
        float NextSample();

    protected:
        int SampleRate = 48000;
        // Filter coefficients
        float B0 = 0.0f;
        float B1 = 0.0f;
        float B2 = 0.0f;
        float B3 = 0.0f;
        float B4 = 0.0f;
        float B5 = 0.0f;
        float B6 = 0.0f;
    };

    /* Phasor Generator
    C++ implementation of the "phasor-generator" worklet, not to be confused with a phasor effect.
    Similar to a Sawtooth wave, but bound to [0,1] and with parameters for the Duty Cycle and
    initial Phase. */
    class PhasorGen
    {
    public:
        // Default Constructor
        PhasorGen();
        // Constructor
        PhasorGen(int InSampleRate, float InFrequency);
        // Constructor
        PhasorGen(int InSampleRate, float InFrequency, float InPhase, float InDuty);
        // Destructor
        virtual ~PhasorGen() {}

        // Function setting the Frequency property of the oscillator
        void SetFrequency(float InFrequency);
        // Set Phase
        void SetPhase(float InPhase);
        // Set Duty Cycle
        void SetDuty(float InDuty);
        // Function Calculating the value of the next sample
        float NextSample();

    protected:
        // Explicit
        float Frequency = 440.0f;
        int SampleRate = 48000;
        float Phase = 0.0f;
        float PhaseInc = 440.0f / 48000.0f;
        float Duty = 1.0f;
    };

    /* Random Ramp Generator
    Used in the "stream" model. Essentially a continuous series of linear ramps with random targets.
    The target changes every x ms (where x is the InInterval parameter) to a random value between -1 and 1.*/
    class RandRampsGen
    {
    public:
        // Default Constructor
        RandRampsGen();
        // Constructor
        RandRampsGen(int InSampleRate, float InInterval);
        // Destructor
        virtual ~RandRampsGen() {}

        // Sets the interval time in ms
        void SetInterval(float InInterval);
        // Returns the value of the next sample
        float NextSample();

    protected:
        int SampleRate = 48000;
        float CurrentTarget = 0.0f;
        float StartingValue = 0.0f;
        float LastOut = 0.0f;
        float Phase = 0.0f;
        float PhaseInc = 1.0f / 48000.0f;
        float Interval = 0.0f;
    };

    /*### ENVELOPES ###*/

    //  The envelopes are essentially a gain class but the gain varies automatically through time. 
    //  Here is a visualisation: 
    // https://drive.google.com/file/d/1NV7QBamaxX99HYEBdlvmNRadfgtbGh-G/view?usp=sharing

    /* Basic Envelope
    Produces a continuously changing value between 0.0 and 1.0 in a linear ADSR pattern. */
    class LinEnvelope
    {
    public:
        // Default Constructor
        LinEnvelope();
        // Constructor
        LinEnvelope(int InSampleRate, float InAttack, float InHold, float InDecay, float InSustain, float InRelease);
        LinEnvelope(int InSampleRate, float InAttack, float InHold, float InDecay, float InSustain, float InRelease, float InMin, float InMax);
        // Destructor
        virtual ~LinEnvelope() {}

        void SetAttack(float InAttack); // Sets the attack time in s
        void SetHold(float InHold); //Sets the hold time in s
        void SetDecay(float InDecay); // Sets the decay time in s
        void SetSustain(float InSustain); // Sets the sustain value
        void SetRelease(float InRelease); // Sets the release time in s
        void SetMax(float InMax);
        void SetMin(float InMin);

        float GetNextEnvelopePoint(); // Get the next value of the envelope

        void ResetEnvelope(); // Restart the enveloppe

    protected:
        float Attack = 0.05f;
        float Hold = 1.0f;
        float Decay = 1.0f;
        float Sustain = 0.2f;
        float Release = 0.5f;
        float EnvPos = 0.0f;
        float EnvPosInc = 0.0f; // <- Increase the envelope position by this much
        float Min = 0.0f;
        float Max = 1.0f;
        bool HasStarted = false;
    };

    /* ASR Envelope
    Produces a continuously changing value between 0.0 and 1.0 in a linear ASR pattern. */
    class LinASREnvelope
    {
    public:
        // Default Constructor
        LinASREnvelope();
        // Constructor
        LinASREnvelope(int InSampleRate, float InAttack, float InSustain, float InRelease);
        LinASREnvelope(int InSampleRate, float InAttack, float InSustain, float InRelease, float InAttackLVL, float InSustainLVL,
            float InStartLVL, float InEndLVL);
        // Destructor
        virtual ~LinASREnvelope() {}

        void SetAttack(float InAttack); // Sets the attack time in s
        void SetSustain(float InSustain); // Sets the sustain time in s
        void SetRelease(float InRelease); // Sets the release time in s
        void SetAttackLVL(float InAttackLVL); // Sets the Attack level
        void SetSustainLVL(float InSustainLVL); // Sets the Sustain level
        void SetStartLVL(float InStartLVL); // Sets the Start level
        void SetEndLVL(float InEndLVL); // Sets the End level

        float GetNextEnvelopePoint(); // Get the next value of the envelope

        void ResetEnvelope(); // Restart the enveloppe

    protected:
        float AttackTime = 0.5f;
        float SustainTime = 0.2f;
        float ReleaseTime = 0.5f;
        float AttackLVL = 1.0f;
        float SustainLVL = 0.2f;
        float StartLVL = 0.0f;
        float EndLVL = 0.0f;
        float EnvPos = 0.0f;
        float EnvPosInc = 0.0f; // <- Increase the envelope position by this much
        bool HasStarted = false;
    };

    /* Exponential Envelope
    Produces a continuously changing value between 0.0 and 1.0 in an exponential ADSR pattern.
    Equivalent to "exponentialADRenvelope" in JS. */
    class ExpEnvelope
    {
    public:
        // Default Constructor
        ExpEnvelope();
        // Constructor
        ExpEnvelope(int InSampleRate, float InAttack, float InHold, float InDecay, float InSustain, float InRelease);
        ExpEnvelope(int InSampleRate, float InAttack, float InHold, float InDecay, float InSustain, float InRelease, float InMin, float InMax);
        // Destructor
        virtual ~ExpEnvelope() {}

        void SetAttack(float InAttack); // Sets the attack time in s
        void SetHold(float InHold); //Sets the hold time in s
        void SetDecay(float InDecay); // Sets the decay time in s
        void SetSustain(float InSustain); // Sets the sustain value
        void SetRelease(float InRelease); // Sets the release time in s
        void SetMin(float InMin);
        void SetMax(float InMax);

        float GetNextEnvelopePoint(); // Get the next value of the envelope
        void ResetEnvelope(); // Restart the enveloppe

    protected:
        float Attack = 0.05f;
        float Hold = 1.0f;
        float Decay = 1.0f;
        float Sustain = 0.2f;
        float Release = 0.5f;
        float PreviousValue = 0.0f;
        float AttackTimeConst = 0.0f;
        float DecayTimeConst = 0.0f;
        float ReleaseTimeConst = 0.0f;
        float Min = 0.0f;
        float Max = 1.0f;
        float EnvPosInc = 1.0f / 48000.0f;
        float EnvPos = 0.0f;
        int SampleRate = 48000;
        bool HasStarted = false;
    };

    /* Exponential Envelope
    Produces a continuously changing value between 0.0 and 1.0 in an "exponential-like" ADSR pattern.
    Instead of using exp it is based on powers, and instead of tending infinitely towards a value,
    it actually reaches it. Equivalent to "exponentialADRenvelope2" in JS. */
    class ExpEnvelope2
    {
    public:
        // Default Constructor
        ExpEnvelope2();
        // Constructor
        ExpEnvelope2(int InSampleRate, float InAttack, float InHold, float InDecay, float InSustain, float InRelease);
        ExpEnvelope2(int InSampleRate, float InAttack, float InHold, float InDecay, float InSustain, float InRelease, float InMin, float InMax);
        // Destructor
        virtual ~ExpEnvelope2() {}

        void SetAttack(float InAttack); // Sets the attack time in s
        void SetHold(float InHold); //Sets the hold time in s
        void SetDecay(float InDecay); // Sets the decay time in s
        void SetSustain(float InSustain); // Sets the sustain value
        void SetRelease(float InRelease); // Sets the release time in s
        void SetMin(float InMin);
        void SetMax(float InMax);

        float GetNextEnvelopePoint(); // Get the next value of the envelope
        void ResetEnvelope(); // Restart the enveloppe

    protected:
        float Attack = 0.05f;
        float Hold = 1.0f;
        float Decay = 1.0f;
        float Sustain = 0.2f;
        float Release = 0.5f;
        float Min = 0.0f;
        float Max = 1.0f;
        int SampleRate = 48000;
        bool HasStarted = false;
        float EnvPos = 0.0f;
        float EnvPosInc = 1.0f / 48000.0f;
    };

    /* Exponential Target
    This class is analogous to the "setTargetAtTime" operation in Javascript. Unlike in JS, it is
    not asynchronous. This enveloppe will follow an exponential curve from a default value towards
    a final value. */
    class ExpTarget {
    public:
        // Default Constructor
        ExpTarget();
        // Constructor
        ExpTarget(int InSampleRate, float InInitVal, float InFinalVal, float InTimeConst);
        // Destructor 
        virtual ~ExpTarget() {}

        void SetInitValue(float InInitVal);
        void SetFinalValue(float InFinalVal);
        void SetTimeConst(float InTimeConst);
        float GetNextEnvelopePoint();
        void ResetEnvelope();
    private:
        int SampleRate = 48000;
        float InitValue = 1.0f;
        float FinalValue = 0.0f;
        float TimeConst = 1.0f;
        float PreviousValue = 1.0f;
        bool HasStarted = false;
    };

    /* Linear Ramp
    A linear Attack-Hold envelope, similar to linearRampToValueAtTime in JS. */
    class LinRamp {
    public:
        LinRamp();
        LinRamp(int InSampleRate, float InAttack, float InInitVal, float InFinalVal);
        virtual ~LinRamp() {}

        void SetInitValue(float InInitVal);
        void SetFinalValue(float InFinalVal);
        void SetAttackTime(float InAttack);
        float GetNextEnvelopePoint();
        void ResetEnvelope();
    private:
        int SampleRate = 48000;
        float InitValue = 0.0f;
        float FinalValue = 1.0f;
        float AttackTime = 1.0f;
        float EnvPos = 0.0f;
        float EnvInc = 1 / 48000.0f;
        bool HasStarted = false;
    };

    /*### FILTER PROCESSORS ###*/

    // Associating filter types with integers
    enum : int {
        bq_type_lowpass = 0, // Low-Pass : 0
        bq_type_highpass, // High-Pass : 1
        bq_type_bandpass, // Band-Pass : 2
        bq_type_notch, // Notch filter : 3
        bq_type_peak, // Peak filter : 4
        bq_type_lowshelf, // Low-shelf : 5
        bq_type_highshelf, // High-shelf : 6
        bq_type_allpass // All Pass : 7
    };

    /* Biquad Filer
    This filter implementation is based on the WAA specifications.
    Important : For HP and LP, the Q factor must be specified in dB.*/
    class BiquadFilter
    {
    public:
        BiquadFilter();
        BiquadFilter(int InSampleRate, float InFrequency, float InQFactor, float InPeakGainDB, int InType);
        virtual ~BiquadFilter() {}

        void SetFrequency(float InFrequency);
        void SetQFactor(float InQFactor);
        void SetPeakGain(float InPeakGainDB);
        void SetType(int InType);
        float ProcessSample(float InSample);
        void ResetFilter();
    protected:
        int SampleRate = 48000;
        float V = 1.0f;
        float W = 1.5f;
        float Q = 1.0f;
        float AQ = 0.5f;
        float AQdB = 0.5f;
        float AS = 0.7f;
        int Type = 0;
        float Y1 = 0.0f; // y[n-1]
        float Y2 = 0.0f; // y[n-2]
        float X1 = 0.0f; // x[n-1]
        float X2 = 0.0f; // x[n-2]
        float B0 = 0.0f;
        float B1 = 0.0f;
        float B2 = 0.0f;
        float A0 = 0.0f;
        float A1 = 0.0f;
        float A2 = 0.0f;
        void ComputeCoeff();
    };

    /* Single pole LPF
    Single pole IIR Low pass filter, cut-off frequency corresponds to transition point (avg of max and
    min of |H(w)|^2), not -3dB point. Acts like an all-pass after f > fs/4. Important: if implementing
    a model that was made in Puredata, the cut-off frequency from the PD model must be converted. see the
    PureDataFreq() function. */
    class OnePoleLPF
    {
    public:
        // Default Constructor
        OnePoleLPF();
        // Constructor
        OnePoleLPF(int InSampleRate, float InFrequency);
        // Destructor
        virtual ~OnePoleLPF() {}

        void SetFrequency(float InFrequency); // Sets the cut-off frequency of the filter
        float ProcessSample(float InSample); // Processes one input sample

    protected:
        int SampleRate = 48000;
        float LastOut = 0.0f;
        float Coeff = 1.0f;
    };

    /* Single pole HPF
    Single pole IIR High pass filter, cut-off frequency corresponds to -3dB point.
    Acts like an all-cut after f > fs/2. Important: if implementing a model that was made in Puredata,
    the cut-off frequency from the PD model must be converted. see the PureDataFreq() function.*/
    class OnePoleHPF
    {
    public:
        // Default Constructor
        OnePoleHPF();
        // Constructor
        OnePoleHPF(int InSampleRate, float InFrequency);
        // Destructor
        virtual ~OnePoleHPF() {}

        void SetFrequency(float InFrequency); // Sets the cut-off frequency of the filter
        float ProcessSample(float InSample); // Processes one input sample

    protected:
        int SampleRate = 48000;
        float LastOut = 0.0f;
        float LastIn = 0.0f;
        float Coeff = 1.0f;
    };

    /* Low Order BPF
    In our JS implementation, this class is referred as "one-pole-BP-processor". Despite the name, it is
    in fact a two pole filter based on github.com/pure-data/pure-data/blob/master/src/d_filter.c
    Note that this implementation uses loose approximations for the filter coefficients, and affects
    low frequencies far more than high frequencies. Past Fs/2.75, it actually acts as a high-frequency boost.
    Using the biquad filter instead is advised. */
    class TwoPoleBPF
    {
    public:
        // Default Constructor
        TwoPoleBPF();
        // Constructor
        TwoPoleBPF(int InSampleRate, float InFrequency, float InQFactor);
        // Destructor
        virtual ~TwoPoleBPF() {}

        void SetFrequency(float InFrequency); // Sets the cut-off frequency of the filter
        void SetQFactor(float InQFactor); // Q-factor
        float ProcessSample(float InSample); // Processes one input sample

    protected:
        int SampleRate = 48000;
        float Frequency = 2000.0f;
        float Q = 0.707f;
        float Z1 = 0.0f;
        float Z2 = 0.0f;
        float Coeff1 = 0.0f;
        float Coeff2 = 0.0f;
        float CompGain = 0.0f;
        void ComputeCoeff(); // Calculates the filter coefficients based on the input parameters
    };

    /* High Order BandPass Filter */
    class HighOrderBPF
    {
    public:
        // Default Constructor
        HighOrderBPF();
        // Constructor
        HighOrderBPF(int InSampleRate, float InFrequency, float InQFactor);
        // Destructor
        virtual ~HighOrderBPF() {}

        void SetFrequency(float InFrequency); // Sets the cut-off frequency of the filter
        void SetQFactor(float InQFactor); // Q-factor
        float ProcessSample(float InSample); // Processes one input sample

    protected:
        int SampleRate = 48000;
        float Frequency = 2000.0f;
        float Q = 0.707f;
        // Previous outputs
        float Y1 = 0.0f;
        float Y2 = 0.0f;
        float Y3 = 0.0f;
        float Y4 = 0.0f;
        // Previous inputs
        float X1 = 0.0f;
        float X2 = 0.0f;
        float X3 = 0.0f;
        float X4 = 0.0f;
        // Coefficients
        float C0 = 0.0f;
        float C1 = 0.0f;
        float C2 = 0.0f;
        float C3 = 0.0f;
        float C4 = 0.0f;
        // Normalisation
        float CompGain = 0.0f;
        void ComputeCoeff(); // Calculates the filter coefficients based on the input parameters
        float Beta = 0.0f;
    };

    /* Signal Differential (Sigma Delta)
    Set the gain to the Sample rate to get the equivalent of our JS signal-differential worklet.
    Set it to 1 to get the delta-processor worklet. By default it is 1. */
    class SigmaDelta {
    public:
        // Default Constructor
        SigmaDelta();
        // Constructor
        SigmaDelta(float InGain);
        // Destructor
        virtual ~SigmaDelta() {}

        void SetGain(float InGain);
        float ProcessSample(float InSample);
    private:
        float Gain = 1.0f;
        float LastIn = 0.0f;
    };

    /*### DISTORTION PROCESSORS ###*/

    /* Clipping Processor */
    class ClipProcessor {
    public:
        // Default Constructor
        ClipProcessor();
        // Constructor
        ClipProcessor(float InLowThreshold, float InHighThreshold);
        // Destructor
        virtual ~ClipProcessor() {}

        void SetLowThresh(float InLowThreshold);
        void SetHighThresh(float InHighThresh);
        float ProcessSample(float InSample);
    private:
        float HiThresh = 1.0f;
        float LoThresh = -1.0f;
    };

    /* Overdrive Processor
    Important : specify Drive and Volume in dB. */
    class OverDriveProcessor {
    public:
        // Default Constructor
        OverDriveProcessor();
        // Constructor
        OverDriveProcessor(float InVolumedB, float InDrivedB, float InBias, float InKnee);
        // Destructor
        virtual ~OverDriveProcessor() {}

        void SetVolume(float InVolumedB);
        void SetDrive(float InDrivedB);
        void SetBias(float InBias);
        void SetKnee(float InKnee);
        float ProcessSample(float InSample);
    private:
        float Volume = 1.0f;
        float Drive = 0.0f;
        float Bias = 0.0f;
        float Knee = 0.001f;
        float C0 = 0.0f;
        float C1 = 0.0f;
        float C2 = 0.0f;
        float Alpha = 0.99f;
        float Prev = 0.0f;
        void ComputeCoeff();
    };

    /* Distortion Processor
    A sort of soft-clipping processor with only one parameter. High input values are
    "squashed" more than low input values, giving the function a sigmoid-like shape. */
    class DistortionProcessor {
    public:
        // Default Constructor
        DistortionProcessor();
        // Constructor
        DistortionProcessor(float InAmount);
        DistortionProcessor(float InAmount, float OutputGain);
        // Destructor
        virtual ~DistortionProcessor() {}

        void SetAmount(float InAmount);
        void SetGain(float InGain);
        float ProcessSample(float InSample);
    private:
        float Amount = 1.0f;
        float Gain = 1.0f / 3.0f;
    };

    /*### OTHER PROCESSORS ###*/

    /* Stereo Panning
    Whether the input is mono or stereo, the output of this class will be an std::vector
    in the form [LeftChSample, RightChSample]. Uses circular panning law. */
    class StereoPanner
    {
    public:
        StereoPanner();
        StereoPanner(float InPanParam);
        virtual ~StereoPanner() {}
        std::vector<float> ProcessSample(float InSample);
        std::vector<float> ProcessSample(float InLeftSample, float InRightSample);
        void SetPan(float InPanParam);
    protected:
        float pan = 0.0f;
    };

    /* Delay */
    class Delay
    {
    public:
        Delay();
        Delay(int InSampleRate, float InDelayTime);
        virtual ~Delay() {}
        void SetDelay(float InDelayTime);
        float ProcessSample(float InSample);
    private:
        int SampleRate;
        float DelayTime;
        int ReadPointer = 0;
        int WritePointer = 0;
        std::vector<float> DelayBuffer;
    };

    /* Feedback Delay */
    class FeedbackDelay
    {
    public:
        FeedbackDelay();
        FeedbackDelay(int InSampleRate, float InDelayTime, float InFeedbackGain, float InDryGain, float InWetGain);
        virtual ~FeedbackDelay() {}
        void SetDelay(float InDelayTime);
        void SetFeedback(float InFeedbackGain);
        void SetDryGain(float InDryGain);
        void SetWetGain(float InWetGain);
        float ProcessSample(float InSample);
    private:
        int SampleRate = 48000;
        float DelayTime = 0.5f;
        float Feedback = 0.5f;
        float Dry = 0.0f;
        float Wet = 1.0f;
        float Prev = 0.0f;
        nemlib::Delay Delay;
    };

    /* Haas Effect
    Uses the Haas effect to spread a mono signal in the stereo field.
    Whether the input is mono or stereo, the output of this class will be an std::vector
    in the form [LeftChSample, RightChSample]. Depth is the amount of delay in ms, and Separation controls
    how different the Left and Right channels are. */
    class HaasEffect
    {
    public:
        HaasEffect();
        HaasEffect(int InSampleRate, float InDepth, float InSeparation);
        virtual ~HaasEffect() {}
        std::vector<float> ProcessSample(float InSample);
        void SetDepth(float InDepth);
        void SetSeparation(float InSeparation);
    protected:
        float Separation = 0.5f;
        nemlib::Delay Wet;
    };

    /* RMS
    Calculates the RMS over the specifed window length */
    class RMS
    {
    public:
        RMS();
        RMS(int InSampleRate);
        RMS(int InSampleRate, int InWindowLength);
        virtual ~RMS() {}
        // Set the window size
        void SetWindowLength(int InWindowLength);
        // Performs RMS calculation
        float ProcessSample(float InSample);
    private:
        int SampleRate = 48000;
        int WindowLength = 256;
        int WindowCount = 0;
        float SquareSum = 0.0f;
        float Output = 0.0f;
    };

    /*### OTHERS ###*/

    /* Timer
    checkTime() Will output 'true' after the timer has reached the amount of time (seconds) specified.
    must be called once for every tick to function as intended. If possible, use this instead of a Delay
    to save computation cost. */
    class Timer
    {
    public:
        Timer();
        Timer(int InSampleRate, float InTime);
        virtual ~Timer() {}
        // Time in s that the timer must go to 
        void SetTime(float InTime);
        // Set the timer back to 0
        void ResetTimer();
        // Check if the timer has reached the desired time
        bool checkTime();
        // Pauses the timer, checkTime will output false
        void PauseTimer();
        // Starts the timer
        void ResumeTimer();
    private:
        int SampleRate = 48000;
        float Inc = 1 / 48000.0f;
        float Counter = 0.0f;
        float Time = 2.0f;
        bool Play = false;
    };

    /* A helper function from our JS implementation. Applies linear mapping to a value in the range [oldMin, oldMax]
    to fit in the range [newMin, newMax]. */
    float Rescale(float value, float newMin, float newMax, float oldMin, float oldMax);

    /* PD frequency conversion
    Used to convert a cutoff frequency specified for pure data to its equivalent value for our 1-pole LP and HP filters */
    float PureDataFreq(int InSampleRate, float frequency);

    /* Helper function for binding input parameter to a certain range of values
    clamps InParam to the range [InMix; InMax] */
    float Clamp(float InParam, float InMin, float InMax);
    int Clamp(int InParam, int InMin, int InMax);

    /*### Interactor Worklets ###*/

    /* Pulse Processor
    Used in the fire model, processes white noise to create a hissing sound */
    class PulseProcessor
    {
    public:
        // Default Constructor
        PulseProcessor();
        // Constructor
        PulseProcessor(int InSampleRate);
        // Destructor
        ~PulseProcessor() {}

        float ProcessSample(float InSample);
        float ProcessSample(float InSample, nemlib::BiquadFilter InFilter);

    protected:
        int SampleRate = 48000;
        int SampleNum = 0;
        int SampleCounter = 0;
        float decayS = 0.0f;
    };

    /* Vary
     Multiplies the input by a random number between 1 - InAmount and 1 + InAmount.
     The line srand(static_cast <unsigned> (time(0))); must be included at some point
     before calling Vary() to initialise the random number generator.*/
    float Vary(float InValue, float InAmount);

    /* Mode
    Structure of a mode for Filterbank */
    struct Mode {
        int nModes;
        std::vector<int> Types;
        std::vector<float> Freqs;
        std::vector<float> Qs;
        std::vector<float> Gains;
    };

    /* FilterBank
    Class containing multiple biquad filters  */
    class FilterBank {
    public:
        FilterBank();
        FilterBank(int InSampleRate, int InNumFilters);
        ~FilterBank() {}

        void InitialiseFilterBank(Mode InFilterInfo);
        void VaryParameters(Mode InFilterInfo);
        void ResetFilter();
        void Mute();
        void Unmute();
        void Unmute(float InGain);
        float ProcessSample(float InSample);
    private:
        float MuteGain;
        int SampleRate;
        std::vector<float> FilterBandGains;
        std::vector<BiquadFilter> Filters;
        float OutputMult;
    };

    /* CurveEnvelope
    Special Type of Envelope analogous to the Web Audio's setValueCurveAtTime() method */
    class CurveEnvelope
    {
    public:
        CurveEnvelope();
        CurveEnvelope(int InSampleRate, std::vector<float> InValues, float InTime);
        CurveEnvelope(int InSampleRate, std::vector<float> InValues, std::vector<float> InTimes);
        virtual ~CurveEnvelope() {}
        void SetValues(std::vector<float> InValues);
        void SetTime(float InTime);
        void SetTimes(std::vector<float> InTimes);
        float GetNextEnvelopePoint();
        void ResetEnvelope();

    protected:
        std::vector<float> Values;
        float Time;
        std::vector<float> Times;
        float EnvPos = 0.0f;
        float EnvPosInc = 0.0f;
        float Boundary;
        bool HasStarted = false;
        int NumOfValues;
        int Counter;
        int SampleRate;
    };
}

