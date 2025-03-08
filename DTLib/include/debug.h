#ifndef DEBUG_H
#define DEBUG_H
#include <cassert>



static void debug_assert(bool predicate)
{
#ifdef DT_DEBUG
    assert(predicate);
#endif
}

static void debug_stop()
{
#ifdef DT_DEBUG
    assert(false);
#endif
}

#endif // DEBUG_H
