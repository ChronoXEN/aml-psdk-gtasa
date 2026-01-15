#include <aml-psdk/sdk_base.h>

struct CTimer
{
    static inline auto Initialise = GetMainLibrarySymbol<void(*)()>("_ZN6CTimer10InitialiseEv");
    static inline auto Shutdown = GetMainLibrarySymbol<void(*)()>("_ZN6CTimer8ShutdownEv");
    static inline auto Update = GetMainLibrarySymbol<void(*)()>("_ZN6CTimer6UpdateEv");
    static inline auto UpdateVariables = GetMainLibrarySymbol<void(*)(float)>("_ZN6CTimer15UpdateVariablesEf");
    static inline auto Suspend = GetMainLibrarySymbol<void(*)()>("_ZN6CTimer7SuspendEv");
    static inline auto Resume = GetMainLibrarySymbol<void(*)()>("_ZN6CTimer6ResumeEv");
    static inline auto GetCyclesPerMillisecond = GetMainLibrarySymbol<u32(*)()>("_ZN6CTimer23GetCyclesPerMillisecondEv");
    static inline auto GetCyclesPerFrame = GetMainLibrarySymbol<u32(*)()>("_ZN6CTimer17GetCyclesPerFrameEv");
    static inline auto GetCurrentTimeInCycles = GetMainLibrarySymbol<u32(*)()>("_ZN6CTimer22GetCurrentTimeInCyclesEv");
    static inline auto Stop = GetMainLibrarySymbol<void(*)()>("_ZN6CTimer4StopEv");
    static inline auto GetIsSlowMotionActive = GetMainLibrarySymbol<bool(*)()>("_ZN6CTimer21GetIsSlowMotionActiveEv");
    static inline auto StartUserPause = GetMainLibrarySymbol<void(*)()>("_ZN6CTimer14StartUserPauseEv");
    static inline auto EndUserPause = GetMainLibrarySymbol<void(*)()>("_ZN6CTimer12EndUserPauseEv");

    DECL_VALUE_PLT_U32(m_snTimeInMilliseconds, BYBIT(0x676FFC, 0x84C030));
    DECL_VALUE_PLT_U32(m_snTimeInMillisecondsNonClipped, BYBIT(0x679DBC, 0x851B90));
    DECL_VALUE_PLT_U32(m_snTimeInMillisecondsPauseMode, BYBIT(0x677A2C, 0x84D480));
};