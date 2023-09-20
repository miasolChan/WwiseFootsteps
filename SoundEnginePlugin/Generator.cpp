#include "Generator.h"

Generator::Generator()
	: m_sampleRate(0)
	, m_ShoeType(0)
	, m_SurfaceType(0)
	, m_Terrain(0)
	, m_Pace(82.0f)
	, m_Firmness(0.3f)
	, m_Steadiness(0.1f)
	, m_Automated(true)
	, m_ShoeTypeChanged(false)
	, m_SurfaceTypeChanged(false)
	, m_TerrainChanged(false)
	, m_PaceChanged(false)
	, m_FirmnessChanged(false)
	, m_SteadinessChanged(false)
	, m_AutomatedChanged(false)
{
}

Generator::~Generator()
{
}

void Generator::PrepareModel(AkUInt32 in_sampleRate)
{
	//sample rate
	m_sampleRate = in_sampleRate;
	// init nemlib classes
	HeelEnv = nemlib::CurveEnvelope(m_sampleRate, {}, {});
	BallEnv = nemlib::CurveEnvelope(m_sampleRate, {}, {});
	Noise = nemlib::WhiteNoiseGen();
	Highpass = nemlib::BiquadFilter(m_sampleRate, 1000.0f, 1.0f, 0.0f, 1);
	OutHP = nemlib::BiquadFilter(m_sampleRate, 100.0f, 1.0f, 0.0f, 1);
	OutLP = nemlib::BiquadFilter(m_sampleRate, 10000.0f, 1.0f, 0.0f, 0);
	Filters = nemlib::FilterBank(m_sampleRate, 9);
	Filters.InitialiseFilterBank(Modes[0]);
	Filters.Unmute(0.6f);
	Distortion = nemlib::DistortionProcessor(200.0f);
	CrunchBP = nemlib::BiquadFilter(m_sampleRate, 500.0f, 3.0f, 0.0f, 0);
	CrunchEnv = nemlib::CurveEnvelope(m_sampleRate, {}, {});
	SeparationDelay = nemlib::Delay(m_sampleRate, 0.02f);
	CrunchTimer = nemlib::Timer(m_sampleRate, 0.1f);
	CrunchTimer.ResumeTimer();

	//model parameters
	//ShoeType = m_pParams->RTPC.fShoe;
	//SurfaceType = m_pParams->RTPC.fSurface;
	//TerrainType = m_pParams->RTPC.fTerrain;
	//Pace = m_pParams->RTPC.fPace;
	//Steadiness = m_pParams->RTPC.fSteadiness;
	//Firmness = m_pParams->RTPC.fFirmness;
	//Automated = m_pParams->RTPC.fAutomated;

	StepTimer = nemlib::Timer(m_sampleRate, 60.0f / m_Pace);

	UpdatePaceModifiers(m_Pace);
	UpdateShoeModifiers(m_ShoeType);
	UpdateSurfaceModifiers(m_SurfaceType);
	UpdateStepEnvelope();

	if (m_Automated) {
		StepTimer.ResetTimer();
		StepTimer.ResumeTimer();
	}

	StepCounter = 0;
}


