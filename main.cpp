// WE USE RELATIVE PATHS BECAUSE YOUR REPO STRUCTURE IS DIFFERENT
#include "aml-psdk/game_sa/plugin.h"
#include "aml-psdk/game_sa/game/CWorld.h"
#include "aml-psdk/game_sa/entity/Ped.h" 

// Namespace usually provided by the SDK
using namespace plugin;

// --- MAIN LOGIC ---

void ApplyLegIK(CPed* ped) {
    if (!ped->m_pRwClump) return;

    // 1. Get Hierarchy (Using SDK helper if available, or manual)
    RpHAnimHierarchy* hier = GetAnimHierarchyFromSkinClump(ped->m_pRwClump);
    if (!hier) return;

    // 2. Bone IDs (2 = L_Thigh, 3 = L_Calf)
    int thighIdx = RpHAnimIDGetIndex(hier, 2);
    int calfIdx = RpHAnimIDGetIndex(hier, 3);

    if (thighIdx == -1 || calfIdx == -1) return;

    // 3. Get Matrices
    RwMatrix* thighMat = &hier->pNodeInfo[thighIdx].pFrame->modelingMtx;

    // 4. Raycast
    CVector startPos = { thighMat->pos.x, thighMat->pos.y, thighMat->pos.z };
    CVector endPos = startPos;
    endPos.z -= 1.2f;

    CColPoint colPoint;
    CEntity* hitEntity = nullptr;

    // 5. The SDK Raycast Function
    if (CWorld::ProcessLineOfSight(startPos, endPos, colPoint, hitEntity, true, false, false, true, false, false, false, false)) {
        float dist = startPos.z - colPoint.m_vecPoint.z;
        float upper = 0.44f;
        float lower = 0.45f;

        if (dist < (upper + lower) && dist > 0.2f) {
            float num = (upper * upper) + (lower * lower) - (dist * dist);
            float denom = 2 * upper * lower;
            float cosAng = num / denom;

            if (cosAng > 1.0f) cosAng = 1.0f;
            if (cosAng < -1.0f) cosAng = -1.0f;

            float angleRad = acos(cosAng);
            float angleDeg = angleRad * (180.0f / 3.14159f);

            // 6. Apply Rotation
            RwV3d axis = { 1.0f, 0.0f, 0.0f };
            RwFrameRotate(hier->pNodeInfo[calfIdx].pFrame, &axis, angleDeg, rwCOMBINEPRECONCAT);
            RwFrameUpdateObjects(hier->pNodeInfo[thighIdx].pFrame);
        }
    }
}

// --- ENTRY POINT ---
// Since we don't have 'mod/amlmod.h', we use a standard constructor function 
// that NDK plugins use. The SDK likely calls a function on load.

extern "C" void OnModLoad() {
    // This connects our logic to the game timer
    Events::processScriptsEvent += [] {
        CPed* player = FindPlayerPed();
        if (player) {
            ApplyLegIK(player);
        }
    };
}
