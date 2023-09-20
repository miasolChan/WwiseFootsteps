/*
* Copyright(c) 2021 Nemisindo Ltd. All Rights Reserved.
*
* NEMISINDO LIBRARY
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
*
*/
#include "FootstepsLibrary.h"

namespace nemlib
{
    /*### UTILITIES ###*/

    // Function to map the range [-1.0;1.0] to ]0.0;900[
    float CustomMapping(float X) {
        return 0.3f * pow(3000.7f, X);
    }

    // A basic timer class. Preferable over using Delay to save on computing cost
    Timer::Timer() {}
    Timer::Timer(int InSampleRate, float InTime) {
        SampleRate = std::max(InSampleRate, 1);
        Inc = 1.0f / (float)SampleRate;
        Counter = 0.0f;
        Time = std::max(InTime, 0.0f);
        Play = false;
    }
    void Timer::SetTime(float InTime) { Time = std::max(InTime, 0.0f); }
    void Timer::ResetTimer() { Counter = 0.0f; }
    void Timer::PauseTimer() { Play = false; }
    void Timer::ResumeTimer() { Play = true; }
    bool Timer::checkTime() {
        bool Complete = false;
        if (Play == true) {
            if (Counter >= Time) {
                Complete = true;
            }
            else {
                Counter += Inc;
            }
        }
        return Complete;
    }

    // A helper function from our JS implementation.
    float Rescale(float value, float newMin, float newMax, float oldMin, float oldMax) {
        if (oldMax == oldMin) {
            oldMin -= 0.01f; // prevent division by zero.
        }
        if (value < oldMin) {
            value = oldMin;
        }
        else if (value > oldMax) {
            value = oldMax;
        }
        return newMin + (value - oldMin) * (newMax - newMin) / (oldMax - oldMin);
    }

    // PD frequency conversion
    float PureDataFreq(int InSampleRate, float frequency) {
        InSampleRate = std::max(InSampleRate, 1);
        float newFreq = ((float)InSampleRate / (2.0f * (float)NEM_PI)) * acos(2.0f * (1.0f - 2.0f * (float)NEM_PI * frequency / (float)InSampleRate)
            / (1.0f + pow(1.0f - 2.0f * (float)NEM_PI * frequency / (float)InSampleRate, 2.0f)));
        return newFreq;
    }

    // Clamping function for parameters
    float Clamp(float InParam, float InMin, float InMax) {
        return std::min(std::max(InParam, InMin), InMax);
    }
    int Clamp(int InParam, int InMin, int InMax) {
        return std::min(std::max(InParam, InMin), InMax);
    }

    /*### SINE WAVE CLASS ###*/

    // Sine Wave Class default constructor declaration
    SineOsc::SineOsc() {}
    // Sine Wave Class constructor declaration
    SineOsc::SineOsc(float InFrequency, int InSampleRate)
    {
        Frequency = std::max(InFrequency, 1.0f);
        SampleRate = std::max(InSampleRate, 1);
        PhaseInc = InFrequency / (float)SampleRate;
    }

    // Sets the value of the oscillator's frequency property
    void SineOsc::SetFrequency(float InFrequency)
    {
        Frequency = std::max(InFrequency, 1.0f);
        PhaseInc = Frequency / (float)SampleRate;
    }
    // Returns the value of the next sample
    float SineOsc::NextSample()
    {
        // fmod(x,y) returns the remainder of x/y. (Neat way of wrapping the phase.)
        Phase = (float)fmod(Phase, 1.0f);

        const float Radians = 2.0f * Phase * (float)NEM_PI - (float)NEM_PI;
        float Sample = sin(-1.0f * Radians);
        Phase += PhaseInc;

        return Sample;
    }

    /*### SQUARE WAVE CLASS ###*/

    // Square wave oscillator default constructor
    SquareOsc::SquareOsc() {}
    // Square wave oscillator constructor declaration
    SquareOsc::SquareOsc(float InFrequency, int InSampleRate)
    {
        if (InSampleRate > 0 && InFrequency > 0.0f)
        {
            Frequency = std::max(InFrequency, 1.0f);
            SampleRate = std::max(InSampleRate, 1);
            PhaseInc = InFrequency / (float)SampleRate;
        }
    }

    // Sets the value of the oscillator's frequency property
    void SquareOsc::SetFrequency(float InFrequency)
    {
        Frequency = std::max(InFrequency, 1.0f);
        PhaseInc = Frequency / (float)SampleRate;
    }
    // Returns the value of the next sample
    float SquareOsc::NextSample()
    {
        Phase = (float)fmod(Phase, 1.0f);

        float Sample = 0.0;
        // Calculating the sample
        if (Phase <= 0.5) {
            Sample = -1.0;
        }
        else {
            Sample = 1.0;
        }

        // Incrementing phase
        Phase += PhaseInc;
        return Sample;
    }

    /*### SAWTOOTH WAVE CLASS ###*/

   // Sawtooth wave oscillator default constructor
    SawOsc::SawOsc() {}
    // Sawtooth wave oscillator constructor declaration
    SawOsc::SawOsc(float InFrequency, int InSampleRate)
    {
        Frequency = std::max(InFrequency, 1.0f);
        SampleRate = std::max(InSampleRate, 1);
        PhaseInc = Frequency / (float)SampleRate;
    }

    // Sets the value of the oscillator's frequency property
    void SawOsc::SetFrequency(float InFrequency)
    {
        Frequency = std::max(InFrequency, 1.0f);
        PhaseInc = Frequency / (float)SampleRate;
    }
    // Returns the value of the next sample
    float SawOsc::NextSample()
    {
        Phase = (float)fmod(Phase, 1.0f);
        float Sample = 2.0f * Phase - 1.0f;
        // Incrementing phase
        Phase += PhaseInc;
        return Sample;
    }

    /*### TRIANGLE WAVE CLASS ###*/

   // Triangle wave oscillator default constructor
    TriangleOsc::TriangleOsc() {}
    // Triangle wave oscillator constructor declaration
    TriangleOsc::TriangleOsc(float InFrequency, int InSampleRate)
    {
        Frequency = std::max(InFrequency, 1.0f);
        SampleRate = std::max(InSampleRate, 1);
        PhaseInc = Frequency / (float)SampleRate;
    }

    // Sets the value of the oscillator's frequency property
    void TriangleOsc::SetFrequency(float InFrequency)
    {
        Frequency = std::max(InFrequency, 1.0f);
        PhaseInc = Frequency / (float)SampleRate;
    }
    // Returns the value of the next sample
    float TriangleOsc::NextSample()
    {
        Phase = (float)fmod(Phase, 1.0f);
        float Sample = 0.0f;
        if (Phase <= 0.5f) {
            Sample = 4.0f * Phase - 1.0f;
        }
        else {
            Sample = -4.0f * Phase + 3.0f;
        }

        // Incrementing phase
        Phase += PhaseInc;
        return Sample;
    }

    /*### PWM SIGNAL CLASS ###*/

   // Pulse Width Modulation default constructor
    PWMOsc::PWMOsc() {}
    // Pulse Width Modulation constructor declaration
    PWMOsc::PWMOsc(float InFrequency, int InSampleRate, float InDutyCycle)
    {
        Frequency = std::max(InFrequency, 1.0f);
        SampleRate = std::max(InSampleRate, 1);
        PhaseInc = Frequency / (float)SampleRate;
        DutyCycle = Clamp(InDutyCycle, 0.0f, 1.0f);
    }

    // Sets the value of the oscillator's frequency property
    void PWMOsc::SetFrequency(float InFrequency)
    {
        Frequency = std::max(InFrequency, 1.0f);
        PhaseInc = Frequency / (float)SampleRate;
    }
    // Sets the value of the oscillator's duty cycle
    void PWMOsc::SetDutyCycle(float InDutyCycle)
    {
        DutyCycle = Clamp(InDutyCycle, 0.0f, 1.0f);
    }
    // Returns the value of the next sample
    float PWMOsc::NextSample()
    {
        Phase = (float)fmod(Phase, 1.0f);

        float Sample = 0.0;
        // Calculating the sample
        if (Phase >= DutyCycle) {
            Sample = -1.0;
        }
        else {
            Sample = 1.0;
        }

        // Incrementing phase
        Phase += PhaseInc;
        return Sample;
    }

    /*### WHITE NOISE CLASS ###*/

   // White noise generator default constructor
    WhiteNoiseGen::WhiteNoiseGen() {
        srand(static_cast <unsigned> (time(0)));
    }
    // White noise generator constructor declaration
    WhiteNoiseGen::WhiteNoiseGen(int InSampleRate)
    {
        SampleRate = std::max(InSampleRate, 1);
    }