void Generator::UpdateStepEnvelope()
{
	//OutputDebugString(L"Process\n");
	//Surface
	VaryFilterBank();

	//Automated
	if (!m_Automated) {
		if (StepCounter > 0) {
			//CHECK THIS
			UpdatePaceModifiers(60.0f / StepCounter);
			StepTimer.SetTime(StepCounter);
		}
		StepCounter = 0.0f;
	}
	//Shoe Envelope
	ShoeEnvelope NewShoeEnvelope = AddVariation();
	/**
	 TerrainType：Flat Surface or Upstairs
	 */
	if (m_Terrain == 0) {

		float HeelGain = NewShoeEnvelope.HeelGain * HeelToBallRatio[0];
		float HeelAttack = (NewShoeEnvelope.HeelAttack + Surface.HeelAttack) / 1000.0f;
		float HeelDecay = (NewShoeEnvelope.HeelDecay + Surface.HeelDecay) / 1000.0f;
		float HeelSustain = NewShoeEnvelope.HeelSustain + Surface.HeelSustain + 0.05f * nemlib::Vary(m_Firmness, m_Firmness);
		float HeelRelease = (NewShoeEnvelope.HeelRelease + Surface.HeelRelease + 10.0f * nemlib::Vary(m_Firmness, 0.2f)) / 1000.0f;
		float BallGain = NewShoeEnvelope.BallGain * HeelToBallRatio[1];
		float BallAttack = (NewShoeEnvelope.BallAttack + Surface.BallAttack) / 1000.0f;
		float BallSustain = NewShoeEnvelope.BallSustain + Surface.BallSustain;
		float BallDecay = (NewShoeEnvelope.BallDecay + Surface.BallDecay) / 1000.0f;
		float BallRelease = (NewShoeEnvelope.BallRelease + Surface.BallRelease) / 1000.0f;
		float StepSeparation = NewShoeEnvelope.StepSeparation * (1.0f + RollSpeedPercentage / 10.0f) * (1.5f - 0.5f * m_Firmness) / 1000.0f;
		HeelEnv.SetValues({ 0.0f, HeelGain, HeelSustain, 0.0f });
		BallEnv.SetValues({ 0.0f, BallGain, BallSustain, 0.0f });
		HeelEnv.SetTimes({ HeelAttack, HeelDecay, HeelRelease });
		BallEnv.SetTimes({ BallAttack, BallDecay, BallRelease });
		HeelEnv.ResetEnvelope();
		BallEnv.ResetEnvelope();
		SeparationDelay.SetDelay(StepSeparation);
	}

	else {

		float BallGain = NewShoeEnvelope.BallGain * HeelToBallRatio[1];
		float BallSustain = (NewShoeEnvelope.BallSustain + Surface.BallSustain) / 1000.0f;
		float BallAttack = NewShoeEnvelope.BallAttack / 1000.0f;
		float BallDecay = (NewShoeEnvelope.BallDecay + Surface.BallDecay) / 1000.0f;
		float BallRelease = (NewShoeEnvelope.BallRelease + Surface.BallRelease) / 1000.0f;
		HeelEnv.SetValues({ 0.0f, BallGain, BallSustain, 0.0f });
		HeelEnv.SetTimes({ BallAttack, BallDecay, BallRelease });
		HeelEnv.ResetEnvelope();
	}
}

float Generator::IncrementTheModelChannel()
{
	if (m_Automated) {
		if (StepTimer.checkTime() == true) {
			UpdateStepEnvelope();
			StepTimer.SetTime(nemlib::Vary(60.0f / m_Pace, m_Steadiness));
			StepTimer.ResetTimer();
			StepTimer.ResumeTimer();
		}
	}
	else {
		StepCounter += 1.0f / (float)m_sampleRate;
	}

	if (CrunchFlag) {
		if (CrunchTimer.checkTime() == true) {
			CrunchLoop();
		}
	}

	float NoiseSample = Noise.NextSample();
	float FilteredNoise = FiltersOut * Filters.ProcessSample(NoiseSample);
	float Crunch = CrunchOut * CrunchEnv.GetNextEnvelopePoint() * CrunchBP.ProcessSample(Distortion.ProcessSample(NoiseSample));
	float HeelOut = HeelEnv.GetNextEnvelopePoint() * (FilteredNoise + Crunch);
	float BallOut = SeparationDelay.ProcessSample(Highpass.ProcessSample(BallEnv.GetNextEnvelopePoint() * (FilteredNoise + Crunch)));

	LastOut = OutLP.ProcessSample(OutHP.ProcessSample(40.0f * (HeelOut + BallOut)));

	float OutputSample = nemlib::Clamp(0.8f * LastOut, -0.5f, 0.5f);

	return OutputSample;
}

void Generator::ExcuteModel(AkReal32* pBuf, AkUInt16 in_uValidFrames)
{
	//==========Ramp Block==========
	//shoe
	if (m_ShoeTypeChanged)
	{
		m_ShoeTypeChanged = false; 
		//ParamChanged();
	}
	//surface
	if (m_SurfaceTypeChanged)
	{
		m_SurfaceTypeChanged = false;
		//ParamChanged();
	}
	//terrain
	if (m_TerrainChanged)
	{
		m_TerrainChanged = false;
		//ParamChanged();
	}
	//pace
	AkReal32 m_PaceBegin = m_Pace, m_PaceEnd = 0.0f, m_PaceStep = 0.0f;
	if (m_PaceChanged)
	{
		m_PaceEnd = m_Pace;
		m_PaceStep = ((m_PaceEnd - m_PaceBegin) / in_uValidFrames);
		m_PaceChanged = false;
	}
	//firmness
	AkReal32 m_FirmnessBegin = m_Firmness, m_FirmnessEnd = 0.0f, m_FirmnessStep = 0.0f;
	if (m_FirmnessChanged)
	{
		m_FirmnessEnd = m_Firmness;
		m_FirmnessStep = ((m_FirmnessEnd - m_FirmnessBegin) / in_uValidFrames);
		m_FirmnessChanged = false;
		//ParamChanged();
	}
	//steadiness
	AkReal32 m_SteadinessBegin = m_Steadiness, m_SteadinessEnd = 0.0f, m_SteadinessStep = 0.0f;
	if (m_SteadinessChanged)
	{
		m_SteadinessBegin = m_Steadiness;
		m_SteadinessEnd = m_Steadiness;
		m_SteadinessStep = ((m_SteadinessEnd - m_SteadinessBegin) / in_uValidFrames);
		m_SteadinessChanged = false;
		//ParamChanged();
	}
	//automated
	if (m_AutomatedChanged) m_AutomatedChanged = false;

	//
	//==========Output==========
	AkUInt16 uFramesProduced = 0;
	//OutputDebugString(L"Process\n");
	while (uFramesProduced < in_uValidFrames)
	{
		m_Pace = m_PaceBegin;
		m_Firmness = m_FirmnessBegin;
		m_Steadiness = m_SteadinessBegin;


		*pBuf++ = IncrementTheModelChannel();

		m_PaceBegin += m_PaceStep;
		m_FirmnessBegin += m_FirmnessStep;
		m_SteadinessBegin += m_SteadinessStep;

		++uFramesProduced;

	}

	m_Pace = m_PaceBegin;
	m_Firmness = m_FirmnessBegin;
	m_Steadiness = m_SteadinessBegin;

}

