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


#ifdef DT_DEBUG
#define DEBUG_ASSERT(predicate) debug_assert(predicate)
#define DEBUG_STOP debug_stop()
#else
#define DEBUG_ASSERT(predicate) {}
#define DEBUG_STOP {}
#endif
#endif // DEBUG_H
