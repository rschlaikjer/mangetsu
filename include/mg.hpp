#pragma once

#define ASSERT(expr)                                                           \
  do {                                                                         \
    if (!expr) {                                                               \
      asm volatile("ud2");                                                     \
    }                                                                          \
  } while (false)