    // Returns the value of the next sample
    float WhiteNoiseGen::NextSample()
    {
        float Sample = ((float(rand()) / float(RAND_MAX)) * 2.0f) - 1.0f;
        return Sample;
    }

    /*### PINK NOISE CLASS ###*/

   // Pink noise generator default constructor
    PinkNoiseGen::PinkNoiseGen() {
        B0 = 0.0f;
        B1 = 0.0f;
        B2 = 0.0f;
        B3 = 0.0f;
        B4 = 0.0f;
        B5 = 0.0f;
        B6 = 0.0f;
        srand(static_cast <unsigned> (time(0)));
    }
    // Pink noise generator constructor declaration
    PinkNoiseGen::PinkNoiseGen(int InSampleRate)
    {
        SampleRate = std::max(InSampleRate, 1);
        B0 = 0.0f;
        B1 = 0.0f;
        B2 = 0.0f;
        B3 = 0.0f;
        B4 = 0.0f;
        B5 = 0.0f;
        B6 = 0.0f;
        srand(static_cast <unsigned> (time(0)));
    }

    // Returns the value of the next sample
    float PinkNoiseGen::NextSample()
    {
        float Sample = ((float(rand()) / float(RAND_MAX)) * 2.0f) - 1.0f;
        B0 = 0.99886f * B0 + Sample * 0.0555179f;
        B1 = 0.99332f * B1 + Sample * 0.0750759f;
        B2 = 0.96900f * B2 + Sample * 0.1538520f;
        B3 = 0.86650f * B3 + Sample * 0.3104856f;
        B4 = 0.55000f * B4 + Sample * 0.5329522f;
        B5 = -0.7616f * B5 - Sample * 0.0168980f;
        float Output = B0 + B1 + B2 + B3 + B4 + B5 + B6 + Sample * 0.5362f;
        B6 = Sample * 0.115926f;
        return 0.11f * Output;
    }

    /*### PHASOR GENERATOR CLASS ###*/

    // Default Constructor
    PhasorGen::PhasorGen() {}
    // Constructor
    PhasorGen::PhasorGen(int InSampleRate, float InFrequency) {
        Frequency = std::max(InFrequency, 1.0f);
        SampleRate = std::max(InSampleRate, 1);
        PhaseInc = Frequency / (float)SampleRate;
        Phase = 0.0f;
        Duty = 1.0f;
    }
    // Constructor
    PhasorGen::PhasorGen(int InSampleRate, float InFrequency, float InPhase, float InDuty) {
        Frequency = std::max(InFrequency, 1.0f);
        SampleRate = std::max(InSampleRate, 1);
        PhaseInc = Frequency / (float)SampleRate;
        Phase = InPhase;
        Duty = Clamp(InDuty, 0.001f, 1.0f);
    }

    void PhasorGen::SetFrequency(float InFrequency) {
        Frequency = std::max(InFrequency, 1.0f);
        PhaseInc = Frequency / (float)SampleRate;
    }
    void PhasorGen::SetPhase(float InPhase) {
        Phase = InPhase;
    }
    void PhasorGen::SetDuty(float InDuty) {
        Duty = Clamp(InDuty, 0.001f, 1.0f);
    }
    float PhasorGen::NextSample() {
        float Output = 0.0f;
        if (Phase > Duty) { Output = 0.0f; }
        else { Output = Phase / Duty; }
        Phase = (Phase + PhaseInc) - floor(Phase + PhaseInc);
        return Output;
    }

    /*### RANDOM RAMP GENERATOR ###*/

   // default constructor
    RandRampsGen::RandRampsGen() {
        srand(static_cast <unsigned> (time(0)));
    }
    // constructor
    RandRampsGen::RandRampsGen(int InSampleRate, float InInterval)
    {
        srand(static_cast <unsigned> (time(0)));
        SampleRate = std::max(InSampleRate, 1);
        PhaseInc = 1.0f / (float)SampleRate;
        Phase = 0.0f;
        Interval = InInterval / 1000.0f;
        CurrentTarget = 2.0f * (float(rand()) / float(RAND_MAX)) - 1.0f;
    }

    void RandRampsGen::SetInterval(float InInterval) {
        Interval = InInterval / 1000.0f;
    }
    // Returns the value of the next sample
    float RandRampsGen::NextSample()
    {
        if (Phase >= Interval) {
            CurrentTarget = 2.0f * (float(rand()) / float(RAND_MAX)) - 1.0f;
            Phase = 0.0f;
            StartingValue = LastOut;
        }
        LastOut = StartingValue + Phase * (CurrentTarget - StartingValue) / Interval;
        LastOut = nemlib::Clamp(LastOut, -1.0f, 1.0f);
        Phase += PhaseInc;
        return LastOut;
    }

    /*### BASIC ENVELOPE CLASS ###*/

    // Default constructor
    LinEnvelope::LinEnvelope() {}
    // Envelope Class constructor declaration
    LinEnvelope::LinEnvelope(int InSampleRate, float InAttack, float InHold, float InDecay, float InSustain, float InRelease) {
        Hold = std::max(InHold, 0.0f);
        EnvPosInc = 1.0f / (float)std::max(InSampleRate, 1);
        Sustain = Clamp(InSustain, 0.0f, 1.0f);
        Attack = std::max(InAttack, 0.0001f);
        Decay = std::max(InDecay, 0.0001f);
        Release = std::max(InRelease, 0.0001f);
        EnvPos = 0.0f;
        Min = 0.0f;
        Max = 1.0f;
        HasStarted = false;
    }
    LinEnvelope::LinEnvelope(int InSampleRate, float InAttack, float InHold, float InDecay, float InSustain, float InRelease, float InMin, float InMax) {
        Hold = std::max(InHold, 0.0f);
        EnvPosInc = 1.0f / (float)std::max(InSampleRate, 1);
        Min = InMin;
        Max = InMax;
        Sustain = Clamp(InSustain, InMin, InMax);
        Attack = std::max(InAttack, 0.0001f);
        Decay = std::max(InDecay, 0.0001f);
        Release = std::max(InRelease, 0.0001f);
        EnvPos = 0.0f;
        HasStarted = false;
    }

    // Set Attack time in s
    void LinEnvelope::SetAttack(float InAttack) {
        Attack = std::max(InAttack, 0.0001f);
    }
    // Set Hold time in s
    void LinEnvelope::SetHold(float InHold) {
        Hold = std::max(InHold, 0.0f);
    }
    // Set Decay time in s
    void LinEnvelope::SetDecay(float InDecay) {
        Decay = std::max(InDecay, 0.0001f);
    }
    // Set Sustain value
    void LinEnvelope::SetSustain(float InSustain) {
        Sustain = Clamp(InSustain, Min, Max);
    }
    // Set Release time in s
    void LinEnvelope::SetRelease(float InRelease) {
        Release = std::max(InRelease, 0.0001f);
    }
    // Set Max
    void LinEnvelope::SetMax(float InMax) {
        Max = InMax;
    }
    // Set Min
    void LinEnvelope::SetMin(float InMin) {
        Min = InMin;
    }
    // Reset the current position to the start of the envelope
    void LinEnvelope::ResetEnvelope() {
        EnvPos = 0.0f;
        HasStarted = true;
    }
    // Get the next position of the envelope
    float LinEnvelope::GetNextEnvelopePoint() {
        float EnvReturnValue = Min;

        if (HasStarted == true) {
            if (EnvPos <= Attack) {
                EnvReturnValue = Max * EnvPos / Attack;
            }
            else if (EnvPos <= Attack + Hold) {
                EnvReturnValue = Max;
            }
            else if (EnvPos <= Attack + Hold + Decay) {
                EnvReturnValue = Max - (Sustain - Max) * (Attack + Hold - EnvPos) / Decay;
            }
            else if (EnvPos <= Attack + Hold + Decay + Release) {
                EnvReturnValue = Sustain - (Min - Sustain) * (Attack + Hold + Decay + Release - EnvPos) / Release;
            }
            else {
                EnvReturnValue = Min;
            }
            if (EnvPos <= Attack + Hold + Decay + Release) {
                EnvPos += EnvPosInc;
            }
        }

        return EnvReturnValue;
    }

    /*### LINEAR ASR ENVELOPE CLASS ###*/

