#ifndef ESB_RAND_H
#include <ESBRand.h>
#endif

#ifndef ESB_SYSTEM_TIME_SOURCE_H
#include <ESBSystemTimeSource.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

namespace ESB {

Rand::Rand() {
#ifdef HAVE_RAND_R
  _seed = SystemTimeSource::Now().microSeconds();
#endif
}

Rand::Rand(unsigned int seed) : _seed(seed) {}

Rand::~Rand() {}

double Rand::generate() {
#if defined HAVE_RAND_R && defined HAVE_RAND_MAX
  return rand_r(&_seed) / (RAND_MAX + 1.0);
#elif defined HAVE_RAND && defined HAVE_RAND_MAX
  return rand() / (RAND_MAX + 1.0);
#else
#error "rand_r and RAND_MAX or equivalent is required"
#endif
}

}  // namespace ESB
