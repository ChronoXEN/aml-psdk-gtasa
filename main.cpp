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

    RwFrame* calfFrame = hier->pNodeInfo[calfIdx].pFrame;
    RwFrame* thighFrame = hier->pNodeInfo[thighIdx].pFrame;

    // --- FORCE TEST ---
    // Ignore raycasts. Just bend the leg 90 degrees to prove we have control.
    
    RwV3d axis = { 1.0f, 0.0f, 0.0f }; // Rotate on X axis
    float angle = 45.0f; // 45 Degrees

    if(RwFrameRotate) {
        // PRECONCAT adds to the animation. 
        // If the mod is working, CJ will look like he is constantly squatting on one leg.
        RwFrameRotate(calfFrame, &axis, angle, rwCOMBINEPRECONCAT);
        
        if(RwFrameUpdateObjects) RwFrameUpdateObjects(thighFrame);
    }
}
