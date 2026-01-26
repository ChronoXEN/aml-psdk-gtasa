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

struct CNodeRoute
{
    int m_iRouteSize;
    CNodeAddress m_routePoints[8];
};

template <class T> struct OSArray
{
    unsigned int numAlloced;
    unsigned int numEntries;
    T *data;

    inline unsigned int& Count() { return numEntries; }
    inline unsigned int& AllocatedCount() { return numAlloced; }
    inline unsigned int AllocatedMem() { return numAlloced * sizeof(T); }
    inline T* Element(unsigned int i) { return &data[i]; }
    inline bool IsFull()
    {
        return numAlloced <= numEntries;
    }
    inline T* AllocNew()
    {
        T* container = data;
        if(IsFull())
        {
            uint32_t reallocCount = 4 * (numEntries + 1) / 3u + 3;
            container = (T*)malloc(sizeof(T) * reallocCount);
            if(data)
            {
                T* oldContainer = data;
                memcpy(container, data, sizeof(T) * numEntries);
                free(oldContainer);
            }
            numAlloced = reallocCount;
            data = container;
        }
        return &container[numEntries++];
    }
    inline void RemoveAt(unsigned int idx)
    {
        if(idx >= numEntries) return;
        memcpy(&data[idx], &data[idx+1], sizeof(T) * (numEntries - idx));
    }
    inline T& operator[](unsigned int idx)
    {
        return data[idx];
    }
};
#define TDBArray OSArray
#define SArray OSArray

#endif