    // Default constructor
    LinASREnvelope::LinASREnvelope() {}
    // Enveloppe Class constructor declaration
    LinASREnvelope::LinASREnvelope(int InSampleRate, float InAttack, float InSustain, float InRelease) {
        EnvPosInc = 1.0f / (float)std::max(InSampleRate, 1);
        AttackTime = std::max(InAttack, 0.0001f);
        SustainTime = std::max(InSustain, 0.0001f);
        ReleaseTime = std::max(InRelease, 0.0001f);

        AttackLVL = 1.0f;
        SustainLVL = 0.2f;
        StartLVL = 0.0f;
        EndLVL = 0.0f;
        HasStarted = false;
        EnvPos = 0.0f;
    }
    LinASREnvelope::LinASREnvelope(int InSampleRate, float InAttack, float InSustain, float InRelease, float InAttackLVL,
        float InSustainLVL, float InStartLVL, float InEndLVL) {
        EnvPosInc = 1.0f / (float)std::max(InSampleRate, 1);
        AttackTime = std::max(InAttack, 0.0001f);
        SustainTime = std::max(InSustain, 0.0001f);
        ReleaseTime = std::max(InRelease, 0.0001f);

        AttackLVL = InAttackLVL;
        SustainLVL = InSustainLVL;
        StartLVL = InStartLVL;
        EndLVL = InEndLVL;
        HasStarted = false;
        EnvPos = 0.0f;
    }

    // Set Attack time in s
    void LinASREnvelope::SetAttack(float InAttack) {
        AttackTime = std::max(InAttack, 0.0001f);
    }
    // Set Sustain time in s
    void LinASREnvelope::SetSustain(float InSustain) {
        SustainTime = std::max(InSustain, 0.0001f);
    }
    // Set Release time in s
    void LinASREnvelope::SetRelease(float InRelease) {
        ReleaseTime = std::max(InRelease, 0.0001f);
    }
    void LinASREnvelope::SetAttackLVL(float InAttackLVL) {
        AttackLVL = std::max(InAttackLVL, 0.0001f);
    }
    void LinASREnvelope::SetSustainLVL(float InSustainLVL) {
        SustainLVL = std::max(InSustainLVL, 0.0001f);
    }
    void LinASREnvelope::SetStartLVL(float InStartLVL) {
        StartLVL = std::max(InStartLVL, 0.0001f);
    }
    void LinASREnvelope::SetEndLVL(float InEndLVL) {
        EndLVL = std::max(InEndLVL, 0.0001f);
    }
    // Reset the current position to the start of the envelope
    void LinASREnvelope::ResetEnvelope() {
        EnvPos = 0.0f;
        HasStarted = true;
    }
    // Get the next position of the envelope
    float LinASREnvelope::GetNextEnvelopePoint() {
        float EnvReturnValue = StartLVL;

        if (HasStarted == true) {
            if (EnvPos <= AttackTime) {
                EnvReturnValue = StartLVL + EnvPos * (AttackLVL - StartLVL) / AttackTime;
            }
            else if (EnvPos <= AttackTime + SustainTime) {
                EnvReturnValue = SustainLVL;
            }
            else if (EnvPos <= AttackTime + SustainTime + ReleaseTime) {
                EnvReturnValue = SustainLVL + (EnvPos - AttackTime - SustainTime) * (EndLVL - SustainLVL) / ReleaseTime;
            }
            else {
                EnvReturnValue = EndLVL;
            }
            if (EnvPos <= AttackTime + SustainTime + ReleaseTime) {
                EnvPos += EnvPosInc;
            }
        }

        return EnvReturnValue;
    }

    /*### EXPONENTIAL ENVELOPE CLASS ###*/

    // Default constructor
    ExpEnvelope::ExpEnvelope() {}
    // Enveloppe Class constructor declaration
    ExpEnvelope::ExpEnvelope(int InSampleRate, float InAttack, float InHold, float InDecay, float InSustain, float InRelease) {
        SampleRate = std::max(InSampleRate, 1);
        Attack = std::max(InAttack / 4.0f, 0.0001f);
        AttackTimeConst = exp(-1.0f / (Attack * (float)SampleRate));
        Hold = std::max(InHold, 0.0f);
        Decay = std::max(InDecay / 4.0f, 0.0001f);
        DecayTimeConst = exp(-1.0f / (Decay * (float)SampleRate));
        Sustain = Clamp(InSustain, 0.0f, 1.0f);
        Release = std::max(InRelease / 14.0f, 0.0001f);
        ReleaseTimeConst = exp(-1.0f / (Release * (float)SampleRate));
        EnvPosInc = 1.0f / (float)std::max(SampleRate, 1);
        EnvPos = 0.0f;
        PreviousValue = 0.0f;
        Min = 0.0f;
        Max = 1.0f;
        HasStarted = false;
    }
    ExpEnvelope::ExpEnvelope(int InSampleRate, float InAttack, float InHold, float InDecay, float InSustain, float InRelease, float InMin, float InMax) {
        SampleRate = std::max(InSampleRate, 1);
        Attack = std::max(InAttack / 4.0f, 0.0001f);
        AttackTimeConst = exp(-1.0f / (Attack * (float)SampleRate));
        Hold = std::max(InHold, 0.0f);
        Decay = std::max(InDecay / 4.0f, 0.0001f);
        DecayTimeConst = exp(-1.0f / (Decay * (float)SampleRate));
        Sustain = Clamp(InSustain, InMin, InMax);
        Release = std::max(InRelease / 14.0f, 0.0001f);
        ReleaseTimeConst = exp(-1.0f / (Release * (float)SampleRate));
        EnvPosInc = 1.0f / (float)std::max(SampleRate, 1);
        EnvPos = 0.0f;
        PreviousValue = Min;
        Min = InMin;
        Max = InMax;
        HasStarted = false;
    }

    // Set Attack time in s
    void ExpEnvelope::SetAttack(float InAttack) {
        Attack = std::max(InAttack / 4.0f, 0.0001f);
        AttackTimeConst = exp(-1.0f / (Attack * (float)SampleRate));
    }
    // Set Hold time in s
    void ExpEnvelope::SetHold(float InHold) {
        Hold = std::max(InHold, 0.0f);
    }
    // Set Decay time in s
    void ExpEnvelope::SetDecay(float InDecay) {
        Decay = std::max(InDecay / 4.0f, 0.0001f);
        DecayTimeConst = exp(-1.0f / (Decay * (float)SampleRate));
    }
    // Set Sustain value
    void ExpEnvelope::SetSustain(float InSustain) {
        Sustain = Clamp(InSustain, Min, Max);
    }
    // Set Release time in s
    void ExpEnvelope::SetRelease(float InRelease) {
        Release = std::max(InRelease / 14.0f, 0.0001f);
        ReleaseTimeConst = exp(-1.0f / (Release * (float)SampleRate));
    }
    // Set Min
    void ExpEnvelope::SetMin(float InMin) {
        Min = InMin;
    }
    // Set Max
    void ExpEnvelope::SetMax(float InMax) {
        Max = InMax;
    }
    // Reset the current position to the start of the envelope
    void ExpEnvelope::ResetEnvelope() {
        EnvPos = 0.0;
        PreviousValue = Min;
        HasStarted = true;
    }
    // Get the next position of the envelope
    float ExpEnvelope::GetNextEnvelopePoint() {
        float EnvReturnValue = Min;
        if (HasStarted == true) {
            if (EnvPos <= Attack + Hold) {
                EnvReturnValue = Max + (PreviousValue - Max) * AttackTimeConst;
                if (EnvReturnValue > Max) {
                    EnvReturnValue = Max;
                }
            }
            else if (EnvPos <= Attack + Hold + Decay) {
                EnvReturnValue = Sustain + (PreviousValue - Sustain) * DecayTimeConst;
                if (EnvReturnValue < Sustain) {
                    EnvReturnValue = Sustain;
                }
            }
            else {
                EnvReturnValue = Min + (PreviousValue - Min) * ReleaseTimeConst;
                if (EnvReturnValue < Min) {
                    EnvReturnValue = Min;
                }
            }
            if (EnvPos <= Attack + Hold + Decay + Release) {
                EnvPos += EnvPosInc;
            }

            PreviousValue = EnvReturnValue;
        }

        return EnvReturnValue;
    }

    /*### EXPONENTIAL ENVELOPPE CLASS 2 ###*/

    // Default constructor
    ExpEnvelope2::ExpEnvelope2() {}
    // Enveloppe Class constructor declaration
    ExpEnvelope2::ExpEnvelope2(int InSampleRate, float InAttack, float InHold, float InDecay, float InSustain, float InRelease) {
        SampleRate = std::max(InSampleRate, 1);
        Hold = std::max(InHold, 0.0f);
        Attack = std::max(InAttack, 0.0001f);
        Decay = std::max(InDecay, 0.0001f);
        Min = 0.0f;
        Max = 1.0f;
        Sustain = Clamp(InSustain, Min, Max);
        Release = std::max(InRelease, 0.0001f);
        EnvPosInc = 1.0f / (float)SampleRate;
        EnvPos = 0.0f;
        HasStarted = false;
    }
    ExpEnvelope2::ExpEnvelope2(int InSampleRate, float InAttack, float InHold, float InDecay, float InSustain, float InRelease, float InMin, float InMax) {
        SampleRate = std::max(InSampleRate, 1);
        Hold = std::max(InHold, 0.0f);
        Attack = std::max(InAttack, 0.0001f);
        Decay = std::max(InDecay, 0.0001f);
        Min = InMin;
        Max = std::max(InMax, Min);
        Sustain = Clamp(InSustain, Min, Max);
        Release = std::max(InRelease, 0.0001f);
        EnvPosInc = 1.0f / (float)SampleRate;
        EnvPos = 0.0f;
        HasStarted = false;
    }

