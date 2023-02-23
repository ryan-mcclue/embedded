// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(BASE_MEMORY_H)
#define BASE_MEMORY_H

// IMPORTANT(Ryan): Although using memory arenas restricts arbitrary lifetimes, this provides more benefits than negatives.
// In majority of cases, a large number of allocations can be bucketed into same arena

// TODO(Ryan): Put device into panic mode when out of memory  

#include <string.h>
#include <stdlib.h>

#define MEMORY_ZERO(p, n) memset((p), 0, (n))
#define MEMORY_ZERO_STRUCT(p) MEMORY_ZERO((p), sizeof(*(p)))
#define MEMORY_ZERO_ARRAY(a) MEMORY_ZERO((a), sizeof(a[0]))

#define MEMORY_COPY(d, s, n) memmove((d), (s), (n))
#define MEMORY_COPY_STRUCT(d, s, n) MEMORY_COPY((d), (s), sizeof(*(s)))
#define MEMORY_COPY_ARRAY(d, s, n) MEMORY_COPY((d), (s), sizeof((s)))

#define MEMORY_MATCH(a, b, n) (memcmp((a), (b), (n)) == 0)

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) (((u64)x) << 30)
#define TB(x) (((u64)x) << 40)

typedef struct MemArena MemArena;
struct MemArena
{
  void *memory;
  size_t commit_pos;
  size_t max;
  size_t pos;
  size_t align;
};

// TODO(Ryan): Better to zero from the get go to reduce incorrect ZII assumption errors?
INTERNAL MemArena *
mem_arena_allocate(size_t cap)
{
  MemArena *result = (MemArena *)malloc(cap);
  ERRNO_ASSERT(result != NULL);

  result->memory = result + sizeof(MemArena);
  result->max = cap;
  result->pos = sizeof(MemArena);
  result->align = 8;

  return result;
}

INTERNAL void
mem_arena_deallocate(MemArena *arena)
{
  free(arena);
}
 
#define MEM_ARENA_PUSH_ARRAY(a,T,c) (T*)mem_arena_push((a), sizeof(T)*(c))
#define MEM_ARENA_PUSH_ARRAY_ZERO(a,T,c) (T*)mem_arena_push_zero((a), sizeof(T)*(c))
#define MEM_ARENA_POP_ARRAY(a,T,c) mem_arena_pop((a), sizeof(T)*(c))

#define MEM_ARENA_PUSH_STRUCT(a,T) (T*)mem_arena_push((a), sizeof(T))
#define MEM_ARENA_PUSH_STRUCT_ZERO(a,T) (T*)mem_arena_push_zero((a), sizeof(T))

INTERNAL void *
mem_arena_push_aligned(MemArena *arena, size_t size, size_t align)
{
  void *result = NULL;

  size_t clamped_align = CLAMP_BOTTOM(align, arena->align);

  size_t pos = arena->pos;

  size_t pos_address = INT_FROM_PTR(arena) + pos;
  size_t aligned_pos = ALIGN_POW2_UP(pos_address, clamped_align);
  size_t alignment_size = aligned_pos - pos_address;

  if (pos + alignment_size + size <= arena->max)
  {
    u8 *mem_base = (u8 *)arena;
    result = mem_base + pos + alignment_size;
    size_t new_pos = pos + alignment_size + size;
    arena->pos = new_pos;
  }

  return result;
}

INTERNAL void *
mem_arena_push(MemArena *arena, size_t size)
{
  return mem_arena_push_aligned(arena, size, arena->align);
}

INTERNAL void *
mem_arena_push_zero(MemArena *arena, size_t size)
{
  void *memory = mem_arena_push(arena, size);

  MEMORY_ZERO(memory, size);

  return memory;
}

INTERNAL void
mem_arena_set_pos_back(MemArena *arena, size_t pos)
{
  size_t clamped_pos = CLAMP_BOTTOM(sizeof(*arena), pos);

  if (arena->pos > clamped_pos)
  {
    arena->pos = clamped_pos;
  }
}

INTERNAL void
mem_arena_pop(MemArena *arena, size_t size)
{
  mem_arena_set_pos_back(arena, arena->pos - size);
}

INTERNAL void
mem_arena_clear(MemArena *arena)
{
  mem_arena_pop(arena, arena->pos);
}


typedef struct TempMemArenas TempMemArenas;
struct TempMemArenas
{
  MemArena *arenas[2];  
};

GLOBAL TempMemArenas global_temp_mem_arenas = ZERO_STRUCT;

INTERNAL void
initialise_global_temp_mem_arenas(size_t cap)
{
  for (u32 arena_i = 0; arena_i < ARRAY_COUNT(global_temp_mem_arenas.arenas); ++arena_i)
  {
    global_temp_mem_arenas.arenas[arena_i] = mem_arena_allocate(cap);
  }
}

typedef struct TempMemArena TempMemArena;
struct TempMemArena
{
  MemArena *arena;
  size_t pos;
};

INTERNAL TempMemArena
temp_mem_arena_get(MemArena **conflicts, u32 conflict_count)
{
  TempMemArena scratch = ZERO_STRUCT;

  for (u32 temp_idx = 0; temp_idx < ARRAY_COUNT(global_temp_mem_arenas.arenas); temp_idx += 1)
  {
    b32 is_conflicting = false;
    for (MemArena **conflict = conflicts; conflict < conflicts+conflict_count; conflict += 1)
    {
      if (*conflict == global_temp_mem_arenas.arenas[temp_idx])
      {
        is_conflicting = true;
        break;
      }
    }

    if (!is_conflicting)
    {
      scratch.arena = global_temp_mem_arenas.arenas[temp_idx];
      scratch.pos = scratch.arena->pos;
      break;
    }
  }

  return scratch;
}

INTERNAL void
temp_mem_arena_release(TempMemArena temp)
{
  mem_arena_set_pos_back(temp.arena, temp.pos);
}

#endif
