#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>

// CORRECT PATHS
#include "aml-psdk/game_sa/plugin.h"
#include "aml-psdk/game_sa/engine/World.h"
#include "aml-psdk/game_sa/entity/Ped.h"
#include "aml-psdk/renderware/RwMatrix.h"
#include "aml-psdk/renderware/RwRender.h"

using namespace plugin;

// Instantiate fakes to satisfy linker
IAMLer* aml = new IAMLer();
FakeLogger* logger = new FakeLogger();

MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// --- HELPERS ---

// Manual function to find a bone index (Safest method)
int GetBoneIndex(RpHAnimHierarchy* hier, int boneID) {
    if (!hier) return -1;
    for (int i = 0; i < hier->numNodes; i++) {
        if (hier->pNodeInfo[i].nodeID == boneID) {
            return i;
        }
    }
    return -1;
}

RpHAnimHierarchy* GetHierarchy(RpClump* clump) {
    if (!clump) return nullptr;
    RpHAnimHierarchy* hier = nullptr;
    RpClumpForAllAtomics(clump, [](RpAtomic* atomic, void* data) {
        if (RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic))) {
            *(RpHAnimHierarchy**)data = RpSkinAtomicGetHAnimHierarchy(atomic);
            return (RpAtomic*)nullptr;
        }
        return atomic;
    }, &hier);
    return hier;
}

// --- LOGIC ---

void ApplyLegIK(CPed* ped) {
    if (!ped->m_pRwClump) return;

    RpHAnimHierarchy* hier = GetHierarchy(ped->m_pRwClump);
    if (!hier) return;

    // Bone IDs: 2=Thigh, 3=Calf
    int thighIdx = GetBoneIndex(hier, 2);
    int calfIdx = GetBoneIndex(hier, 3);

    if (thighIdx == -1 || calfIdx == -1) return;

    RwMatrix* thighMat = &hier->pNodeInfo[thighIdx].pFrame->modelingMtx;

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

            RwV3d axis = { 1.0f, 0.0f, 0.0f };
            // Use standard RW function if available, otherwise manual matrix math would be needed
            // But RwFrameRotate is usually linked in game_sa libraries
            RwFrameRotate(hier->pNodeInfo[calfIdx].pFrame, &axis, angleDeg, rwCOMBINEPRECONCAT);
            RwFrameUpdateObjects(hier->pNodeInfo[thighIdx].pFrame);
        }
    }
}

extern "C" void OnModLoad() {
    logger->SetTag("LegIK");
    
    Events::processScriptsEvent += [] {
        CPed* player = FindPlayerPed();
        if (player) ApplyLegIK(player);
    };
}
