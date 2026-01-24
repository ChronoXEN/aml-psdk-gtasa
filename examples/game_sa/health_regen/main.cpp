#include <mod/amlmod.h>

#include <aml-psdk/game_sa/engine/Game.h>
#include <aml-psdk/game_sa/entity/PlayerPed.h>
#include <aml-psdk/game_sa/base/Timer.h>
#include <aml-psdk/game_sa/Events.h>

MYMOD(net.psdk.mymod.guid, AML PSDK Example: Health Regen, 1.0, RusJJ)

ON_MOD_LOAD()
{
    Events::gameProcessEvent += []()
    {
        CPlayerPed* player = FindPlayerPed(-1);
        if(!player || !player->IsAlive() || player->IsMaxHealth()) return;

        if(CTimer::IsTimePassed(player->m_nLastDamagedTime + 7000)) // after 7s
        {
            float hpPerFrame = CTimer::GetTimeScale() * CTimer::GetTimeStepInSeconds() * 1.0f;
            player->AddHealthSafe(hpPerFrame);
        }
    };
}