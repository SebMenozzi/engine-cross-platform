#pragma once

#include <memory>
#include <cassert>

namespace Diligent
{
    // Structure describing quad tree node location
    struct QuadTreeNodeLocation
    {
        // Position in a tree
        int horzOrder;
        int vertOrder;
        int level;

        QuadTreeNodeLocation(int h, int v, int l) :
            horzOrder(h), vertOrder(v), level(l)
        {
            VERIFY_EXPR(h < (1 << l));
            VERIFY_EXPR(v < (1 << l));
        }
        QuadTreeNodeLocation() :
            horzOrder(0), vertOrder(0), level(0)
        {}

        // Gets location of a child
        inline friend QuadTreeNodeLocation GetChildLocation(
            const QuadTreeNodeLocation& parent,
            unsigned int siblingOrder
        )
        {
            VERIFY_EXPR(siblingOrder >= 0 && siblingOrder < 4);

            return QuadTreeNodeLocation(
                parent.horzOrder * 2 + (siblingOrder & 1),
                parent.vertOrder * 2 + (siblingOrder >> 1),
                parent.level + 1
            );
        }

        // Gets location of a parent
        inline friend QuadTreeNodeLocation GetParentLocation(const QuadTreeNodeLocation& node)
        {
            assert(node.level > 0);

            return QuadTreeNodeLocation(node.horzOrder / 2, node.vertOrder / 2, node.level - 1);
        }
    };

    // Base class for iterators traversing the quad tree
    class HierarchyIteratorBase
    {
        public:
            operator const QuadTreeNodeLocation&() const
            {
                return current_;
            }

            int Level() const 
            { 
                return current_.level;
            }

            int Horz() const 
            {
                return current_.horzOrder;
            }

            int Vert() const
            {
                return current_.vertOrder;
            }

        protected:
            QuadTreeNodeLocation current_;
            int currentLevelSize_;
    };

    // Iterator for recursively traversing the quad tree starting from the root up to the specified level
    class HierarchyIterator : public HierarchyIteratorBase
    {
        public:
            HierarchyIterator(int nLevels) :
                nLevels_(nLevels)
            {
                currentLevelSize_ = 1;
            }

            bool IsValid() const
            {
                return current_.level < nLevels_;
            }

            void Next()
            {
                if (++current_.horzOrder == currentLevelSize_)
                {
                    current_.horzOrder = 0;
                    if (++current_.vertOrder == currentLevelSize_)
                    {
                        current_.vertOrder = 0;
                        currentLevelSize_  = 1 << ++current_.level;
                    }
                }
            }

        private:
            int nLevels_;
    };

    // Iterator for recursively traversing the quad tree starting from the specified level up to the root
    class HierarchyReverseIterator : public HierarchyIteratorBase
    {
        public:
            HierarchyReverseIterator(int nLevels)
            {
                current_.level    = nLevels - 1;
                currentLevelSize_ = 1 << current_.level;
            }

            bool IsValid() const
            { 
                return current_.level >= 0; 
            }

            void Next()
            {
                if (++current_.horzOrder == currentLevelSize_)
                {
                    current_.horzOrder = 0;

                    if (++current_.vertOrder == currentLevelSize_)
                    {
                        current_.vertOrder = 0;
                        currentLevelSize_  = 1 << --current_.level;
                    }
                }
            }
    };

    // Template class for the node of a dynamic quad tree
    template <typename NodeDataType>
    class DynamicQuadTreeNode
    {
        public:
            DynamicQuadTreeNode() :
                pAncestor_(NULL)
            {}

            NodeDataType& GetData()
            {
                return data_;
            }

            const NodeDataType& GetData() const 
            {
                return data_;
            }

            DynamicQuadTreeNode* GetAncestor() const 
            {
                return pAncestor_;
            }

            void GetDescendants(
                const DynamicQuadTreeNode*& LBDescendant,
                const DynamicQuadTreeNode*& RBDescendant,
                const DynamicQuadTreeNode*& LTDescendant,
                const DynamicQuadTreeNode*& RTDescendant
            ) const
            {
                LBDescendant = pLBDescendant_.get();
                RBDescendant = pRBDescendant_.get();
                LTDescendant = pLTDescendant_.get();
                RTDescendant = pRTDescendant_.get();
            }

