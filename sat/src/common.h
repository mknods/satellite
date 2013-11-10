typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;

typedef signed char sint8;
typedef signed short sint16;
typedef signed long sint32;

// ### use Beagle Bone Black ###
//#define BBB 1

#define _DEBUG
#ifdef _DEBUG
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif
