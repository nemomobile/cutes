#ifndef _CUTES_V4_UTIL_HPP_
#define _CUTES_V4_UTIL_HPP_

#include <QJSValueList>
#include <QJSEngine>

namespace cutes {

static inline char const *cutesRegisterName()
{
    return "cutesRegister";
}

typedef QJSValue (*cutesRegisterFnType)(QJSEngine *);

}

extern "C" QJSValue cutesRegister(QJSEngine *);

#endif // _CUTES_V4_UTIL_HPP_
