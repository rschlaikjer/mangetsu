#pragma once

namespace mg {

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

// LE <-> Host operations are identity
static inline uint16_t le_to_host_u16(uint16_t v) { return v; }
static inline uint32_t le_to_host_u32(uint32_t v) { return v; }
static inline uint64_t le_to_host_u64(uint64_t v) { return v; }
static inline uint16_t host_to_le_u16(uint16_t v) { return v; }
static inline uint32_t host_to_le_u32(uint32_t v) { return v; }
static inline uint64_t host_to_le_u64(uint64_t v) { return v; }

// BE <-> Host operations are bswapped
static inline uint16_t be_to_host_u16(uint16_t v) {
  return __builtin_bswap16(v);
}
static inline uint32_t be_to_host_u32(uint32_t v) {
  return __builtin_bswap32(v);
}
static inline uint64_t be_to_host_u64(uint64_t v) {
  return __builtin_bswap64(v);
}
static inline uint16_t host_to_be_u16(uint16_t v) {
  return __builtin_bswap16(v);
}
static inline uint32_t host_to_be_u32(uint32_t v) {
  return __builtin_bswap32(v);
}
static inline uint64_t host_to_be_u64(uint64_t v) {
  return __builtin_bswap64(v);
}

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

// BE <-> Host operations are identity
static inline uint16_t be_to_host_u16(uint16_t v) { return v; }
static inline uint32_t be_to_host_u32(uint32_t v) { return v; }
static inline uint64_t be_to_host_u64(uint64_t v) { return v; }
static inline uint16_t host_to_be_u16(uint16_t v) { return v; }
static inline uint32_t host_to_be_u32(uint32_t v) { return v; }
static inline uint64_t host_to_be_u64(uint64_t v) { return v; }

// LE <-> Host operations are bswapped
static inline uint16_t le_to_host_u16(uint16_t v) {
  return __builtin_bswap16(v);
}
static inline uint32_t le_to_host_u32(uint32_t v) {
  return __builtin_bswap32(v);
}
static inline uint64_t le_to_host_u64(uint64_t v) {
  return __builtin_bswap64(v);
}
static inline uint16_t host_to_le_u16(uint16_t v) {
  return __builtin_bswap16(v);
}
static inline uint32_t host_to_le_u32(uint32_t v) {
  return __builtin_bswap32(v);
}
static inline uint64_t host_to_le_u64(uint64_t v) {
  return __builtin_bswap64(v);
}

#else
#error "Unsupported __BYTE_ORDER__ " #__BYTE_ORDER__
#endif

} // namespace mg
