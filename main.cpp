#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>

// Include GTA SA Game Classes (AML SDK Structure)
#include <game_sa/common.h>
#include <game_sa/CWorld.h>
#include <game_sa/CPed.h>
#include <game_sa/Rw/RwHelper.h> // For RwFrame operations

// Config for your mod
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// --- HELPER FUNCTIONS ---

// Helper to get bone matrix
RwMatrix* GetBoneMatrix(CPed* ped, int boneID) {
    if(!ped->m_pRwClump) return nullptr;
    
    // In AML SDK, RpHAnimHierarchy is usually accessed via RpSkin
    RpHAnimHierarchy* hier = GetAnimHierarchyFromSkinClump(ped->m_pRwClump);
    if(!hier) return nullptr;

    int idx = RpHAnimIDGetIndex(hier, boneID);
    if(idx == -1) return nullptr;

    return &hier->pNodeInfo[idx].pFrame->modelingMtx;
}

// The Main IK Logic
void ApplyLegIK(CPed* ped) {
    // 1. Raycast Setup
    // Bone IDs: 2=L_Thigh, 3=L_Calf, 22=R_Thigh, 23=R_Calf
    // Let's try Left Leg (Thigh = 2)
    RwMatrix* thighMat = GetBoneMatrix(ped, 2);
    
    if(!thighMat) return;

    CVector startPos = thighMat->pos;
    CVector endPos = startPos;
    endPos.z -= 1.2f; // Raycast down

    CColPoint colPoint;
    CEntity* hitEntity = nullptr;

    // 2. Process Line of Sight (Raycast)
    if (CWorld::ProcessLineOfSight(&startPos, &endPos, &colPoint, &hitEntity, true, false, false, true, false, false, false, false)) {
        
        float dist = startPos.z - colPoint.m_vecPoint.z;
        
        // 3. Math (Law of Cosines)
        float upper = 0.44f;
        float lower = 0.45f;
        
        if(dist < (upper + lower) && dist > 0.2f) {
            float num = (upper*upper) + (lower*lower) - (dist*dist);
            float denom = 2 * upper * lower;
            float cosAng = num / denom;

            // Clamp
            if(cosAng > 1.0f) cosAng = 1.0f;
            if(cosAng < -1.0f) cosAng = -1.0f;

            float angleRad = acos(cosAng);
            float angleDeg = angleRad * (180.0f / 3.14159f);

            // 4. Apply Rotation (AML helper might differ, using raw RW if needed)
            // Note: In AML you might need to manually rotate the matrix if RwFrameRotate isn't exposed directly
            // But usually, you can access the frame from the hierarchy:
            
            RpHAnimHierarchy* hier = GetAnimHierarchyFromSkinClump(ped->m_pRwClump);
            int calfIdx = RpHAnimIDGetIndex(hier, 3); // Left Calf
            RwFrame* calfFrame = hier->pNodeInfo[calfIdx].pFrame;

            RwV3d axis = {1.0f, 0.0f, 0.0f};
            RwFrameRotate(calfFrame, &axis, angleDeg, rwCOMBINEPRECONCAT);
            
            // Update
            RwFrameUpdateObjects(hier->pNodeInfo[RpHAnimIDGetIndex(hier, 2)].pFrame);
        }
    }
}

// --- HOOKS ---

// Hooking CTimer::Update is the standard way to run code every frame in AML
DECL_HOOKv(CTimer_Update)
{
    CTimer_Update(); // Call original function first
    
    // Get Player
    CPed* player = (CPed*)FindPlayerPed(-1);
    
    if(player && player->m_pRwClump) {
        ApplyLegIK(player);
    }
}

// --- MOD ENTRY POINT ---
ON_MOD_LOAD()
{
    logger->SetTag("LegIK");
    logger->Info("LegIK Mod Loaded!");

    // Install the hook
    HOOK(CTimer_Update, aml->GetSym(NULL, "_ZN6CTimer6UpdateEv")); 
    // Note: "_ZN6CTimer6UpdateEv" is the symbol for CTimer::Update in the Android lib
}
