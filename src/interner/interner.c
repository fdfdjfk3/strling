#include "interner.h"
#include "types.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define SEED 0x12345678

uint32_t MurmurOAAT_32(const char *str, size_t len) {
  uint32_t h = SEED;
  // One-byte-at-a-time hash based on Murmur's mix
  // Source: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp
  for (size_t i = 0; i < len; ++str) {
    h ^= *str;
    h *= 0x5bd1e995;
    h ^= h >> 15;
    i += 1;
  }
  return h;
}

// adds str if it doesn't exist, else returns pointer of existing string
StrId table_addstr(StrTable *t, Slice str, int should_alloc_str) {
  if (t->occupied > (t->max_entries / 2)) {
    puts("This error occured because the table was large enough to start being "
         "slower. I need to implement table resizing :3");
    exit(1);
  }

  uint32_t idx = MurmurOAAT_32(str.ptr, str.len) % t->max_entries;

  // This assumes the string does not already exist in the table. (well, it
  // kinda checks for that). This also assumes that there are no null pointers
  // inserted in this table. Pretty bold assumptions, but I don't need a sanity
  // check for every minute detail.
  while (t->entries[idx] != NULL) {
    Slice slice = *(t->entries[idx]);
    if (slice.len == str.len) {
      if (strncmp(slice.ptr, str.ptr, str.len) == 0) {
        // looks like it already exists lol
        return t->entries[idx];
      }
    }
    idx = (idx + 1) % t->max_entries;
  }

  if (should_alloc_str) {
    char *alloced_str = malloc(sizeof(char) * str.len);
    strncpy(alloced_str, str.ptr, str.len);
    str.ptr = alloced_str;
  }

  t->entries[idx] = malloc(sizeof(Slice));
  *(t->entries[idx]) = str;
  return t->entries[idx];
}

// Slice table_getstr(StrTable *t, StrId id) { return t->entries[id]; }

Interner interner_new() {
  Interner interner = (Interner){
      .table = (StrTable){.entries = NULL, .occupied = 0, .max_entries = 0}};

  return interner;
}

void interner_init(Interner *i) {
  i->table.max_entries = 10000;
  i->table.entries = calloc(sizeof(Slice *), 10000);
  i->table.occupied = 0;
}

// This should be used instead of table_addstr directly to handle null str
StrId interner_intern(Interner *i, const char *ptr, size_t len) {
  Slice str = (Slice){.ptr = ptr, .len = len};

  return table_addstr(&i->table, str, 1);
}

StrId interner_intern_noalloc(Interner *i, const char *ptr, size_t len) {
  Slice str = (Slice){.ptr = ptr, .len = len};

  return table_addstr(&i->table, str, 0);
}
