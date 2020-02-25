#ifndef ESTF_ASSERT_H
#define ESTF_ASSERT_H

#ifdef ESTF_USE_RESULT_COLLECTOR

#ifndef ESTF_RESULT_COLLECTOR_ASSERT_H
#include <ESTFResultCollectorAssert.h>
#endif

#else

#ifndef ESTF_SYSTEM_ASSERT_H
#include <ESTFSystemAssert.h>
#endif

#endif

#endif
