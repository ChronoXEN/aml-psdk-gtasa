#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <jni.h>
#include <stdlib.h> // For abort()

IAMLer* aml = new IAMLer();
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// Symbol we are testing
void* (*RpAnimBlendClumpFindBone)(void* clump, unsigned int boneID) = nullptr;

extern "C" void OnModLoad() {
    // 1. GET HANDLE
    void* hGame = aml->GetLibHandle("libGTASA.so");
    
    // 2. CHECK FOR THE SYMBOL YOU FOUND
    // If this symbol exists in v2.00, this pointer will be valid.
    // If it doesn't exist, this pointer will be NULL.
    RpAnimBlendClumpFindBone = (void*(*)(void*, unsigned int))aml->GetSym(hGame, "_Z24RpAnimBlendClumpFindBoneP7RpClumpj");

    // ---------------------------------------------------------
    // THE CRASH TEST
    // ---------------------------------------------------------
    
    // TEST A: PROVE MOD IS LOADED
    // Uncomment the line below. If the game crashes instantly, your mod is installed correctly.
    // abort(); 

    // TEST B: PROVE SYMBOL EXISTS
    // If the symbol was NOT found, we crash the game to tell you.
    if (RpAnimBlendClumpFindBone == nullptr) {
        // FORCE CRASH by writing to null pointer
        // This means: "I am loaded, but I can't find the function!"
        *(int*)0 = 0; 
    }
}