    // Set Attack time in s
    void ExpEnvelope2::SetAttack(float InAttack) {
        Attack = std::max(InAttack, 0.0001f);
    }
    // Set Hold time in s
    void ExpEnvelope2::SetHold(float InHold) {
        Hold = std::max(InHold, 0.0f);
    }
    // Set Decay time in s
    void ExpEnvelope2::SetDecay(float InDecay) {
        Decay = std::max(InDecay, 0.0001f);
    }
    // Set Sustain value
    void ExpEnvelope2::SetSustain(float InSustain) {
        Sustain = Clamp(InSustain, Min, Max);
    }
    // Set Release time in s
    void ExpEnvelope2::SetRelease(float InRelease) {
        Release = std::max(InRelease, 0.0001f);
    }
    // Set Min
    void ExpEnvelope2::SetMin(float InMin) {
        Min = InMin;
    }
    // Set Max
    void ExpEnvelope2::SetMax(float InMax) {
        Max = std::max(InMax, Min);
    }
    // Reset the current position to the start of the envelope
    void ExpEnvelope2::ResetEnvelope() {
        EnvPos = 0.0f;
        HasStarted = true;
    }
    // Get the next position of the envelope
    float ExpEnvelope2::GetNextEnvelopePoint() {
        float EnvReturnValue = Min;
        float MockMin = std::max(Min, 0.0001f);

        if (HasStarted == true) {
            if (EnvPos <= Attack + Hold) {
                EnvReturnValue = std::min(MockMin * (float)pow(Max / MockMin, EnvPos / Attack), Max);
                if (abs(EnvReturnValue - Max) < 0.005f || EnvReturnValue >= Max) {
                    EnvReturnValue = Max;
                }
            }
            else if (EnvPos <= Attack + Hold + Decay) {
                EnvReturnValue = std::max(Max * (float)pow(Sustain / Max, (EnvPos - Attack - Hold) / Decay), Sustain);
                if (abs(EnvReturnValue - Sustain) < 0.005f || EnvReturnValue <= Sustain) {
                    EnvReturnValue = Sustain;
                }
            }
            else {
                EnvReturnValue = std::max(Sustain * (float)pow(MockMin / Sustain, (EnvPos - Attack - Hold - Decay) / Release), Min);
                if (abs(EnvReturnValue - Min) < 0.005f || EnvReturnValue <= Min) {
                    EnvReturnValue = Min;
                }
            }

            if (EnvPos <= Attack + Hold + Decay + Release) {
                EnvPos += EnvPosInc;
            }
        }
        return EnvReturnValue;
    }

    /*### EXPONENTIAL TARGET CLASS ###*/

    // Default constructor
    ExpTarget::ExpTarget() {}
    // Enveloppe Class constructor declaration
    ExpTarget::ExpTarget(int InSampleRate, float InInitVal, float InFinalVal, float InTimeConst) {
        SampleRate = std::max(InSampleRate, 1);
        if (InTimeConst > 0.0f) {
            TimeConst = exp(-1.0f / (InTimeConst * (float)SampleRate));
        }
        else {
            TimeConst = 0.0f;
        }
        InitValue = InInitVal;
        PreviousValue = InInitVal;
        FinalValue = InFinalVal;
        HasStarted = false;
    }
    // Set Inital Value (Only affects the envelope after it is reset)
    void ExpTarget::SetInitValue(float InInitVal) {
        InitValue = InInitVal;
    }
    // Set Final Value
    void ExpTarget::SetFinalValue(float InFinalVal) {
        FinalValue = InFinalVal;
    }
    // Set Time Constant
    void ExpTarget::SetTimeConst(float InTimeConst) {
        if (InTimeConst > 0.0f) {
            TimeConst = exp(-1.0f / (InTimeConst * (float)SampleRate));
        }
        else {
            TimeConst = 0.0f;
        }
    }
    // Restarts the envelope
    void ExpTarget::ResetEnvelope() {
        PreviousValue = InitValue;
        HasStarted = true;
    }
    // Get the next position of the envelope
    float ExpTarget::GetNextEnvelopePoint() {
        float EnvReturnValue = InitValue;
        if (HasStarted == true) {
            EnvReturnValue = FinalValue + (PreviousValue - FinalValue) * TimeConst;
            if (abs(EnvReturnValue - FinalValue) < 0.005f) {
                EnvReturnValue = FinalValue;
            }
            PreviousValue = EnvReturnValue;
        }
        return EnvReturnValue;
    }

    /*### LINEAR RAMP CLASS ###*/

    // Default constructor
    LinRamp::LinRamp() {}
    // Envelope Class constructor declaration
    LinRamp::LinRamp(int InSampleRate, float InAttack, float InInitVal, float InFinalVal) {
        SampleRate = std::max(InSampleRate, 1);
        InitValue = InInitVal;
        FinalValue = InFinalVal;
        AttackTime = std::max(InAttack, 0.0001f);
        EnvPos = 0.0f;
        EnvInc = 1.0f / (float)SampleRate;
        HasStarted = false;
    }
    // Set Inital Value (Only affects the envelope after it is reset)
    void LinRamp::SetInitValue(float InInitVal) {
        InitValue = InInitVal;
    }
    // Set Final Value
    void LinRamp::SetFinalValue(float InFinalVal) {
        FinalValue = InFinalVal;
    }
    // Set Time Constant
    void LinRamp::SetAttackTime(float InAttack) {
        AttackTime = std::max(InAttack, 0.0001f);
    }
    // Restarts the envelope
    void LinRamp::ResetEnvelope() {
        EnvPos = 0.0f;
        HasStarted = true;
    }
    // Get the next position of the envelope
    float LinRamp::GetNextEnvelopePoint() {
        float EnvReturnValue = InitValue;
        if (HasStarted == true) {
            if (EnvPos <= AttackTime) {
                EnvReturnValue = InitValue + EnvPos * (FinalValue - InitValue) / AttackTime;
            }
            else if (EnvPos > AttackTime) {
                EnvReturnValue = FinalValue;
            }
            if (EnvPos <= AttackTime) {
                EnvPos += EnvInc;
            }
        }
        return EnvReturnValue;
    }

    /*### BIQUAD FILTER CLASS ###*/

