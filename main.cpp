#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <jni.h>
#include <math.h>

// --- 1. MEMORY STRUCTURES (The Map) ---
struct RwV3d { float x, y, z; };
struct RwMatrix { 
    RwV3d right; uint32_t flags; 
    RwV3d up;    uint32_t pad1; 
    RwV3d at;    uint32_t pad2; 
    RwV3d pos;   uint32_t pad3; 
};
struct RwFrame { RwMatrix modelingMtx; void* parent; void* next; void* root; };
struct RpHAnimNodeInfo { int32_t nodeID; int32_t index; int32_t flags; RwFrame* pFrame; };
struct RpHAnimHierarchy { int32_t flags; int32_t numNodes; RwMatrix* pMatrixArray; void* pMatrixArrayUnaligned; RpHAnimNodeInfo* pNodeInfo; };

// --- 2. HELPERS ---

// Helper to calculate specific bone rotation without game functions
void RotateMatrixX(RwMatrix* matrix, float angleDeg) {
    float rad = angleDeg * (3.14159f / 180.0f);
    float c = cosf(rad);
    float s = sinf(rad);

    // Rotate the UP and AT vectors around the RIGHT vector (X-Axis)
    // This is manual matrix math
    RwV3d oldUp = matrix->up;
    RwV3d oldAt = matrix->at;

    matrix->up.x = oldUp.x; // X stays same
    matrix->up.y = oldUp.y * c - oldUp.z * s;
    matrix->up.z = oldUp.y * s + oldUp.z * c;

    matrix->at.x = oldAt.x; // X stays same
    matrix->at.y = oldAt.y * c - oldAt.z * s;
    matrix->at.z = oldAt.y * s + oldAt.z * c;
    
    // Mark as dirty (usually flag 0x2 or similar, but just editing values works for rendering)
}

// Manual hierarchy finder (Traverses pointers)
RpHAnimHierarchy* GetHierarchyManual(void* clump) {
    if(!clump) return nullptr;
    
    // In GTA SA, Clump -> AtomicList -> Atomic -> Geometry -> Skin -> Hierarchy
    // We will take a shortcut. The Hierarchy is often linked in the Atomic's extension.
    // But since that's hard to hardcode, let's look for the standard pointer chain.
    
    // POINTER CHAIN FOR v2.00 (Approximation)
    // Clump + 0x18 -> Atomic List
    // Atomic + 0x18 -> Geometry
    // Geometry + 0x28 (variable) -> Skin
    
    // IF THIS FAILS, WE ABORT.
    // For now, let's rely on the symbol strictly for this one function 
    // because manual traversal is crash-prone without exact v2.00 offsets.
    return nullptr; 
}

// --- 3. MOD SETUP ---

IAMLer* aml = new IAMLer();
FakeLogger* logger = new FakeLogger();
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// Function pointer for the one function we really need
void* (*GetAnimHierarchyFromSkinClump)(void* clump) = nullptr;


// --- 4. THE LOGIC ---

void ApplyManualBend(void* pRwClump) {
    if (!pRwClump || !GetAnimHierarchyFromSkinClump) return;

    RpHAnimHierarchy* hier = (RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(pRwClump);
    if (!hier) return;

    // Find Left Calf (ID 3)
    int calfIdx = -1;
    // Iterate manually through the array (safe)
    for(int i=0; i<hier->numNodes; i++) {
        if(hier->pNodeInfo[i].nodeID == 3) {
            calfIdx = i;
            break;
        }
    }

    if (calfIdx == -1) return;

    RwFrame* calfFrame = hier->pNodeInfo[calfIdx].pFrame;

    // FORCE BEND: 90 Degrees
    // We are editing the matrix directly. The game cannot "overwrite" this 
    // because we are doing it right before it draws.
    RotateMatrixX(&calfFrame->modelingMtx, 90.0f);
}

// Hooking CPed::PreRender (Offset 0x18 for Clump is consistent across versions)
DECL_HOOKv(CPed_PreRender, void* self) {
    CPed_PreRender(self); // Let game calculate animation
    
    if(self) {
        // v2.00 Clump Offset check
        void* clump = *(void**)((uintptr_t)self + 0x18);
        if(clump) ApplyManualBend(clump);
    }
}

extern "C" void OnModLoad() {
    logger->SetTag("LegIK");
    void* hGame = aml->GetLibHandle("libGTASA.so");
    
    // v2.00 Symbol Check
    // If this symbol fails, we are dead in the water anyway.
    GetAnimHierarchyFromSkinClump = (void*(*)(void*))aml->GetSym(hGame, "_Z29GetAnimHierarchyFromSkinClumpP7RpClump");

    // Hook PreRender
    HOOK(CPed_PreRender, aml->GetSym(hGame, "_ZN4CPed9PreRenderEv"));
}
