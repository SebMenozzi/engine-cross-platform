#pragma once

#include <vector>

#include "RenderDevice.h"
#include "DeviceContext.h"
#include "Buffer.h"
#include "Texture.h"
#include "BufferView.h"
#include "TextureView.h"
#include "GraphicsTypes.h"
#include "RefCntAutoPtr.hpp"
#include "RenderStateNotationLoader.h"

#include "AdvancedMath.hpp"

namespace Diligent
{

// Include structures in Diligent namespace
#include "HostSharedTerrainStructs.fxh"

// Structure describing terrain rendering parameters
struct RenderingParams
{
    TerrainAttribs m_TerrainAttribs;

    enum TEXTURING_MODE
    {
        TM_HEIGHT_BASED     = 0,
        TM_MATERIAL_MASK    = 1,
        TM_MATERIAL_MASK_NM = 2
    };

    // Patch rendering params
    TEXTURING_MODE m_TexturingMode  = TM_MATERIAL_MASK_NM;
    int            m_iRingDimension = 65;
    int            m_iNumRings      = 15;

    int            m_iNumShadowCascades         = 6;
    int            m_bBestCascadeSearch         = 1;
    int            m_FixedShadowFilterSize      = 5;
    bool           m_FilterAcrossShadowCascades = true;
    int            m_iColOffset                 = 1356;
    int            m_iRowOffset                 = 924;
    TEXTURE_FORMAT DstRTVFormat                 = TEX_FORMAT_R11G11B10_FLOAT;
    TEXTURE_FORMAT ShadowMapFormat              = TEX_FORMAT_D32_FLOAT;
};

struct RingSectorMesh
{
    RefCntAutoPtr<IBuffer> pIndBuff;
    Uint32                 uiNumIndices;
    BoundBox               BndBox;
    RingSectorMesh() :
        uiNumIndices(0) {}
};

// This class renders the adaptive model using DX11 API
class EarthHemsiphere
{
public:
    EarthHemsiphere(void) :
        m_ValidShaders(0) {}

    // clang-format off
    EarthHemsiphere             (const EarthHemsiphere&) = delete;
    EarthHemsiphere& operator = (const EarthHemsiphere&) = delete;
    EarthHemsiphere             (EarthHemsiphere&&)      = delete;
    EarthHemsiphere& operator = (EarthHemsiphere&&)      = delete;
    // clang-format on

    // Renders the model
    void Render(IDeviceContext*        pContext,
                const RenderingParams& NewParams,
                const float3&          vCameraPosition,
                const float4x4&        CameraViewProjMatrix,
                ITextureView*          pShadowMapSRV,
                ITextureView*          pPrecomputedNetDensitySRV,
                ITextureView*          pAmbientSkylightSRV,
                bool                   bZOnlyPass);

    // Creates device resources
    void Create(class ElevationDataSource* pDataSource,
                const RenderingParams&     Params,
                IRenderDevice*             pDevice,
                IDeviceContext*            pContext,
                const char*                MaterialMaskPath,
                const char*                TileTexturePath[],
                const char*                TileNormalMapPath[],
                IBuffer*                   pcbCameraAttribs,
                IBuffer*                   pcbLightAttribs,
                IBuffer*                   pcMediaScatteringParams);

    enum
    {
        NUM_TILE_TEXTURES = 1 + 4
    }; // One base material + 4 masked materials

private:
    void RenderNormalMap(IRenderDevice*  pd3dDevice,
                         IDeviceContext* pd3dImmediateContext,
                         const Uint16*   pHeightMap,
                         size_t          HeightMapPitch,
                         int             HeightMapDim,
                         ITexture*       ptex2DNormalMap);

    RenderingParams m_Params;

    RefCntAutoPtr<IRenderDevice> m_pDevice;

    RefCntAutoPtr<IRenderStateNotationLoader> m_pRSNLoader;

    RefCntAutoPtr<IBuffer>      m_pcbTerrainAttribs;
    RefCntAutoPtr<IBuffer>      m_pVertBuff;
    RefCntAutoPtr<ITextureView> m_ptex2DNormalMapSRV, m_ptex2DMtrlMaskSRV;

    RefCntAutoPtr<ITextureView> m_ptex2DTilesSRV[NUM_TILE_TEXTURES];
    RefCntAutoPtr<ITextureView> m_ptex2DTilNormalMapsSRV[NUM_TILE_TEXTURES];

    RefCntAutoPtr<IResourceMapping> m_pResMapping;

    RefCntAutoPtr<IPipelineState>         m_pHemisphereZOnlyPSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pHemisphereZOnlySRB;
    RefCntAutoPtr<IPipelineState>         m_pHemispherePSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pHemisphereSRB;
    RefCntAutoPtr<ISampler>               m_pComparisonSampler;

    std::vector<RingSectorMesh> m_SphereMeshes;

    Uint32 m_ValidShaders;
};

} // namespace Diligent
