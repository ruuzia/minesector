#include <__config>
#if defined(FRONTEND_NATIVE)
#include "native_frontend.cpp"
#elif defined(FRONTEND_TEST) 
#include "test_frontend.cpp"
#else
static_assert(0, "Missing FRONTEND_ macro");
#endif
