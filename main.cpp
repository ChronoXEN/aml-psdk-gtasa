#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <jni.h>
#include <stdlib.h>

// --- 1. FORCE VISIBILITY MACROS ---
// These tell the compiler "Keep these visible to the outside world!"
#define EXPORT __attribute__((visibility("default")))

IAMLer* aml = new IAMLer();

// We also mark the config as 'used' so it doesn't get optimized away
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// Symbol we are testing
void* (*RpAnimBlendClumpFindBone)(void* clump, unsigned int boneID) = nullptr;

// --- 2. THE EXPORTED FUNCTION ---
// Notice the 'EXPORT' tag. This is the fix.
extern "C" EXPORT void OnModLoad() {
    
    // ---------------------------------------------------------
    // THE CRASH TEST (ROUND 2)
    // ---------------------------------------------------------
    
    // Uncomment this line to force a crash.
    // If the game crashes, the mod is finally loading!
    abort(); 

    // Standard Setup (This runs if you comment out abort)
    void* hGame = aml->GetLibHandle("libGTASA.so");
    RpAnimBlendClumpFindBone = (void*(*)(void*, unsigned int))aml->GetSym(hGame, "_Z24RpAnimBlendClumpFindBoneP7RpClumpj");
}
