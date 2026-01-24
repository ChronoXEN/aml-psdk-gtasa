#include "plugin.h"
#include <aml-psdk/sdk_callevents.h>

struct Events
{
    DECL_EVENT_SYM_ARG0(void, gameProcessEvent, _ZN5CGame7ProcessEv);
};