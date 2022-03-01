#pragma once

#include <vector>
#include "DynamicQuadTreeNode.hpp"

namespace Diligent
{

// Template class implementing hierarchy array, which is a quad tree indexed by
// quad tree node location
template <class T>
class HierarchyArray
{
public:
    T& operator[](const QuadTreeNodeLocation& at)
    {
        return m_data[at.level][at.horzOrder + (at.vertOrder << at.level)];
    }
    const T& operator[](const QuadTreeNodeLocation& at) const
    {
        return m_data[at.level][at.horzOrder + (at.vertOrder << at.level)];
    }

    void Resize(size_t numLevelsInHierarchy)
    {
        m_data.resize(numLevelsInHierarchy);
        if (numLevelsInHierarchy)
        {
            for (size_t level = numLevelsInHierarchy; level--;)
            {
                size_t numElementsInLevel = (size_t)1 << level;
                m_data[level].resize(numElementsInLevel * numElementsInLevel);
            }
        }
    }

    bool Empty() const
    {
        return m_data.empty();
    }

private:
    std::vector<std::vector<T>> m_data;
};

} // namespace Diligent
