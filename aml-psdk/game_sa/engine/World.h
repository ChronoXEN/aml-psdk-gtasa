#ifndef __AML_PSDK_SAWORLD_H
#define __AML_PSDK_SAWORLD_H

#include <aml-psdk/game_sa/plugin.h>
#include <aml-psdk/gta_base/Vector.h>

DECL_CLASS(CWorld)
    // STATIC functions
    // TODO:
    DECL_FASTCALL_SIMPLE(FindGroundZForCoord, _ZN6CWorld19FindGroundZForCoordEff, float, float x, float y);
DECL_CLASS_END()

#endif // __AML_PSDK_SAWORLD_H
