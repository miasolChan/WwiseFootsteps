#pragma once

#include "FootstepsLibrary.h"
#include <AK/SoundEngine/Common/AkCommonDefs.h>
//#include <Windows.h>

// Struct for Shoe envelope modifiers
struct ShoeEnvelope {
    float HeelGain;
    float HeelAttack;
    float HeelSustain;
    float HeelDecay;
    float HeelRelease;
    float StepSeparation;
    float BallGain;
    float BallAttack;
    float BallSustain;
    float BallDecay;
    float BallRelease;
};

// Struct for Surface envelope modifiers
struct SurfaceEnvelope {
    float HeelAttack;
    float HeelSustain;
    float HeelDecay;
    float HeelRelease;
    float BallAttack;
    float BallSustain;
    float BallDecay;
    float BallRelease;
};

class Generator
{
public:
	Generator();
	~Generator();

    //Model Step
	void PrepareModel(AkUInt32 in_sampleRate);
	void UpdateStepEnvelope();
	float IncrementTheModelChannel();
    void ExcuteModel(AkReal32* pBuf, AkUInt16 in_uValidFrames);

    //Set Parameters
    void SetShoeType(AkInt32 in_ShoeType);
    void SetSurfaceType(AkInt32 in_SurfaceType);
    void SetTerrain(AkInt32 in_Terrain);
    void SetPace(AkReal32 in_Pace);
    void SetFirmness(AkReal32 in_Firmness);
    void SetSteadiness(AkReal32 in_Steadiness);
    void SetAutomeated(bool in_Automated);

	//Model Parameters Update
	void UpdatePaceModifiers(float Pace);
	void UpdateShoeModifiers(int ShoeType);
	void UpdateSurfaceModifiers(int SurfaceType);

	void CrunchLoop();
	void VaryFilterBank();

    ShoeEnvelope AddVariation();

    //
    AkInt32 m_sampleRate;//sample rate
    AkInt32 m_ShoeType;
    AkInt32 m_SurfaceType;
    AkInt32 m_Terrain;
    AkReal32 m_Pace;
    AkReal32 m_Firmness;
    AkReal32 m_Steadiness;
    bool m_Automated;

private:

    //Changed Parameters
    bool m_ShoeTypeChanged;
    bool m_SurfaceTypeChanged;
    bool m_TerrainChanged;
    bool m_PaceChanged;
    bool m_FirmnessChanged;
    bool m_SteadinessChanged;
    bool m_AutomatedChanged;
    // Library Components
    nemlib::CurveEnvelope HeelEnv;
    nemlib::CurveEnvelope BallEnv;
    nemlib::WhiteNoiseGen Noise;
    nemlib::BiquadFilter Highpass;
    nemlib::FilterBank Filters;
    nemlib::DistortionProcessor Distortion;
    nemlib::BiquadFilter CrunchBP;
    nemlib::CurveEnvelope CrunchEnv;
    nemlib::Delay SeparationDelay;
    nemlib::Timer StepTimer;
    nemlib::Timer CrunchTimer;
    nemlib::BiquadFilter OutHP;
    nemlib::BiquadFilter OutLP;
    // Model Variables
    //float Pace = 82.0f;
    //float Steadiness = 0.1f;
    //float Firmness = 0.3f;
    //bool Automated = false;
    ShoeEnvelope Shoe = { 1.0f, 1.0f, 0.0f, 10.0f, 0.1f, 40.0f, 0.5f, 1.0f, 0.0f, 20.0f, 0.1f };
    SurfaceEnvelope Surface = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    //int SurfaceType;
    //int ShoeType;
    //int TerrainType = 0;
    // Helper variables
    bool CrunchFlag = false;
    float RollSpeedPercentage = 1.92f;
    std::vector<float> HeelToBallRatio = { 0.8f, 0.5f };
    float CrunchOut = 0.0f;
    float FiltersOut = 1.0f;
    float StepCounter = 0.0f;
    float Freq1 = 0.0f;
    float Freq2 = 0.0f;
    float Delay1 = 0.0f;
    float Delay2 = 0.0f;
    float LastOut = 0.0f;
    // Constants
    std::vector<nemlib::Mode> Modes = {
        { // Wood
            9,
            { 0, 2, 2, 2, 2, 2, 2, 2, 2 },
            { 80.0f, 95.0f, 134.0f, 139.0f, 154.0f, 201.0f, 123.0f, 156.0f, 189.0f },
            { 20.0f, 20.0f, 20.0f, 20.0f, 20.0f, 15.0f, 10.0f, 20.0f, 20.0f },
            { 0.2f, 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.2f, 0.2f, 0.2f }
        },
        { // Concrete
            5,
            { 0, 2, 2, 2, 2 },
            { 140.0f, 234.0f, 380.0f, 1450.0f, 2156.0f },
            { 10.0f, 10.0f, 10.0f, 10.0f, 10.0f },
            { 0.1f, 0.2f, 0.1f, 0.05f, 0.05f }
        },
        { // Dirt
            4,
            { 2, 2, 2, 0 } ,
            { 180.0f, 300.0f, 650.0f, 2200.0f },
            { 2.0f, 2.0f, 2.0f, 1.0f },
            { 0.6f, 0.1f, 0.1f, 0.1f }
        },
        { // Grass
            3,
            { 1, 2, 0 },
            { 890.0f, 2023.0f, 3000.0f },
            { 3.5f, 2.0f, 2.0f },
            { 0.05f, 0.05f, 0.05f }
        },
        { // Hollow Wood
            4,
            { 2, 2, 2, 2},
            { 109.0f, 230.0f, 352.0f, 413.0f },
            { 10.0f, 10.0f, 10.0f, 10.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f }
        },
        { // Metal
            7,
            { 2, 2, 2, 2, 2, 2, 2 },
            { 124.0f,  218.0f,  615.0f,  1098.0f,  1250.0f,  1764.0f,  2682.0f },
            { 2.0f, 60.0f, 60.0f, 60.0f, 60.0f, 60.0f, 60.0f },
            { 1.0f, 0.80f, 0.65f, 0.50f, 0.35f, 0.20f, 0.05f }
        }
    };
};