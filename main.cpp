#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <jni.h>
#include <math.h>

// --- MANUAL DEFINITIONS (Skipping the broken SDK) ---

struct RwV3d { float x, y, z; };
struct RwMatrix { RwV3d right; uint32_t flags; RwV3d up; uint32_t pad1; RwV3d at; uint32_t pad2; RwV3d pos; uint32_t pad3; };
struct RwFrame { RwMatrix modelingMtx; void* parent; void* next; void* root; };
struct RpHAnimNodeInfo { int32_t nodeID; int32_t index; int32_t flags; RwFrame* pFrame; };
struct RpHAnimHierarchy { int32_t flags; int32_t numNodes; RwMatrix* pMatrixArray; void* pMatrixArrayUnaligned; RpHAnimNodeInfo* pNodeInfo; };
struct RpAtomic { void* geometry; }; 
struct RpClump { void* object; }; // Simplified

// Standard Helpers
// Function pointers we will find at runtime
void* (*GetAnimHierarchyFromSkinClump)(void* clump);
void (*RwFrameRotate)(RwFrame* frame, RwV3d* axis, float angle, int combine);
void (*RwFrameUpdateObjects)(RwFrame* frame);
void* (*FindPlayerPed)(int id);
bool (*ProcessLineOfSight)(RwV3d* start, RwV3d* end, void* colPoint, void* hitEntity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool seeThroughStuff, bool ignoreSomeObjects, bool shootThroughStuff);

// Helpers to match SDK style
#define rwCOMBINEPRECONCAT 1

IAMLer* aml = new IAMLer();
FakeLogger* logger = new FakeLogger();

MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// --- LOGIC ---

void ApplyLegIK(void* pRwClump) {
    if (!pRwClump || !GetAnimHierarchyFromSkinClump) return;

    RpHAnimHierarchy* hier = (RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(pRwClump);
    if (!hier) return;

    // Find Indices Manually
    int thighIdx = -1, calfIdx = -1;
    for(int i=0; i<hier->numNodes; i++) {
        if(hier->pNodeInfo[i].nodeID == 2) thighIdx = i; // L_Thigh
        if(hier->pNodeInfo[i].nodeID == 3) calfIdx = i;  // L_Calf
    }

    if (thighIdx == -1 || calfIdx == -1) return;

    RwFrame* thighFrame = hier->pNodeInfo[thighIdx].pFrame;
    RwFrame* calfFrame = hier->pNodeInfo[calfIdx].pFrame;
    
    RwV3d startPos = thighFrame->modelingMtx.pos;
    RwV3d endPos = startPos;
    endPos.z -= 1.2f;

    // 48 bytes for CColPoint (approx)
    char colPoint[64]; 
    void* hitEntity = nullptr;

    if (ProcessLineOfSight && ProcessLineOfSight(&startPos, &endPos, colPoint, &hitEntity, true, false, false, true, false, false, false, false)) {
        // CColPoint.point is usually at offset 0
        RwV3d* hitPoint = (RwV3d*)colPoint;
        float dist = startPos.z - hitPoint->z;
        float upper = 0.44f;
        float lower = 0.45f;

        if (dist < (upper + lower) && dist > 0.2f) {
            float num = (upper * upper) + (lower * lower) - (dist * dist);
            float denom = 2 * upper * lower;
            float cosAng = num / denom;

            if (cosAng > 1.0f) cosAng = 1.0f;
            if (cosAng < -1.0f) cosAng = -1.0f;

            float angleDeg = acos(cosAng) * (180.0f / 3.14159f);

            RwV3d axis = { 1.0f, 0.0f, 0.0f };
            if(RwFrameRotate) {
                RwFrameRotate(calfFrame, &axis, angleDeg, rwCOMBINEPRECONCAT);
                if(RwFrameUpdateObjects) RwFrameUpdateObjects(thighFrame);
            }
        }
    }
}

// Hook CTimer::Update to run every frame
DECL_HOOKv(CTimer_Update) {
    CTimer_Update(); // Run original game code

    if(FindPlayerPed) {
        // 0x140 = offset of m_pRwClump in CPed usually, but let's be safer:
        // We cast the CPed pointer to something we can read offsets from if needed
        // For now, assume SDK headers were partially right about CPed structure or just pass the pointer
        // CPed is complex, but we only need m_pRwClump.
        // On Android 2.10, m_pRwClump is at offset 0x18. Let's try to fetch it safely.
        
        void* player = FindPlayerPed(-1);
        if(player) {
             // DANGEROUS MAGIC: accessing m_pRwClump manually
             // In GTA SA Android, offset 24 (0x18) is usually m_pRwClump
             void* clump = *(void**)((uintptr_t)player + 0x18);
             ApplyLegIK(clump);
        }
    }
}

extern "C" void OnModLoad() {
    logger->SetTag("LegIK");
    
    // 1. Get Game Library Address
    void* hGame = aml->GetLibHandle("libGTASA.so");
    
    // 2. Load Function Pointers (Symbol Names are for GTA SA Android v2.00/2.10)
    GetAnimHierarchyFromSkinClump = (void*(*)(void*))aml->GetSym(hGame, "_Z29GetAnimHierarchyFromSkinClumpP7RpClump");
    RwFrameRotate = (void(*)(RwFrame*, RwV3d*, float, int))aml->GetSym(hGame, "_Z13RwFrameRotateP7RwFramePK5RwV3d19RwOpCombineType");
    RwFrameUpdateObjects = (void(*)(RwFrame*))aml->GetSym(hGame, "_Z20RwFrameUpdateObjectsP7RwFrame");
    FindPlayerPed = (void*(*)(int))aml->GetSym(hGame, "_Z13FindPlayerPedi");
    ProcessLineOfSight = (bool(*)(RwV3d*, RwV3d*, void*, void*, bool, bool, bool, bool, bool, bool, bool, bool))aml->GetSym(hGame, "_ZN6CWorld18ProcessLineOfSightERK7CVectorS2_R9CColPointRP7CEntitybbbbbbbb");

    // 3. Install Hook
    HOOK(CTimer_Update, aml->GetSym(hGame, "_ZN6CTimer6UpdateEv"));
}