    // Default constructor
    BiquadFilter::BiquadFilter() {}
    // Biquad Filter Class constructor declaration
    BiquadFilter::BiquadFilter(int InSampleRate, float InFrequency, float InQFactor, float InPeakGainDB, int InType) {
        SampleRate = std::max(InSampleRate, 1);
        Type = InType;
        V = pow(10.0f, InPeakGainDB / 40.0f);
        W = 2.0f * (float)NEM_PI * std::min(std::max(InFrequency, 1.0f) / (float)SampleRate, 0.499f);
        Q = abs(InQFactor);
        AQ = sin(W) / (2.0f * std::max(Q, 0.001f));
        AQdB = sin(W) / (2.0f * pow(10.0f, Q / 20.0f));
        AS = sin(W) / sqrt(2.0f);
        Y1 = 0.0f;
        Y2 = 0.0f;
        X1 = 0.0f;
        X2 = 0.0f;
        ComputeCoeff();
    }
    void BiquadFilter::SetFrequency(float InFrequency) {
        W = 2.0f * (float)NEM_PI * std::min(std::max(InFrequency, 1.0f) / (float)SampleRate, 0.499f);
        AQ = sin(W) / (2.0f * std::max(Q, 0.001f));
        AQdB = sin(W) / (2.0f * pow(10.0f, Q / 20.0f));
        AS = sin(W) / sqrt(2.0f);
        ComputeCoeff();
    }
    void BiquadFilter::SetQFactor(float InQFactor) {
        Q = abs(InQFactor);
        AQ = sin(W) / (2.0f * std::max(Q, 0.001f));
        AQdB = sin(W) / (2.0f * pow(10.0f, Q / 20.0f));
        ComputeCoeff();
    }
    void BiquadFilter::SetPeakGain(float InPeakGainDB) {
        V = pow(10.0f, InPeakGainDB / 40.0f);
        ComputeCoeff();
    }
    void BiquadFilter::SetType(int InType) {
        Type = InType;
        ComputeCoeff();
    }
    float BiquadFilter::ProcessSample(float InSample) {
        float out = ((InSample * B0) + (X1 * B1) + (X2 * B2) - (Y1 * A1) - (Y2 * A2)) / A0;
        Y2 = Y1;
        Y1 = out;
        X2 = X1;
        X1 = InSample;
        return out;
    }
    void BiquadFilter::ComputeCoeff(void) {
        float Wc = cos(W);
        switch (Type) {
        case bq_type_lowpass:
            B0 = (1.0f - Wc) / 2.0f;
            B1 = 1.0f - Wc;
            B2 = (1.0f - Wc) / 2.0f;
            A0 = 1.0f + AQdB;
            A1 = -2.0f * Wc;
            A2 = 1.0f - AQdB;
            break;

        case bq_type_highpass:
            B0 = (1.0f + Wc) / 2.0f;
            B1 = -1.0f - Wc;
            B2 = (1.0f + Wc) / 2.0f;
            A0 = 1.0f + AQdB;
            A1 = -2.0f * Wc;
            A2 = 1.0f - AQdB;
            break;

        case bq_type_bandpass:
            B0 = AQ;
            B1 = 0.0f;
            B2 = -AQ;
            A0 = 1.0f + AQ;
            A1 = -2.0f * Wc;
            A2 = 1.0f - AQ;
            break;

        case bq_type_notch:
            B0 = 1.0f;
            B1 = -2.0f * Wc;
            B2 = 1.0f;
            A0 = 1 + AQ;
            A1 = -2.0f * Wc;
            A2 = 1 - AQ;
            break;

        case bq_type_peak:
            B0 = 1.0f + AQ * V;
            B1 = -2.0f * Wc;
            B2 = 1.0f - AQ * V;
            A0 = 1.0f + AQ / V;
            A1 = -2.0f * Wc;
            A2 = 1.0f - AQ / V;
            break;

        case bq_type_lowshelf:
            B0 = V * (V + 1.0f + 2.0f * sqrt(V) * AS - (V - 1.0f) * Wc);
            B1 = 2.0f * V * (V - 1.0f - (V + 1.0f) * Wc);
            B2 = V * (V + 1.0f - 2.0f * sqrt(V) * AS - (V - 1.0f) * Wc);
            A0 = V + 1.0f + 2.0f * sqrt(V) * AS + (V - 1.0f) * Wc;
            A1 = -2.0f * (V - 1.0f + (V + 1.0f) * Wc);
            A2 = V + 1.0f - 2.0f * sqrt(V) * AS + (V - 1.0f) * Wc;
            break;


        case bq_type_highshelf:
            B0 = V * (V + 1.0f + 2.0f * sqrt(V) * AS + (V - 1.0f) * Wc);
            B1 = -2.0f * V * (V - 1.0f + (V + 1.0f) * Wc);
            B2 = V * (V + 1.0f - 2.0f * sqrt(V) * AS + (V - 1.0f) * Wc);
            A0 = V + 1.0f + 2.0f * sqrt(V) * AS - (V - 1.0f) * Wc;
            A1 = -2.0f * (V - 1.0f - (V + 1.0f) * Wc);
            A2 = V + 1.0f - 2.0f * sqrt(V) * AS - (V - 1.0f) * Wc;
            break;

        case bq_type_allpass:
            B0 = 1.0f - AQ;
            B1 = -2.0f * Wc;
            B2 = 1.0f + AQ;
            A0 = 1.0f + AQ;
            A1 = -2.0f * Wc;
            A2 = 1.0f - AQ;
            break;

        default:
            B0 = (1.0f - Wc) / 2.0f;
            B1 = 1.0f - Wc;
            B2 = (1.0f - Wc) / 2.0f;
            A0 = 1.0f + AQdB;
            A1 = -2.0f * Wc;
            A2 = 1.0f - AQdB;
            break;
        }
        return;
    }
    void BiquadFilter::ResetFilter() {
        Y1 = 0.0f;
        Y2 = 0.0f;
        X1 = 0.0f;
        X2 = 0.0f;
    }

    /*### SINGLE POLE LPF ###*/

    // Default Constructor
    OnePoleLPF::OnePoleLPF() {};
    // Constructor
    OnePoleLPF::OnePoleLPF(int InSampleRate, float InFrequency) {
        SampleRate = std::max(InSampleRate, 1);
        LastOut = 0.0f;
        float K = 2.0f * (float)NEM_PI * Clamp(InFrequency, 1.0f, ((float)SampleRate / 4.0f) - 0.001f) / (float)SampleRate;
        Coeff = std::min(1.0f - (1.0f / (float)cos(K)) + (float)tan(K), 0.999f);
    };
    // Sets the cut-off frequency of the filter
    void OnePoleLPF::SetFrequency(float InFrequency) {
        float K = 2.0f * (float)NEM_PI * Clamp(InFrequency, 1.0f, ((float)SampleRate / 4.0f) - 0.001f) / (float)SampleRate;
        Coeff = std::min(1.0f - (1.0f / (float)cos(K)) + (float)tan(K), 0.999f);
    };
    // Processes one input sample
    float OnePoleLPF::ProcessSample(float InSample) {
        LastOut = InSample * Coeff + (1.0f - Coeff) * LastOut;
        return LastOut;
    };

    /*### SINGLE POLE HPF ###*/

    // Default Constructor
    OnePoleHPF::OnePoleHPF() {};
    // Constructor
    OnePoleHPF::OnePoleHPF(int InSampleRate, float InFrequency) {
        SampleRate = std::max(InSampleRate, 1);
        LastOut = 0.0f;
        LastIn = 0.0f;
        float K = 2.0f * (float)NEM_PI * Clamp(InFrequency, 1.0f, ((float)SampleRate / 2.0f) - 0.001f) / (float)SampleRate;
        // prevents division by 0
        if (InFrequency == (float)SampleRate / 4.0f) { K = 0.001f + (float)NEM_PI / 2.0f; }
        Coeff = std::min(1.0f - (1.0f / (float)cos(K)) + (float)tan(K), 1.999f);
    };
    // Sets the cut-off frequency of the filter
    void OnePoleHPF::SetFrequency(float InFrequency) {
        float K = 2.0f * (float)NEM_PI * Clamp(InFrequency, 1.0f, ((float)SampleRate / 2.0f) - 0.001f) / (float)SampleRate;
        // prevents division by 0
        if (InFrequency == (float)SampleRate / 4.0f) { K = 0.001f + (float)NEM_PI / 2.0f; }
        Coeff = std::min(1.0f - (1.0f / (float)cos(K)) + (float)tan(K), 1.999f);
    };
    // Processes one input sample
    float OnePoleHPF::ProcessSample(float InSample) {
        LastOut = InSample * (2.0f - Coeff) / 2.0f - LastIn * (2.0f - Coeff) / 2.0f + (1.0f - Coeff) * LastOut;
        LastIn = InSample;
        return LastOut;
    };

    /*### LOW ORDER BPF ###*/

    // Default Constructor
    TwoPoleBPF::TwoPoleBPF() {};
    // Constructor
    TwoPoleBPF::TwoPoleBPF(int InSampleRate, float InFrequency, float InQFactor) {
        SampleRate = std::max(InSampleRate, 1);
        Frequency = Clamp(InFrequency, 1.0f, (float)SampleRate / 2.0f);
        Q = InQFactor;
        Z1 = 0.0f;
        Z2 = 0.0f;
        Coeff1 = 0.0f;
        Coeff2 = 0.0f;
        CompGain = 0.0f;
        ComputeCoeff();
    };

    // Sets the cut-off frequency of the filter
    void TwoPoleBPF::SetFrequency(float InFrequency) {
        Frequency = Clamp(InFrequency, 1.0f, (float)SampleRate / 2.0f);
        ComputeCoeff();
    };
    // Q-factor
    void TwoPoleBPF::SetQFactor(float InQFactor) {
        Q = InQFactor;
        ComputeCoeff();
    };
    // Processes one input sample
    float TwoPoleBPF::ProcessSample(float InSample) {
        float Output = InSample + (Coeff1 * Z1) + (Coeff2 * Z2);
        Z2 = Z1;
        Z1 = Output;
        return CompGain * Output;
    };
    // Update the coefficients
    void TwoPoleBPF::ComputeCoeff() {
        Q = std::min(0.001f, Q);
        float K = 2.0f * (float)NEM_PI * Frequency / (float)SampleRate; //  w = 2*pi*f/fc
        float OneMinusR = std::min(1.0f, K / Q);
        float R = 1.0f - OneMinusR;
        Coeff1 = 2.0f * cos(K) * R;
        Coeff2 = -R * R;
        CompGain = 2.0f * OneMinusR * (OneMinusR + R * K);
    };

