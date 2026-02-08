// --- FAKE AMLMOD.H (Since you are missing the file) ---
#pragma once

// Define the macros the SDK needs
#define MYMOD(_guid, _name, _ver, _author) \
    const char* g_szGUID = #_guid; \
    const char* g_szName = #_name; \
    const char* g_szVersion = #_ver; \
    const char* g_szAuthor = #_author;

#define MYMODCFG(_guid, _name, _ver, _author) MYMOD(_guid, _name, _ver, _author)

#define DECL_HOOKv(_name, ...) void _name(__VA_ARGS__)
// -----------------------------------------------------

// Now include the game files you actually have
#include "aml-psdk/game_sa/plugin.h"
#include "aml-psdk/game_sa/game/CWorld.h"
#include "aml-psdk/game_sa/entity/Ped.h"
#include "aml-psdk/game_sa/game/RenderWare.h" // Added for RwFrame

using namespace plugin;

// CONFIG: Name your mod here
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// --- HELPERS ---

// Helper to find hierarchy manually
RpHAnimHierarchy* GetHierarchy(RpClump* clump) {
    if (!clump) return nullptr;
    RpHAnimHierarchy* hier = nullptr;
    
    // Manual search for the hierarchy
    RpClumpForAllAtomics(clump, [](RpAtomic* atomic, void* data) {
        if (RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic))) {
            *(RpHAnimHierarchy**)data = RpSkinAtomicGetHAnimHierarchy(atomic);
            return (RpAtomic*)nullptr; 
        }
        return atomic;
    }, &hier);
    
    return hier;
}

// --- MAIN IK LOGIC ---

void ApplyLegIK(CPed* ped) {
    if (!ped->m_pRwClump) return;

    // 1. Get Hierarchy
    RpHAnimHierarchy* hier = GetHierarchy(ped->m_pRwClump);
    if (!hier) return;

    // 2. Bone IDs (2 = L_Thigh, 3 = L_Calf)
    // Note: We use raw IDs because we can't trust the helper functions exist
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

            float angleDeg = acos(cosAng) * (180.0f / 3.14159f);

            // 6. Apply Rotation
            RwV3d axis = { 1.0f, 0.0f, 0.0f };
            RwFrameRotate(hier->pNodeInfo[calfIdx].pFrame, &axis, angleDeg, rwCOMBINEPRECONCAT);
            RwFrameUpdateObjects(hier->pNodeInfo[thighIdx].pFrame);
        }
    }
}

// --- ENTRY POINT ---

extern "C" void OnModLoad() {
    // Basic hook to run every frame
    Events::processScriptsEvent += [] {
        CPed* player = FindPlayerPed();
        if (player) {
            ApplyLegIK(player);
        }
    };
}
