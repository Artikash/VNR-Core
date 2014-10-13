#ifndef _CC_CCMACRO_H
#define _CC_CCMACRO_H

// ccmacro.h
// 12/9/2011 jichi

#define CC_UNUSED(_var) (void)(_var)
#define CC_NOP          CC_UNUSED(0)

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
# define CC_LIKELY(expr)    __builtin_expect(!!(expr), true)
# define CC_UNLIKELY(expr)  __builtin_expect(!!(expr), false)
#else
# define CC_LIKELY(x)   (x)
# define CC_UNLIKELY(x) (x)
#endif

Q_DECL_CONSTEXPR static inline bool qFuzzyCompare(float p1, float p2)
{
    return (qAbs(p1 - p2) <= 0.00001f * qMin(qAbs(p1), qAbs(p2)));
}


#endif // _CC_CCMACRO_H