    /*### HIGH ORDER BPF ###*/

    // Default Constructor
    HighOrderBPF::HighOrderBPF() {};
    // Constructor
    HighOrderBPF::HighOrderBPF(int InSampleRate, float InFrequency, float InQFactor) {
        SampleRate = std::max(InSampleRate, 1);
        Frequency = Clamp(InFrequency, 1.0f, (float)SampleRate / 2.0f);
        Q = InQFactor;
        Y1 = 0.0f;
        Y2 = 0.0f;
        Y3 = 0.0f;
        Y4 = 0.0f;
        X1 = 0.0f;
        X2 = 0.0f;
        X3 = 0.0f;
        X4 = 0.0f;
        C0 = 0.0f;
        C1 = 0.0f;
        C2 = 0.0f;
        C3 = 0.0f;
        C4 = 0.0f;
        CompGain = 0.0f;
        ComputeCoeff();
    };

    // Sets the cut-off frequency of the filter
    void HighOrderBPF::SetFrequency(float InFrequency) {
        Frequency = Clamp(InFrequency, 1.0f, (float)SampleRate / 2.0f);
        ComputeCoeff();
    };
    // Q-factor
    void HighOrderBPF::SetQFactor(float InQFactor) {
        Q = InQFactor;
        ComputeCoeff();
    };
    // Processes one input sample
    float HighOrderBPF::ProcessSample(float InSample) {
        float Output = (CompGain * (InSample - 2 * X2 + X4) - C1 * Y1 - C2 * Y2 - C3 * Y3 - C4 * Y4) / C0;
        X4 = X3;
        X3 = X2;
        X2 = X1;
        X1 = InSample;
        Y4 = Y3;
        Y3 = Y2;
        Y2 = Y1;
        Y1 = Output;
        return Output;
    };
    // Update the coefficients
    void HighOrderBPF::ComputeCoeff() {
        Q = std::min(0.001f, Q);
        float K = 2.0f * (float)NEM_PI * Frequency / (float)SampleRate; //  w = 2*pi*f/fc
        float B = K / Q;
        if (1.0f + tan(B / 2.0f) != 0.0f && !isnan(1.0f + tan(B / 2.0f))) {
            Beta = (1.0f - tan(B / 2.0f)) / (1.0f + tan(B / 2.0f));
        }
        float Tan2G = tan((float)NEM_PI / 8.0f) * tan((float)NEM_PI / 8.0f);
        float Norm = (1.0f - Beta) / (2.0f * cos((float)NEM_PI / 8.0f));
        Norm = Norm * Norm;
        C0 = 1.0f + Beta * Beta * Tan2G;
        C1 = 0.0f - 2.0f * (1.0f + Beta) * cos(K) * (Beta * Tan2G + 1.0f);
        C2 = (Tan2G + 1.0f) * (2.0f * Beta + (1.0f + Beta) * (1.0f + Beta) * cos(K) * cos(K));
        C3 = 0.0f - 2.0f * (1.0f + Beta) * cos(K) * (Beta + Tan2G);
        C4 = Beta * Beta + Tan2G;
    };

    /*### SIGNAL DIFFERENTIAL ###*/

    // Default Constructor
    SigmaDelta::SigmaDelta() {};
    // Constructor
    SigmaDelta::SigmaDelta(float InGain) {
        Gain = InGain;
    };

    void SigmaDelta::SetGain(float InGain) {
        Gain = InGain;
    }
    // Processes one input sample
    float SigmaDelta::ProcessSample(float InSample) {
        float Output = Gain * (InSample - LastIn);
        LastIn = InSample;
        return Output;
    };

    /*### CLIP PROCESSOR ###*/

    // Default Constructor
    ClipProcessor::ClipProcessor() {};
    // Constructor
    ClipProcessor::ClipProcessor(float InLowThreshold, float InHighThreshold) {
        HiThresh = std::max(InHighThreshold, 0.0f);
        LoThresh = std::min(InLowThreshold, 0.0f);
    }

    void ClipProcessor::SetLowThresh(float InLowThreshold) {
        LoThresh = std::min(InLowThreshold, 0.0f);
    }
    void ClipProcessor::SetHighThresh(float InHighThreshold) {
        HiThresh = std::max(InHighThreshold, 0.0f);
    }
    float ClipProcessor::ProcessSample(float InSample) {
        float Output = InSample;
        if (InSample < LoThresh) Output = LoThresh;
        else if (InSample > HiThresh) Output = HiThresh;
        return Output;
    }

    /*### OVERDRIVE PROCESSOR ###*/

    // Default Constructor
    OverDriveProcessor::OverDriveProcessor() {
        ComputeCoeff();
    };
    // Constructor
    OverDriveProcessor::OverDriveProcessor(float InVolumedB, float InDrivedB, float InBias, float InKnee) {
        Volume = pow(10.0f, InVolumedB / 20.0f); // Output Gain
        Drive = pow(10.0f, InDrivedB / 20.0f); // Input Gain
        Bias = InBias;
        Knee = std::max(InKnee, 0.001f);
        ComputeCoeff();
        Alpha = 0.99f;
        Prev = 0.0f;
    }

    void OverDriveProcessor::SetVolume(float InVolumedB) {
        Volume = pow(10.0f, InVolumedB / 20.0f); // Output Gain
        ComputeCoeff();
    }
    void OverDriveProcessor::SetDrive(float InDrivedB) {
        Drive = pow(10.0f, InDrivedB / 20.0f); // Input Gain
    }
    void OverDriveProcessor::SetBias(float InBias) {
        Bias = InBias;
    }
    void OverDriveProcessor::SetKnee(float InKnee) {
        Knee = InKnee;
        ComputeCoeff();
    }
    float OverDriveProcessor::ProcessSample(float InSample) {
        float X = InSample * Drive;
        float Y = 0.0f;
        if (X > (1 - Knee)) {
            if (X >= (1 + Knee)) Y = Volume;
            else Y = C2 * X * X + C1 * X + C0;
        }
        else if (X < -(1 - Bias) * (1 - Knee)) {
            if (X <= -(1 - Bias) * (1 + Knee)) Y = -(1 - Bias) * Volume;
            else Y = -C2 * X * X / (1 - Bias) + C1 * X - C0 * (1 - Bias);
        }
        else Y = X * Volume;

        float Output = Alpha * Y + (1 - Alpha) * Prev;
        Prev = Output;
        return Output;
    }
    void OverDriveProcessor::ComputeCoeff() {
        C2 = -Volume / (4.0f * Knee);
        C1 = Volume * (1.0f + Knee) / (2.0f * Knee);
        C0 = -Volume * (1.0f - Knee) * (1.0f - Knee) / (4.0f * Knee);
    }

    /*### DISTORTION PROCESSOR ###*/

   // Default Constructor
    DistortionProcessor::DistortionProcessor() {};
    // Constructor
    DistortionProcessor::DistortionProcessor(float InAmount) {
        Amount = InAmount;
        // By default the class matches its JS counterpart, with the output being scaled down by 1/3.
        Gain = 1.0f / 3.0f;
    }
    DistortionProcessor::DistortionProcessor(float InAmount, float OutputGain) {
        Amount = InAmount;
        Gain = OutputGain;
    }

    void DistortionProcessor::SetAmount(float InAmount) {
        Amount = InAmount;
    }
    void DistortionProcessor::SetGain(float OutputGain) {
        Gain = OutputGain;
    }
    float DistortionProcessor::ProcessSample(float InSample) {
        // In Javascript, we calculate the output like this:
        // float Output = (3 + Amount) * InSample / (9 * (1 + (Amount / NEM_PI) * abs(InSample)));
        // This is almost exactly equivalent to the much simpler function:
        float Output = (3.0f + Amount) * InSample / (3.0f + Amount * abs(InSample));
        return Gain * Output;
    }

    /*### STEREO PANNER ###*/

    StereoPanner::StereoPanner() {}
    StereoPanner::StereoPanner(float InPanParam) {
        pan = std::max(-1.0f, InPanParam);
        pan = std::min(1.0f, InPanParam);
    }
    void StereoPanner::SetPan(float InPanParam) {
        pan = std::max(-1.0f, InPanParam);
        pan = std::min(1.0f, InPanParam);
    }
    std::vector<float> StereoPanner::ProcessSample(float InSample) {
        float gainL = cos((pan + 1.0f) * (float)NEM_PI / 4.0f);
        float gainR = sin((pan + 1.0f) * (float)NEM_PI / 4.0f);
        std::vector<float> output{ InSample * gainL, InSample * gainR };
        return output;
    }
    std::vector<float> StereoPanner::ProcessSample(float InLeftSample, float InRightSample) {
        float x;
        if (pan <= 0) {
            x = pan + 1.0f;
        }
        else {
            x = pan;
        }
        float gainL = cos(x * (float)NEM_PI / 2.0f);
        float gainR = sin(x * (float)NEM_PI / 2.0f);
        float outputL = 0.0f;
        float outputR = 0.0f;
        if (pan <= 0) {
            outputL = InLeftSample + InRightSample * gainL;
            outputR = InRightSample * gainR;
        }
        else {
            outputL = InLeftSample * gainL;
            outputR = InRightSample + InLeftSample * gainR;
        }
        std::vector<float> output{ outputL, outputR };
        return output;
    }


    /*### DELAY ###*/

    const float MAX_DELAY_TIME = 5.0f;
    const int MAX_DELAY_TIME_INT = (int)MAX_DELAY_TIME;
    // Default Constructor
    Delay::Delay() {
        SampleRate = 48000;
        DelayTime = 1.0f;
        DelayBuffer.resize(MAX_DELAY_TIME_INT * SampleRate, 0.0f);
        ReadPointer = (int)((MAX_DELAY_TIME - DelayTime) * (float)SampleRate) - 1;
        WritePointer = 0;
    }
    // Delay class constructor declaration
    Delay::Delay(int InSampleRate, float InDelayTime) {
        SampleRate = InSampleRate;
        DelayTime = nemlib::Clamp(InDelayTime, 0.0f, MAX_DELAY_TIME - 1.0f / (float)SampleRate);
        DelayBuffer.resize(MAX_DELAY_TIME_INT * SampleRate, 0.0f);
        ReadPointer = (int)((MAX_DELAY_TIME - DelayTime) * (float)SampleRate) - 1;
        WritePointer = 0;
    }
    void Delay::SetDelay(float InDelayTime) {
        DelayTime = nemlib::Clamp(InDelayTime, 0.0f, MAX_DELAY_TIME - 1.0f / (float)SampleRate);
        ReadPointer = WritePointer - (int)(DelayTime * (float)SampleRate);
        while (ReadPointer < 0) {
            ReadPointer += MAX_DELAY_TIME_INT * SampleRate;
        }
        while (ReadPointer >= MAX_DELAY_TIME_INT * SampleRate) {
            ReadPointer--;
        }
    }
    float Delay::ProcessSample(float InSample) {
        // Get delayed output
        float Output = DelayBuffer[ReadPointer];
        // Updated buffer
        DelayBuffer[WritePointer] = InSample;
        // Increment pointers
        ReadPointer++;
        WritePointer++;
        // Wrap pointers
        if (ReadPointer >= MAX_DELAY_TIME_INT * SampleRate) {
            ReadPointer = 0;
        }
        if (WritePointer >= MAX_DELAY_TIME_INT * SampleRate) {
            WritePointer = 0;
        }
        return Output;
    }

    /*### FEEDBACK DELAY ###*/

    // Default Constructor
    FeedbackDelay::FeedbackDelay() {
        SampleRate = 48000;
        Feedback = 0.5f;
        Dry = 0.0f;
        Wet = 1.0f;
        Delay = nemlib::Delay(48000, 0.5f);
        Prev = 0.0f;
    }
    // FB Delay class constructor declaration
    FeedbackDelay::FeedbackDelay(int InSampleRate, float InDelayTime, float InFeedbackGain, float InDryGain, float InWetGain) {
        SampleRate = InSampleRate;
        Feedback = InFeedbackGain;
        Dry = InDryGain;
        Wet = InWetGain;
        Delay = nemlib::Delay(InSampleRate, InDelayTime);
        Prev = 0.0f;
    }
    void FeedbackDelay::SetDelay(float InDelayTime) { Delay.SetDelay(InDelayTime); }
    void FeedbackDelay::SetFeedback(float InFeedbackGain) { Feedback = InFeedbackGain; }
    void FeedbackDelay::SetDryGain(float InDryGain) { Dry = InDryGain; }
    void FeedbackDelay::SetWetGain(float InWetGain) { Wet = InWetGain; }
    float FeedbackDelay::ProcessSample(float InSample) {

        float DelaySample = Delay.ProcessSample(InSample + Prev);
        Prev = DelaySample * Feedback;
        float Output = Dry * InSample + Wet * DelaySample;
        return Output;
    }

    /*### Haas Effect  ###*/

    HaasEffect::HaasEffect() {
        Wet = nemlib::Delay(48000, 25.0f / 1000.0f);
        Separation = 0.5f;
    }
    HaasEffect::HaasEffect(int InSampleRate, float InDepth, float InSeparation) {
        Wet = nemlib::Delay(InSampleRate, std::max(InDepth, 1.0f) / 1000.0f);
        Separation = nemlib::Clamp(InSeparation, -1.0f, 1.0f);
    }
    std::vector<float> HaasEffect::ProcessSample(float InSample) {
        float DelayedSample = Wet.ProcessSample(InSample);
        float LeftSample = DelayedSample * ((Separation + 1.0f) / 2.0f) + InSample * ((1.0f - Separation) / 2.0f);
        float RightSample = DelayedSample * ((1.0f - Separation) / 2.0f) + InSample * ((Separation + 1.0f) / 2.0f);
        std::vector<float> Output{ LeftSample, RightSample };
        return Output;
    }
    void HaasEffect::SetDepth(float InDepth) {
        Wet.SetDelay(InDepth);
    }
    void HaasEffect::SetSeparation(float InSeparation) {
        Separation = InSeparation;
    }

    /*### RMS PROCESSOR ###*/

    // Default Constructor
    RMS::RMS() {
        SampleRate = 48000;
        WindowLength = 256;
        WindowCount = 0;
        SquareSum = 0.0f;
        Output = 0.0f;
    }
    // RMS class constructor declaration
    RMS::RMS(int InSampleRate) {
        SampleRate = std::max(InSampleRate, 1);
        WindowLength = 256;
        WindowCount = 0;
        SquareSum = 0.0f;
        Output = 0.0f;
    }
    // RMS class constructor declaration
    RMS::RMS(int InSampleRate, int InWindowLength) {
        SampleRate = std::max(InSampleRate, 1);
        WindowLength = std::max(InWindowLength, 1);
        WindowCount = 0;
        SquareSum = 0.0f;
        Output = 0.0f;
    }
    void RMS::SetWindowLength(int InWindowLength) {
        WindowLength = std::max(InWindowLength, 1);
    }
    float RMS::ProcessSample(float InSample) {
        SquareSum += pow(InSample, 2.0f);
        WindowCount++;
        if (WindowCount == WindowLength) {
            Output = sqrt(SquareSum / (float)WindowLength);
            SquareSum = 0.0f;
            WindowCount = 0;
        }
        return Output;
    }

    /*### PULSE PROCESSOR ###*/
    PulseProcessor::PulseProcessor() {
        SampleRate = 48000;
        srand(static_cast <unsigned> (time(0)));
        SampleNum = 0;
        SampleCounter = 0;
        decayS = 0.0f;
    }
    PulseProcessor::PulseProcessor(int InSampleRate) {
        SampleRate = InSampleRate;
        srand(static_cast <unsigned> (time(0)));
        SampleNum = 0;
        SampleCounter = 0;
        decayS = 0.0f;
    }
    float PulseProcessor::ProcessSample(float InSample) {
        float Output = 0.0f;
        if (SampleCounter == 255) {
            if (InSample > 0.49f && InSample < 0.52f) {
                float random = (float)rand() / (float)RAND_MAX;
                decayS = (random * 30.0f) * (float)SampleRate / 1000.0f;
            }
            else {
                decayS = 0.0f;
            }
            SampleCounter = 0;
        }
        if (decayS > 0.0f) {
            if ((float)SampleNum < decayS) {
                Output = pow(1.0f - (float)SampleNum / decayS, 2.0f);
            }
            else if (SampleNum > 256) {
                SampleNum = -1;
            }
            SampleNum++;
        }
        SampleCounter++;
        return Output;
    }
    float PulseProcessor::ProcessSample(float InSample, nemlib::BiquadFilter InFilter) {
        float Output = 0.0f;
        if (SampleCounter == 255) {
            if (InSample > 0.49f && InSample < 0.52f) {
                float random = (float)rand() / (float)RAND_MAX;
                decayS = (random * 30.0f) * (float)SampleRate / 1000.0f;
                InFilter.SetFrequency(1500.0f + (500.0f * decayS * 1000.0f / (float)SampleRate));
            }
            else {
                decayS = 0.0f;
            }
            SampleCounter = 0;
        }
        if (decayS > 0.0f) {
            if ((float)SampleNum < decayS) {
                Output = pow(1.0f - (float)SampleNum / decayS, 2.0f);
            }
            else if (SampleNum > 256) {
                SampleNum = -1;
            }
            SampleNum++;
        }
        SampleCounter++;
        return Output;
    }

    /*### VARY FUNCTION ###*/
    float Vary(float InValue, float InAmount) {
        float Random = float(rand()) / float(RAND_MAX);
        return InValue * (1.0f + InAmount * (2.0f * Random - 1.0f));
    }

    /*### FILTER BANK ###*/
    FilterBank::FilterBank() {
        SampleRate = 48000;
        MuteGain = 1.0f;
        for (int i = 0; i < 9; i++) {
            BiquadFilter Filter = BiquadFilter(SampleRate, 200.0f, 1.0f, 0.0f, 2);
            Filters.push_back(Filter);
            FilterBandGains.push_back(1.0f);
        }
        srand(static_cast <unsigned> (time(0)));
        OutputMult = 0.0f;
    }
    FilterBank::FilterBank(int InSampleRate, int InNumFilters) {
        SampleRate = std::max(InSampleRate, 1);
        MuteGain = 1.0f;
        for (int i = 0; i < InNumFilters; i++) {
            BiquadFilter Filter = BiquadFilter(SampleRate, 200.0f, 1.0f, 0.0f, 2);
            Filters.push_back(Filter);
            FilterBandGains.push_back(1.0f);
        }
        srand(static_cast <unsigned> (time(0)));
        OutputMult = 0.0f;
    }
    void FilterBank::InitialiseFilterBank(Mode InFilterInfo) {
        for (int i = 0; i < InFilterInfo.nModes; i++) {
            Filters[i].ResetFilter();
            Filters[i].SetType(InFilterInfo.Types[i]);
            Filters[i].SetFrequency(InFilterInfo.Freqs[i]);
            Filters[i].SetQFactor(InFilterInfo.Qs[i]);
            FilterBandGains[i] = InFilterInfo.Gains[i];
        }
        for (int i = InFilterInfo.nModes; i < 9; i++) {
            FilterBandGains[i] = 0.0f;
        }
        OutputMult = 0.0f;
    }
    void FilterBank::VaryParameters(Mode InFilterInfo) {
        for (int i = 0; i < InFilterInfo.nModes; i++) {
            Filters[i].SetFrequency(Vary(InFilterInfo.Freqs[i], 0.2f));
            Filters[i].SetQFactor(Vary(InFilterInfo.Qs[i], 0.3f));
            FilterBandGains[i] = Vary(InFilterInfo.Gains[i], 0.3f);
        }
    }
    void FilterBank::ResetFilter() {
        for (int i = 0; i < (int)Filters.size(); i++) {
            FilterBandGains[i] = 0.0f;
        }
    }
    void FilterBank::Mute() {
        MuteGain = 0.0f;
    }
    void FilterBank::Unmute() {
        MuteGain = 1.0f;
    }
    void FilterBank::Unmute(float InGain) {
        MuteGain = InGain;
    }
    float FilterBank::ProcessSample(float InSample) {
        float Output = 0.0f;
        if (OutputMult < 1.0f) {
            OutputMult += 1.0f / (0.01f * (float)SampleRate);
        }
        for (int i = 0; i < (int)Filters.size(); i++) {
            Output += Filters[i].ProcessSample(InSample) * FilterBandGains[i];
        }
        return Output * MuteGain * OutputMult * OutputMult;
    }

    /*### CURVE ENVELOPE ###*/
    CurveEnvelope::CurveEnvelope() {
        SampleRate = 48000;
        EnvPosInc = 1.0f / 48000.0f;
        EnvPos = 0.0f;
        HasStarted = false;
        Values = { 0.0f, 1.0f, 0.0f };
        NumOfValues = 3;
        Time = 0.5f;
        Counter = 1;
        Boundary = Time / (float)NumOfValues;
    }
    CurveEnvelope::CurveEnvelope(int InSampleRate, std::vector<float> InValues, float InTime) {
        SampleRate = std::max(InSampleRate, 1);
        EnvPosInc = 1.0f / (float)SampleRate;
        EnvPos = 0.0f;
        HasStarted = false;
        if (InValues.size() < 2) {
            Values = { 0.0f, 1.0f, 0.0f };
            NumOfValues = 3;
        }
        else {
            Values = InValues;
            NumOfValues = (int)Values.size();
        }
        Time = std::max(InTime, 3.0f * (float)(NumOfValues / SampleRate));
        Counter = 1;
        Boundary = Time / (float)NumOfValues;
    }
    CurveEnvelope::CurveEnvelope(int InSampleRate, std::vector<float> InValues, std::vector<float> InTimes) {
        SampleRate = std::max(InSampleRate, 1);
        EnvPosInc = 1.0f / (float)SampleRate;
        EnvPos = 0.0f;
        HasStarted = false;
        if ((int)InValues.size() < 2) {
            Values = { 0.0f, 1.0f, 0.0f };
            NumOfValues = 3;
        }
        else {
            Values = InValues;
            NumOfValues = (int)Values.size();
        }
        if ((int)InTimes.size() == NumOfValues - 1) {
            Times = InTimes;
            Time = 0.0f;
            for (int i = 0; i < (int)Times.size(); i++) {
                Time += Times[i];
            }
        }
        else {
            Times = std::vector<float>();
            Time = 0.5f;
        }
        Counter = 1;
        Boundary = Times[0];
    }
    void CurveEnvelope::SetValues(std::vector<float> InValues) {
        if (InValues.size() < 2) {
            Values = { 0.0f, 1.0f, 0.0f };
            NumOfValues = 3;
        }
        else {
            Values = InValues;
            NumOfValues = (int)Values.size();
        }
        Time = std::max(Time, 3.0f * (float)(NumOfValues / SampleRate));
        ResetEnvelope();
    }
    void CurveEnvelope::SetTime(float InTime) { Time = std::max(InTime, 3.0f * (float)(NumOfValues / SampleRate)); }
    void CurveEnvelope::SetTimes(std::vector<float> InTimes) {
        if ((int)InTimes.size() == NumOfValues - 1) {
            Times = InTimes;
            Time = 0.0f;
            for (int i = 0; i < (int)Times.size(); i++) {
                Time += Times[i];
            }
        }
        else {
            Times = std::vector<float>();
            Time = 0.5f;
        }
    }
    float CurveEnvelope::GetNextEnvelopePoint() {
        float EnvReturnValue = Values[0];
        if (HasStarted == true) {
            if (Times.empty()) {
                if (Counter <= NumOfValues - 1) {
                    if (EnvPos <= Boundary) {
                        EnvReturnValue = Values[Counter] + (EnvPos - Boundary) * (float)NumOfValues * (Values[Counter] - Values[Counter - 1]) / Time;
                    }
                    else {
                        Counter++;
                        Boundary += Time / (float)NumOfValues;
                        if (Counter <= NumOfValues - 1) {
                            EnvReturnValue = Values[Counter] + (EnvPos - Boundary) * (float)NumOfValues * (Values[Counter] - Values[Counter - 1]) / Time;
                        }
                        else {
                            EnvReturnValue = Values.back();
                        }
                    }
                    if (EnvPos <= Time) {
                        EnvPos += EnvPosInc;
                    }
                }
                else {
                    EnvReturnValue = Values.back();
                }
            }
            else {
                if (Counter <= NumOfValues - 1) {
                    if (EnvPos <= Boundary) {
                        EnvReturnValue = Values[Counter] + (EnvPos - Boundary) * (Values[Counter] - Values[Counter - 1]) / Times[Counter - 1];
                    }
                    else {
                        Counter++;
                        Boundary += Times[Counter - 1];
                        if (Counter <= NumOfValues - 1) {
                            EnvReturnValue = Values[Counter] + (EnvPos - Boundary) * (Values[Counter] - Values[Counter - 1]) / Times[Counter - 1];
                        }
                        else {
                            EnvReturnValue = Values.back();
                        }
                    }
                    if (EnvPos <= Time) {
                        EnvPos += EnvPosInc;
                    }
                }
                else {
                    EnvReturnValue = Values.back();
                }
            }
        }
        return EnvReturnValue;
    }
    void CurveEnvelope::ResetEnvelope() {
        EnvPos = 0.0f;
        HasStarted = true;
        Counter = 1;
        if (Times.empty()) {
            Boundary = Time / (float)NumOfValues;
        }
        else {
            Boundary = Times[0];
        }
    }
}
