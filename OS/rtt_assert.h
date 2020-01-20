#ifndef __RTT_ASSERT_H__
#define __RTT_ASSERT_H__

#define ASSERT(X) \
if (!(X)) { \
    rtt_assert_0(__FILE__,__LINE__, #X); \
}

#define ASSERT1(X,Y) \
if (!(X)) { \
    rtt_assert_1(__FILE__,__LINE__, #X, (Y)); \
}

#define ASSERT2(X,Y,Z) \
if (!(X)) { \
    rtt_assert_2(__FILE__,__LINE__, #X, (Y), (Z)); \
}

void rtt_assert_0(char* filename, int line, char* condition);
void rtt_assert_1(char* filename, int line, char* condition, uint32_t arg_1);
void rtt_assert_2(char* filename, int line, char* condition, uint32_t arg_1, uint32_t arg_2);

#endif