void Generator::SetShoeType(AkInt32 in_ShoeType)
{
	if (m_sampleRate > 0)
	{
		m_ShoeType = in_ShoeType;
		UpdateShoeModifiers(m_ShoeType);
		m_ShoeTypeChanged = true;
	}
}

void Generator::SetSurfaceType(AkInt32 in_SurfaceType)
{
	if (m_sampleRate > 0)
	{
		m_SurfaceType = in_SurfaceType;
		m_SurfaceTypeChanged = true;
	}
}

void Generator::SetTerrain(AkInt32 in_Terrain)
{
	if (m_sampleRate > 0)
	{
		m_Terrain = in_Terrain;
		//UpdateStepEnvelope();
		m_TerrainChanged = true;
	}
}

void Generator::SetPace(AkReal32 in_Pace)
{
	if (m_sampleRate > 0)
	{
		m_Pace = in_Pace;
		UpdatePaceModifiers(m_Pace);
		m_PaceChanged = true;
	}
}

void Generator::SetFirmness(AkReal32 in_Firmness)
{
	if (m_sampleRate > 0)
	{
		m_Firmness = 1.0f - in_Firmness;
		m_FirmnessChanged = true;
	}
}

void Generator::SetSteadiness(AkReal32 in_Steadiness)
{
	if (m_sampleRate > 0)
	{
		m_Steadiness = in_Steadiness;
		m_SteadinessChanged = true;
	}
}

void Generator::SetAutomeated(bool in_Automated)
{
	if (m_sampleRate > 0)
	{
		m_Automated = in_Automated;

		//UpdateStepEnvelope();
		//StepTimer.SetTime(60.0f / m_Pace);
		//StepTimer.ResetTimer();
		//StepTimer.ResumeTimer();
		m_AutomatedChanged = true;
	}
}


void Generator::UpdatePaceModifiers(float Pace)
{
	if (m_Pace < 75.0f) { // Creeping
		RollSpeedPercentage = 22.0f - (4.0f / 15.0f) * Pace;
		HeelToBallRatio[0] = 0.5f;
		HeelToBallRatio[1] = 0.4f;
	}
	else if (m_Pace < 120.0f) { // Walking
		RollSpeedPercentage = (255.0f - Pace) / 90.0f;
		HeelToBallRatio[0] = 1.0f;
		HeelToBallRatio[1] = 0.8f;
	}
	else { // Running
		RollSpeedPercentage = 1.5f;
		HeelToBallRatio[0] = 1.0f;
		HeelToBallRatio[1] = 0.63f;
	}
}

void Generator::UpdateShoeModifiers(int ShoeType)
{
	switch (ShoeType) {
	case 0:  // TRAINER
		Shoe = { 1.0f, 1.0f, 0.0f, 10.0f, 0.1f, 40.0f, 0.5f, 1.0f, 0.0f, 20.0f, 0.1f };
		break;
	case 1:  // HIGH HEEL
		Shoe = { 1.0f, 0.1f, 0.0f, 1.0f, 0.1f, 20.0f, 0.8f, 2.0f, 0.0f, 5.0f, 0.1f };
		break;
	case 2:  // OXFORD
		Shoe = { 1.0f, 0.1f, 0.0f, 3.0f, 0.1f, 40.0f, 1.0f, 1.0f, 0.2f, 5.0f, 20.0f };
		break;
	case 3: // WORK BOOT
		Shoe = { 1.0f, 1.27f, 0.0f, 21.4f, 0.1f, 40.0f, 0.429f, 12.7f, 0.0f, 37.5f, 0.1f };
		break;
	default:
		Shoe = { 1.0f, 1.0f, 0.0f, 10.0f, 0.1f, 40.0f, 0.5f, 1.0f, 0.0f, 20.0f, 0.1f };
	}
}

void Generator::UpdateSurfaceModifiers(int SurfaceType)
{
	CrunchFlag = false;
	switch (SurfaceType) {
	case 0:  // WOOD
		Filters.InitialiseFilterBank(Modes[0]);
		FiltersOut = 1.6f;
		Surface = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		break;
	case 1:  // CONCRETE
		Filters.InitialiseFilterBank(Modes[1]);
		FiltersOut = 0.8f;
		CrunchFlag = true;
		Freq1 = 1000.0f;
		Freq2 = 200.0f;
		Delay1 = 20.0f;
		Delay2 = 4.0f;
		CrunchOut = 0.1f;
		Surface = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		break;
	case 2: // DIRT
		Filters.InitialiseFilterBank(Modes[2]);
		FiltersOut = 0.1f;
		CrunchFlag = true;
		Freq1 = 200.0f;
		Freq2 = 50.0f;
		Delay1 = 20.0f;
		Delay2 = 4.0f;
		CrunchOut = 0.25f;
		Surface = { 20.0f, 0.0f, 3.0f, 20.0f, 5.0f, 0.15f, 3.0f, 20.0f };
		break;
	case 3: // GRASS
		Filters.InitialiseFilterBank(Modes[3]);
		FiltersOut = 0.1f;
		CrunchFlag = true;
		Freq1 = 1500.0f;
		Freq2 = 800.0f;
		Delay1 = 20.0f;
		Delay2 = 4.0f;
		CrunchOut = 0.005f;
		Surface = { 50.0f, 0.0f, 10.0f, 20.0f, 5.0f, 0.15f, 50.0f, 20.0f };
		break;
	case 4: // HOLLOW WOOD
		Filters.InitialiseFilterBank(Modes[4]);
		FiltersOut = 0.6f;
		Surface = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		break;
	case 5: // METAL
		Filters.InitialiseFilterBank(Modes[5]);
		FiltersOut = 0.6f;
		Surface = { 0.0f, 0.1f, 0.0f, 10.0f, 0.0f, 0.1f, 0.0f, 10.0f };
		break;
	default:
		Surface = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	}
	if (CrunchFlag) {
		CrunchLoop();
	}
}

void Generator::CrunchLoop()
{
	CrunchBP.SetFrequency((float(rand()) / float(RAND_MAX)) * (Freq1 - Freq2) + Freq1);
	CrunchBP.SetQFactor((float(rand()) / float(RAND_MAX)) * 7.0f + 3.0f);
	CrunchEnv.ResetEnvelope();
	CrunchEnv.SetValues({ 0.0f, (float(rand()) / float(RAND_MAX)) + 0.7f, 0.0f });
	CrunchEnv.SetTimes({ (float(rand()) / float(RAND_MAX)) * 0.0001f + 0.0001f, (float(rand()) / float(RAND_MAX)) * 0.0342f + 0.0102f });
	CrunchTimer.SetTime((Delay1 + (float(rand()) / float(RAND_MAX)) * (Delay1 - Delay2)) / 1000.0f);
	CrunchTimer.ResetTimer();
}

void Generator::VaryFilterBank()
{
	switch (m_SurfaceType) {
	case 0: //wood
		Filters.VaryParameters(Modes[0]);
		break;
	case 1: //concrete
		Filters.VaryParameters(Modes[1]);
		break;
	case 2: // dirt
		Filters.VaryParameters(Modes[2]);
		break;
	case 3: // grass
		Filters.VaryParameters(Modes[3]);
		break;
	case 4: // hollow wood
		Filters.VaryParameters(Modes[4]);
		break;
	case 5: // metal
		Filters.VaryParameters(Modes[5]);
		break;
	default:
		break;
	}
}

ShoeEnvelope Generator::AddVariation()
{
	ShoeEnvelope NewShoeEnvelope = {
	nemlib::Vary(Shoe.HeelGain, 0.02f),
	nemlib::Vary(Shoe.HeelAttack, 0.05f),
	nemlib::Vary(Shoe.HeelSustain, 0.01f),
	nemlib::Vary(Shoe.HeelDecay, 0.1f),
	nemlib::Vary(Shoe.HeelRelease, 0.05f),
	nemlib::Vary(Shoe.StepSeparation, 0.05f),
	nemlib::Vary(Shoe.BallGain, 0.15f),
	nemlib::Vary(Shoe.BallAttack, 0.1f),
	nemlib::Vary(Shoe.BallSustain, 0.01f),
	nemlib::Vary(Shoe.BallDecay, 0.1f),
	nemlib::Vary(Shoe.BallRelease, 0.05f)
	};
	return NewShoeEnvelope;
}

