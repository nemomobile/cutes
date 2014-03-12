#ifndef _CUTES_PRIVATE_CONFIG_HPP_
#define _CUTES_PRIVATE_CONFIG_HPP_


// #define A_QT_V50 QT_VERSION_CHECK(5, 0, 0)
// #define A_QT_V52 QT_VERSION_CHECK(5, 2, 0)

// #define QJS_ENGINE_V8 8
// #define QJS_ENGINE_V4 4

// #if (QT_VERSION >= A_QT_V50) && (QT_VERSION < A_QT_V52)
// #define QJS_ENGINE QJS_ENGINE_V8
// #elif (QT_VERSION >= A_QT_V52)
// #define QJS_ENGINE QJS_ENGINE_V4
// #endif

// #undef A_QT_V50
// #undef A_QT_V52

#ifndef QJS_ENGINE
#error QJS_ENGINE should be defined
#endif
#define QJS_ENGINE_V8 8
#define QJS_ENGINE_V4 4

#endif // _CUTES_PRIVATE_CONFIG_HPP_
