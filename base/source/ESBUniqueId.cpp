#ifndef ESB_UNIQUE_ID_H
#include <ESBUniqueId.h>
#endif

#ifndef ESB_RAND_H
#include <ESBRand.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

namespace ESB {

static Rand Rand;
static Mutex Lock;

inline static Int16 HexToInt(char hex) {
  if ('0' <= hex && '9' >= hex) {
    return hex - '0';
  }

  if ('a' <= hex && 'f' >= hex) {
    return hex - 'a' + 10;
  }

  if ('A' <= hex && 'F' >= hex) {
    return hex - 'A' + 10;
  }

  return -1;
}

Error UniqueId::Parse(const char *buffer, UInt128 *uuid) {
  if (!buffer || !uuid) {
    return ESB_NULL_POINTER;
  }

  UInt128 out = 0;
  const char *p = buffer;
  int bytes = 0;

  while (*p && bytes < 16) {
    if ('-' == *p) {
      ++p;
      continue;
    }

    if (!*p || !*(p + 1)) {
      return ESB_CANNOT_PARSE;
    }

    Int16 result = HexToInt(*p++);
    if (0 > result) {
      return ESB_CANNOT_PARSE;
    }
    UInt8 digit = ((UInt8)result) << 4;

    result = HexToInt(*p++);
    if (0 > result) {
      return ESB_CANNOT_PARSE;
    }
    digit |= (UInt8)result;

    out |= ((UInt128)digit) << (bytes++ * 8);
  }

  if (bytes != 16) {
    return ESB_CANNOT_PARSE;
  }

  *uuid = out;
  return ESB_SUCCESS;
}

Error UniqueId::Parse(const char *buffer, UniqueId &uuid) {
  UInt128 id = 0;
  Error error = Parse(buffer, &id);
  if (ESB_SUCCESS != error) {
    return error;
  }

  uuid.set(id);
  return ESB_SUCCESS;
}

inline static char IntToHex(UInt8 integer) {
  if (0 <= integer && 9 >= integer) {
    return '0' + integer;
  }

  if (10 <= integer && 15 >= integer) {
    return 'a' + integer - 10;
  }

  return 0;
}

Error UniqueId::Format(char *buffer, Size size, UInt128 uuid) {
  if (ESB_UUID_PRESENTATION_SIZE > size) {
    return ESB_OVERFLOW;
  }

  for (int i = 0, idx = 0; i < 16; ++i) {
    UInt8 digit = (uuid >> (i * 8)) & 0xFF;
    buffer[idx++] = IntToHex((digit >> 4) & 0xF);
    buffer[idx++] = IntToHex(digit & 0xF);

    switch (idx) {
      case 8:
      case 13:
      case 18:
      case 23:
        buffer[idx++] = '-';
        break;
    }
  }

  buffer[ESB_UUID_PRESENTATION_SIZE - 1] = 0;
  return ESB_SUCCESS;
}

#include <unistd.h>

Error UniqueId::Generate(UInt128 *uuid) {
  if (!uuid) {
    return ESB_NULL_POINTER;
  }

  Lock.writeAcquire();
  UInt32 one = Rand.generate(ESB_UINT32_MIN, ESB_UINT32_MAX);
  UInt32 two = Rand.generate(ESB_UINT32_MIN, ESB_UINT32_MAX);
  UInt32 three = Rand.generate(ESB_UINT32_MIN, ESB_UINT32_MAX);
  UInt32 four = Rand.generate(ESB_UINT32_MIN, ESB_UINT32_MAX);
  Lock.writeRelease();

  *uuid = (((UInt128)one) << 96) + (((UInt128)two) << 64) + (((UInt128)three) << 32) + ((UInt128)four);

  return ESB_SUCCESS;
}

Error UniqueId::Generate(UniqueId &uuid) {
  UInt128 id = 0;
  Error error = Generate(&id);
  if (ESB_SUCCESS != error) {
    return error;
  }

  uuid.set(id);

  return ESB_SUCCESS;
}

}  // namespace ESB
