#include <mod/amlmod.h>

#include <aml-psdk/game_sa/engine/Game.h>
#include <aml-psdk/game_sa/entity/PlayerPed.h>
#include <aml-psdk/game_sa/base/Timer.h>

MYMOD(net.psdk.mymod.guid, AML PSDK Example: Health Regen, 1.0, RusJJ)

DECL_HOOKv(CGame__Process)
{
    CGame__Process();

    CPlayerPed* player = FindPlayerPed(-1);
    if(!player || !player->IsAlive()) return;

    if(player->m_fHealth < player->m_fMaxHealth)
    {
        if((CTimer::GetTimeInMS() - player->m_nLastDamagedTime) > 7000) // after 7s
        {
            player->m_fHealth += CTimer::GetTimeScale() * CTimer::GetTimeStepInSeconds() * 1.0f;
            if(player->m_fHealth > player->m_fMaxHealth) // Clamp the value!
            {
                player->m_fHealth = player->m_fMaxHealth;
            }
        }
    }
}

ON_MOD_LOAD()
{
    HOOK(CGame__Process, CGame::Process);
}