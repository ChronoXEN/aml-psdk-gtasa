#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <jni.h>
#include <math.h>

// --- DEFINITIONS ---
struct RwV3d { float x, y, z; };
struct RwMatrix { RwV3d right; uint32_t flags; RwV3d up; uint32_t pad1; RwV3d at; uint32_t pad2; RwV3d pos; uint32_t pad3; };
struct RwFrame { RwMatrix modelingMtx; void* parent; void* next; void* root; };
struct RpHAnimNodeInfo { int32_t nodeID; int32_t index; int32_t flags; RwFrame* pFrame; };
struct RpHAnimHierarchy { int32_t flags; int32_t numNodes; RwMatrix* pMatrixArray; void* pMatrixArrayUnaligned; RpHAnimNodeInfo* pNodeInfo; };
struct RwObject { uint8_t type; uint8_t subType; uint8_t flags; uint8_t privateFlags; void* parent; };

// --- FUNCTION POINTERS ---
void* (*GetAnimHierarchyFromSkinClump)(void* clump) = nullptr;
void (*RwFrameRotate)(RwFrame* frame, RwV3d* axis, float angle, int combine) = nullptr;
void (*RwFrameUpdateObjects)(RwFrame* frame) = nullptr;

// Constants
#define rwCOMBINEREPLACE 0 
#define rwCOMBINEPRECONCAT 1

IAMLer* aml = new IAMLer();
FakeLogger* logger = new FakeLogger();
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// --- MEMORY SCANNER ---
// This finds the 3D Model pointer automatically, no matter what version of GTA you have.
void* FindClumpInPed(void* pedPtr) {
    if(!pedPtr) return nullptr;
    
    // Scan the first 50 pointers inside the Ped class
    uintptr_t* scan = (uintptr_t*)pedPtr;
    for(int i = 0; i < 50; i++) {
        void* candidate = (void*)scan[i];
        
        // Basic Validity Check: Is it a valid pointer?
        // (In Android, valid heap pointers usually start with 0xB or 0x7)
        if(candidate == nullptr || (uintptr_t)candidate < 0x100000) continue;

        // Check if it looks like a RwObject (Type 2 = Clump)
        // We use a safe try/catch equivalent by hoping we don't segfault (AML handles some of this)
        // But scanning is generally safe-ish on valid pointers.
        RwObject* obj = (RwObject*)candidate;
        if(obj->type == 2 && obj->subType == 1) { // 2 = rpCLUMP
             return candidate;
        }
    }
    return nullptr;
}

// --- LOGIC ---
void ApplyExorcist(void* pRwClump) {
    if (!pRwClump || !GetAnimHierarchyFromSkinClump || !RwFrameRotate) return;

    RpHAnimHierarchy* hier = (RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(pRwClump);
    if (!hier) return;

    // Find HEAD (Bone ID 5)
    int headIdx = -1;
    for(int i=0; i<hier->numNodes; i++) {
        if(hier->pNodeInfo[i].nodeID == 5) { // 5 = HEAD
            headIdx = i;
            break;
        }
    }

    if (headIdx == -1) return;

    // FORCE ROTATION: 180 Degrees (Backwards)
    RwFrame* headFrame = hier->pNodeInfo[headIdx].pFrame;
    RwV3d axis = { 0.0f, 0.0f, 1.0f }; // Z-Axis (Twist)
    float angle = 180.0f; 

    // REPLACE animation
    RwFrameRotate(headFrame, &axis, angle, rwCOMBINEREPLACE);
    if(RwFrameUpdateObjects) RwFrameUpdateObjects(headFrame);
}

// --- RENDER HOOK ---
DECL_HOOKv(CPed_Render, void* self) {
    CPed_Render(self); 
    
    if(self) {
        // Use the Scanner instead of a fixed offset
        void* clump = FindClumpInPed(self);
        if(clump) ApplyExorcist(clump);
    }
}

extern "C" void OnModLoad() {
    logger->SetTag("LegIK");
    void* hGame = aml->GetLibHandle("libGTASA.so");
    
    // Look up Symbols
    GetAnimHierarchyFromSkinClump = (void*(*)(void*))aml->GetSym(hGame, "_Z29GetAnimHierarchyFromSkinClumpP7RpClump");
    RwFrameRotate = (void(*)(RwFrame*, RwV3d*, float, int))aml->GetSym(hGame, "_Z13RwFrameRotateP7RwFramePK5RwV3d19RwOpCombineType");
    RwFrameUpdateObjects = (void(*)(RwFrame*))aml->GetSym(hGame, "_Z20RwFrameUpdateObjectsP7RwFrame");

    // Hook
    HOOK(CPed_Render, aml->GetSym(hGame, "_ZN4CPed6RenderEv"));
}
