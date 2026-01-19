#ifndef __AML_PSDK_GTALISTS_H
#define __AML_PSDK_GTALISTS_H

struct CPtrNode
{
    void *m_pVoid;
    CPtrNode *m_pNext;
};

struct CPtrNodeSingleLink : CPtrNode
{
};

struct CPtrNodeDoubleLink : CPtrNode
{
    CPtrNodeDoubleLink *m_pPrevious;
};

struct CPtrList
{
    CPtrNode *m_pHead;
};

struct CPtrListDoubleLink : CPtrList
{
};

struct CPtrListSingleLink : CPtrList
{
};

struct CSector
{
    CPtrListSingleLink m_buildingPtrListArray;
    CPtrListDoubleLink m_dummyPtrListArray;
};

struct CRepeatSector
{
    CPtrListDoubleLink m_ptrListArray[3];
};

struct CEntryInfoNode
{
    CPtrListDoubleLink *m_pList;
    CPtrNodeDoubleLink *m_pNode;
    union
    {
        CSector *m_pStandardSector;
        CRepeatSector *m_pRepeatSector;
    };
    CEntryInfoNode *m_pPrevious;
    CEntryInfoNode *m_pNext;
};

struct CEntryInfoList
{
    CEntryInfoNode *m_pHead;
};

struct CNodeAddress
{
    short m_nAreaId;
    short m_nNodeId;

    inline CNodeAddress() {
        Clear();
    }

    inline CNodeAddress(short areaId, short nodeId) {
        Set(areaId, nodeId);
    }

    inline void Set(short areaId, short nodeId) {
        m_nAreaId = areaId;
        m_nNodeId = nodeId;
    }

    inline bool IsEmpty() const {
        return m_nAreaId == -1 || m_nNodeId == -1;
    }

    inline void Clear() {
        m_nAreaId = -1;
        m_nNodeId = -1;
    }

    inline bool operator==(CNodeAddress const &rhs) const {
        return m_nAreaId == rhs.m_nAreaId && m_nNodeId == rhs.m_nNodeId;
    }

    inline bool operator!=(CNodeAddress const &rhs) const {
        return m_nAreaId != rhs.m_nAreaId || m_nNodeId != rhs.m_nNodeId;
    }
};

struct CCarPathLinkAddress
{
    short m_nCarPathLinkId : 10;
    short m_nAreaId : 6;
};

#endif
