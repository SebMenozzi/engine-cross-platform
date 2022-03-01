#pragma once

#include <vector>
#include "BasicTypes.h"
#include "BasicMath.hpp"
#include "HierarchyArray.hpp"
#include "DynamicQuadTreeNode.hpp"

namespace Diligent
{

// Class implementing elevation data source
class ElevationDataSource
{
public:
    // Creates data source from the specified raw data file
    ElevationDataSource(const Char* strSrcDemFile);
    virtual ~ElevationDataSource(void);

    void GetDataPtr(const Uint16*& pDataPtr, size_t& Pitch);

    // Returns minimal height of the whole terrain
    Uint16 GetGlobalMinElevation() const;

    // Returns maximal height of the whole terrain
    Uint16 GetGlobalMaxElevation() const;

    void RecomputePatchMinMaxElevations(const QuadTreeNodeLocation& pos);

    void SetOffsets(int iColOffset, int iRowOffset)
    {
        m_iColOffset = iColOffset;
        m_iRowOffset = iRowOffset;
    }
    void GetOffsets(int& iColOffset, int& iRowOffset) const
    {
        iColOffset = m_iColOffset;
        iRowOffset = m_iRowOffset;
    }

    float GetInterpolatedHeight(float fCol, float fRow, int iStep = 1) const;

    float3 ComputeSurfaceNormal(float fCol, float fRow, float fSampleSpacing, float fHeightScale, int iStep = 1) const;

    unsigned int GetNumCols() const { return m_iNumCols; }
    unsigned int GetNumRows() const { return m_iNumRows; }

private:
    inline Uint16  GetElevSample(Int32 i, Int32 j) const;
    inline Uint16& GetElevSample(Int32 i, Int32 j);

    // Calculates min/max elevations for all patches in the tree
    void CalculateMinMaxElevations();

    // Hierarchy array storing minimal and maximal heights for quad tree nodes
    HierarchyArray<std::pair<Uint16, Uint16>> m_MinMaxElevation;

    int m_iNumLevels;
    int m_iPatchSize;
    int m_iColOffset, m_iRowOffset;

    // The whole terrain height map
    std::vector<Uint16> m_TheHeightMap;
    Uint32              m_iNumCols, m_iNumRows, m_iStride;
};

} // namespace Diligent
