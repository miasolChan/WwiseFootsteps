/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/

#include "FootstepsSource.h"
#include "../FootstepsConfig.h"

#include <AK/AkWwiseSDKVersion.h>

AK::IAkPlugin* CreateFootstepsSource(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, FootstepsSource());
}

AK::IAkPluginParam* CreateFootstepsSourceParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, FootstepsSourceParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(FootstepsSource, AkPluginTypeSource, FootstepsConfig::CompanyID, FootstepsConfig::PluginID)

FootstepsSource::FootstepsSource()
    : m_pParams(nullptr)
    , m_pAllocator(nullptr)
    , m_pContext(nullptr)
{
}

FootstepsSource::~FootstepsSource()
{
}

AKRESULT FootstepsSource::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkSourcePluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
    m_pParams = (FootstepsSourceParams*)in_pParams;
    m_pAllocator = in_pAllocator;
    m_pContext = in_pContext;

    m_durationHandler.Setup(0.1f, 0, in_rFormat.uSampleRate);

    //Prepare Model
    generator.PrepareModel(in_rFormat.uSampleRate);
    
    return AK_Success;
}

AKRESULT FootstepsSource::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT FootstepsSource::Reset()
{
    return AK_Success;
}

AKRESULT FootstepsSource::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
    out_rPluginInfo.eType = AkPluginTypeSource;
    out_rPluginInfo.bIsInPlace = true;
    out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}

void FootstepsSource::Execute(AkAudioBuffer* out_pBuffer)
{
    //m_durationHandler.SetDuration(m_pParams->RTPC.fDuration);
    m_durationHandler.ProduceBuffer(out_pBuffer);

    const AkUInt32 uNumChannels = out_pBuffer->NumChannels();

    //AkUInt16 uFramesProduced;

    out_pBuffer->uValidFrames = out_pBuffer->MaxFrames();
    m_durationHandler.SetLooping(0);
    //===========Parameter linear ramp block=============
    //HasChanged

    //shoe
    if (m_pParams->m_paramChangeHandler.HasChanged(PARAM_SHOE_ID))
    {
        generator.SetShoeType(m_pParams->RTPC.fShoeType);
    }

    //surface
    if (m_pParams->m_paramChangeHandler.HasChanged(PARAM_SURFACE_ID))
    {
        generator.SetSurfaceType(m_pParams->RTPC.fSurfaceType);
    }

    //terrain
    if (m_pParams->m_paramChangeHandler.HasChanged(PARAM_TERRAIN_ID))
    {
        generator.SetTerrain(m_pParams->RTPC.fTerrain);
    }

    //Pace
    if (m_pParams->m_paramChangeHandler.HasChanged(PARAM_PACE_ID))
    {
        generator.SetPace(m_pParams->RTPC.fPace);
    }

    //Firmness
    if (m_pParams->m_paramChangeHandler.HasChanged(PARAM_FIRMNESS_ID))
    {
        generator.SetFirmness(m_pParams->RTPC.fFirmness);
        
    }

    //Steadiness
    if (m_pParams->m_paramChangeHandler.HasChanged(PARAM_STEADINESS_ID))
    {
        generator.SetSteadiness(m_pParams->RTPC.fSteadiness);
        
    }
    
    //Automated
    if (m_pParams->m_paramChangeHandler.HasChanged(PARAM_AUTOMATED_ID))
    {
        generator.SetAutomeated(m_pParams->RTPC.fAutomated);

    }
    
     
    for (AkUInt32 i = 0; i < uNumChannels; ++i)
    {
        AkReal32* AK_RESTRICT pBuf = (AkReal32* AK_RESTRICT)out_pBuffer->GetChannel(i);
        generator.ExcuteModel(pBuf, out_pBuffer->uValidFrames);
        //uFramesProduced = 0;
        //while (uFramesProduced < out_pBuffer->uValidFrames)
        //{
        //    // Generate output here
        //    
        //    generator.Step();
        //    //
        // 
        //    float OutputSample = generator.IncrementTheModelChannel();
        //    *pBuf++ = OutputSample;
        //    ++uFramesProduced;
        //    
        //}
    }
}

AkReal32 FootstepsSource::GetDuration() const
{
    return m_durationHandler.GetDuration() * 1000.0f;
}