            void GetDescendants(
                DynamicQuadTreeNode*& LBDescendant,
                DynamicQuadTreeNode*& RBDescendant,
                DynamicQuadTreeNode*& LTDescendant,
                DynamicQuadTreeNode*& RTDescendant
            )
            {
                LBDescendant = pLBDescendant_.get();
                RBDescendant = pRBDescendant_.get();
                LTDescendant = pLTDescendant_.get();
                RTDescendant = pRTDescendant_.get();
            }

            typedef std::unique_ptr<DynamicQuadTreeNode<NodeDataType>> AutoPtrType;
            // Attahes specified descendants to the tree
            void CreateDescendants(AutoPtrType pLBDescendant,
                                AutoPtrType pRBDescendant,
                                AutoPtrType pLTDescendant,
                                AutoPtrType pRTDescendant);
            // Creates descendants UNATTACHED to the tree
            void CreateFloatingDescendants(AutoPtrType& pLBDescendant,
                                        AutoPtrType& pRBDescendant,
                                        AutoPtrType& pLTDescendant,
                                        AutoPtrType& pRTDescendant);
            // Destroys ALL descendants for the node
            void DestroyDescendants();

            const QuadTreeNodeLocation& GetPos() const { return pos_; }

            void SetPos(const QuadTreeNodeLocation& pos) { pos_ = pos; }

        private:
            DynamicQuadTreeNode(DynamicQuadTreeNode* pAncestor, int iSiblingOrder)
                : pAncestor_(pAncestor), pos_(GetChildLocation(pAncestor->pos_, iSiblingOrder))
            {}

            NodeDataType data_;

            std::unique_ptr<DynamicQuadTreeNode> pLBDescendant_;
            std::unique_ptr<DynamicQuadTreeNode> pRBDescendant_;
            std::unique_ptr<DynamicQuadTreeNode> pLTDescendant_;
            std::unique_ptr<DynamicQuadTreeNode> pRTDescendant_;
            DynamicQuadTreeNode* pAncestor_;

            QuadTreeNodeLocation pos_;
    };

    template <typename NodeDataType>
    void DynamicQuadTreeNode<NodeDataType>::CreateFloatingDescendants(
        AutoPtrType& pLBDescendant,
        AutoPtrType& pRBDescendant,
        AutoPtrType& pLTDescendant,
        AutoPtrType& pRTDescendant
    )
    {
        pLBDescendant.reset(new DynamicQuadTreeNode<NodeDataType>(this, 0));
        pRBDescendant.reset(new DynamicQuadTreeNode<NodeDataType>(this, 1));
        pLTDescendant.reset(new DynamicQuadTreeNode<NodeDataType>(this, 2));
        pRTDescendant.reset(new DynamicQuadTreeNode<NodeDataType>(this, 3));
    }

    template <typename NodeDataType>
    void DynamicQuadTreeNode<NodeDataType>::CreateDescendants(AutoPtrType pLBDescendant,
                                                            AutoPtrType pRBDescendant,
                                                            AutoPtrType pLTDescendant,
                                                            AutoPtrType pRTDescendant)
    {
        assert(!pLBDescendant_.get());
        assert(!pRBDescendant_.get());
        assert(!pLTDescendant_.get());
        assert(!pRTDescendant_.get());

        pLBDescendant_ = pLBDescendant;
        pRBDescendant_ = pRBDescendant;
        pLTDescendant_ = pLTDescendant;
        pRTDescendant_ = pRTDescendant;
    }

    template <typename NodeDataType>
    void DynamicQuadTreeNode<NodeDataType>::DestroyDescendants()
    {
        if (pLBDescendant_.get())
            pLBDescendant_->DestroyDescendants();
        if (pRBDescendant_.get())
            pRBDescendant_->DestroyDescendants();
        if (pLTDescendant_.get())
            pLTDescendant_->DestroyDescendants();
        if (pRTDescendant_.get())
            pRTDescendant_->DestroyDescendants();

        pLBDescendant_.reset();
        pRBDescendant_.reset();
        pLTDescendant_.reset();
        pRTDescendant_.reset();
    }
}
