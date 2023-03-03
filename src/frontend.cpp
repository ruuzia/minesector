#if defined(FRONTEND_NATIVE)
#include "native_frontend.cpp"
#elif defined(FRONTEND_TEST) 
extern "C" {
    #include "test_frontend.c"
}
#endif
