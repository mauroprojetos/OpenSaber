#ifndef GRINLIZ_ASSERT_INCLUDED
#define GRINLIZ_ASSERT_INCLUDED

template<bool> struct CompileTimeAssert;
template<> struct CompileTimeAssert <true> {};
#define STATIC_ASSERT(e) (CompileTimeAssert <(e) != 0>())

void AssertOut(const char* message, const char* file, int line);
void AssertOut2(const char* message, int value0, int value1, const char* file, int line);
#define ASSERT( x ) 	    if (!(x)) { AssertOut(#x, __FILE__, __LINE__); while(true) {} }
#define ASSERT2( x, y, z ) 	if (!(x)) { AssertOut2(#x, y, z, __FILE__, __LINE__); while(true) {} }

#define TEST_IS_TRUE(x) {         \
    if((x)) {                     \
    }                             \
    else {                        \
        ASSERT(false);            \
        return false;             \
    }                             \
}

#define TEST_IS_FALSE(x) {        \
    if(!(x)) {                    \
    }                             \
    else {                        \
        ASSERT(false);            \
        return false;             \
    }                             \
}

#define TEST_IS_EQ(x, y) {        \
    if((x) == (y)) {              \
    }                             \
    else {                        \
        ASSERT(false);            \
        return false;             \
    }                             \
}

#endif // GRINLIZ_ASSERT_INCLUDED
