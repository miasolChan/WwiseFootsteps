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

#include "FootstepsSourceParams.h"

#include <AK/Tools/Common/AkBankReadHelpers.h>

FootstepsSourceParams::FootstepsSourceParams()
{
}

FootstepsSourceParams::~FootstepsSourceParams()
{
}

FootstepsSourceParams::FootstepsSourceParams(const FootstepsSourceParams& in_rParams)
{
    RTPC = in_rParams.RTPC;
    NonRTPC = in_rParams.NonRTPC;
    m_paramChangeHandler.SetAllParamChanges();
}

AK::IAkPluginParam* FootstepsSourceParams::Clone(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, FootstepsSourceParams(*this));
}

AKRESULT FootstepsSourceParams::Init(AK::IAkPluginMemAlloc* in_pAllocator, const void* in_pParamsBlock, AkUInt32 in_ulBlockSize)
{
    if (in_ulBlockSize == 0)
    {
        // Initialize default parameters here
        //RTPC.fDuration = 0.0f;
        RTPC.fShoeType = 0;
        RTPC.fSurfaceType = 0;
        RTPC.fTerrain = 0;
        RTPC.fPace = 60;
        RTPC.fFirmness = 0;
        RTPC.fSteadiness = 0.5;
        RTPC.fAutomated = false;
        m_paramChangeHandler.SetAllParamChanges();
        return AK_Success;
    }

    return SetParamsBlock(in_pParamsBlock, in_ulBlockSize);
}

AKRESULT FootstepsSourceParams::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT FootstepsSourceParams::SetParamsBlock(const void* in_pParamsBlock, AkUInt32 in_ulBlockSize)
{
    AKRESULT eResult = AK_Success;
    AkUInt8* pParamsBlock = (AkUInt8*)in_pParamsBlock;

    // Read bank data here
    //RTPC.fDuration = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.fShoeType = READBANKDATA(AkUInt32, pParamsBlock, in_ulBlockSize);
    RTPC.fSurfaceType = READBANKDATA(AkUInt32, pParamsBlock, in_ulBlockSize);
    RTPC.fTerrain = READBANKDATA(AkUInt32, pParamsBlock, in_ulBlockSize);
    RTPC.fPace = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.fFirmness = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.fSteadiness = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.fAutomated = READBANKDATA(bool, pParamsBlock, in_ulBlockSize);

    CHECKBANKDATASIZE(in_ulBlockSize, eResult);
    m_paramChangeHandler.SetAllParamChanges();

    return eResult;
}

AKRESULT FootstepsSourceParams::SetParam(AkPluginParamID in_paramID, const void* in_pValue, AkUInt32 in_ulParamSize)
{
    AKRESULT eResult = AK_Success;

    // Handle parameter change here
    AkReal32 fval;
    switch (in_paramID)
    {
    case PARAM_SHOE_ID:
        fval = *((AkReal32*)in_pValue);
        RTPC.fShoeType = (int)fval;
        m_paramChangeHandler.SetParamChange(PARAM_SHOE_ID);
        break;
    case PARAM_SURFACE_ID:
        fval = *((AkReal32*)in_pValue);
        RTPC.fSurfaceType = (int)fval;
        m_paramChangeHandler.SetParamChange(PARAM_SURFACE_ID);
        break;
    case PARAM_TERRAIN_ID:
        fval = *((AkReal32*)in_pValue);
        RTPC.fTerrain = (int)fval;
        m_paramChangeHandler.SetParamChange(PARAM_TERRAIN_ID);
        break;
    case PARAM_PACE_ID:
        RTPC.fPace = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_PACE_ID);
        break;
    case PARAM_FIRMNESS_ID:
        RTPC.fFirmness = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_FIRMNESS_ID);
        break;
    case PARAM_STEADINESS_ID:
        RTPC.fSteadiness = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_STEADINESS_ID);
        break;
    case PARAM_AUTOMATED_ID:
        fval = *((AkReal32*)in_pValue);
        RTPC.fAutomated = (bool)fval;
        m_paramChangeHandler.SetParamChange(PARAM_AUTOMATED_ID);
        break;
    default:
        eResult = AK_InvalidParameter;
        break;
    }

    return eResult;
}
