// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(BASE_STRING_H)
#define BASE_STRING_H

#include <ctype.h>
#include <stdarg.h>


typedef struct String8 String8;
struct String8
{
  u8 *str;
  size_t size;
};

typedef u32 MATCH_FLAGS;
typedef u32 S8_MATCH_FLAGS;
enum
{
  MATCH_FLAG_FIND_LAST = (1 << 0),
};
enum
{
  S8_MATCH_FLAG_RIGHT_SIDE_LAZY = (1 << 4),
  S8_MATCH_FLAG_CASE_INSENSITIVE = (1 << 5),
};

typedef struct String8Node String8Node;
struct String8Node
{
  String8Node *next;
  String8 string;
};

typedef struct String8List String8List;
struct String8List
{
  String8Node *first;
  String8Node *last;
  size_t node_count;
  size_t total_size;
};

typedef struct String8Join String8Join;
struct String8Join
{
  String8 pre;
  String8 mid;
  String8 post;
};

INTERNAL String8
s8(u8 *str, size_t size)
{
  String8 result = ZERO_STRUCT;

  result.str = str;
  result.size = size;

  return result;
}

#define s8_lit(s) s8((u8 *)(s), sizeof(s) - 1)

#if defined(MAIN_DEBUG)
// NOTE(Ryan): Remove unecessary string literals from flash
#define DEBUG_S8(s) s8_lit(s)
#else
#define DEBUG_S8(s)
#endif

#define s8_cstring(s) s8((u8 *)(s), strlen((char *)s))
// IMPORTANT(Ryan): When substringing will run into situations where not null terminated.
// So, use like: "%.*s", s8_varg(string)
#define s8_varg(s) (int)(s).size, (s).str

INTERNAL String8
s8_up_to(u8 *start, u8 *up_to)
{
  String8 string = ZERO_STRUCT;

  string.str = start;
  string.size = (size_t)(up_to - start);

  return string;
}

INTERNAL String8
s8_substring(String8 str, size_t start, size_t end)
{
  if (end > str.size)
  {
    end = str.size;
  }

  if (start > str.size)
  {
    start = str.size;
  }

  if (start > end)
  {
    SWAP(size_t, start, end);
  }

  str.size = end - start;
  str.str += start;

  return str;
}

INTERNAL String8
s8_advance(String8 str, size_t advance)
{
  return s8_substring(str, advance, str.size);
}

INTERNAL String8
s8_chop(String8 str, size_t chop)
{
  return s8_substring(str, 0, str.size - chop);
}

INTERNAL String8
s8_prefix(String8 str, size_t prefix)
{
  return s8_substring(str, 0, prefix);
}

INTERNAL String8
s8_suffix(String8 str, size_t suffix)
{
  return s8_substring(str, str.size - suffix, str.size);
}

INTERNAL b32
s8_match(String8 a, String8 b, S8_MATCH_FLAGS flags)
{
  b32 result = false;

  if (a.size == b.size || flags & S8_MATCH_FLAG_RIGHT_SIDE_LAZY)
  {
    result = true;

    for (size_t i = 0; i < a.size && i < b.size; i += 1)
    {
      b32 match = (a.str[i] == b.str[i]);

      if (flags & S8_MATCH_FLAG_CASE_INSENSITIVE)
      {
        match |= (tolower(a.str[i]) == tolower(b.str[i]));
      }

      if (!match)
      {
        result = false;
        break;
      }
    }
  }

  return result;
}

INTERNAL size_t
s8_find_substring(String8 str, String8 substring, size_t start_pos, MATCH_FLAGS flags)
{
  b32 found = false;
  size_t found_idx = str.size;

  for (size_t i = start_pos; i < str.size; i += 1)
  {
    if (i + substring.size <= str.size)
    {
      String8 substr_from_str = s8_substring(str, i, i + substring.size);

      if (s8_match(substr_from_str, substring, flags))
      {
        found_idx = i;
        found = true;

        if (!(flags & MATCH_FLAG_FIND_LAST))
        {
          break;
        }
      }
    }
  }

  return found_idx;
}

INTERNAL String8
s8_copy(MemArena *arena, String8 string)
{
  String8 result = ZERO_STRUCT;

  result.size = string.size;
  result.str = MEM_ARENA_PUSH_ARRAY(arena, u8, string.size + 1);
  MEMORY_COPY(result.str, string.str, string.size);
  result.str[string.size] = '\0';

  return result;
}

INTERNAL String8
s8_fmt(MemArena *arena, char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  String8 result = ZERO_STRUCT;
  size_t needed_bytes = (size_t)vsnprintf(NULL, 0, fmt, args) + 1;
  result.str = MEM_ARENA_PUSH_ARRAY(arena, u8, needed_bytes);
  result.size = needed_bytes - 1;
  result.str[needed_bytes - 1] = '\0';
  vsnprintf((char *)result.str, needed_bytes, fmt, args);

  va_end(args);

  return result;
}

INTERNAL String8
s8_fmt_nested(MemArena *arena, char *fmt, va_list args)
{
  String8 result = ZERO_STRUCT;
  size_t needed_bytes = (size_t)vsnprintf(NULL, 0, fmt, args) + 1;
  result.str = MEM_ARENA_PUSH_ARRAY(arena, u8, needed_bytes);
  result.size = needed_bytes - 1;
  result.str[needed_bytes - 1] = '\0';
  vsnprintf((char *)result.str, needed_bytes, fmt, args);

  return result;
}

INTERNAL void
s8_list_push(MemArena *arena, String8List *list, String8 string)
{
  String8Node *node = MEM_ARENA_PUSH_ARRAY_ZERO(arena, String8Node, 1);
  node->string = string;

  SLL_QUEUE_PUSH(list->first, list->last, node);

  list->node_count += 1;
  list->total_size += string.size;
}

INTERNAL void
s8_list_push_fmt(MemArena *arena, String8List *list, char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  String8 string = s8_fmt(arena, fmt, args);

  va_end(args);

  s8_list_push(arena, list, string);
}

INTERNAL void
s8_list_concat(String8List *list, String8List *to_push)
{
  if (to_push->first)
  {
    list->node_count += to_push->node_count;
    list->total_size += to_push->total_size;

    if (list->last == NULL)
    {
      *list = *to_push;
    }
    else
    {
      list->last->next = to_push->first;
      list->last = to_push->last;
    }
  }

  MEMORY_ZERO_STRUCT(to_push);
}

INTERNAL String8List
s8_split(MemArena *arena, String8 string, String8 *splitters, int splitter_count)
{
  String8List list = ZERO_STRUCT;

  size_t split_start = 0;
  for(size_t i = 0; i < string.size; i += 1)
  {
    b32 was_split = 0;
    for (int split_idx = 0; split_idx < splitter_count; split_idx += 1)
    {
      b32 match = 0;
      if (i + splitters[split_idx].size <= string.size)
      {
        match = 1;
        for (size_t split_i = 0; split_i < splitters[split_idx].size && i + split_i < string.size; split_i += 1)
        {
          if (splitters[split_idx].str[split_i] != string.str[i + split_i])
          {
            match = 0;
            break;
          }
        }
      }
      if (match)
      {
        String8 split_string = s8(string.str + split_start, i - split_start);
        s8_list_push(arena, &list, split_string);
        split_start = i + splitters[split_idx].size;
        i += splitters[split_idx].size - 1;
        was_split = 1;
        break;
      }
    }

    if (was_split == 0 && i == string.size - 1)
    {
      String8 split_string = s8(string.str + split_start, i+1 - split_start);
      s8_list_push(arena, &list, split_string);
      break;
    }
  }

  return list;
}

INTERNAL String8
s8_list_join(MemArena *arena, String8List list, String8Join *join_ptr)
{
  // setup join parameters
  String8Join join = ZERO_STRUCT;
  if (join_ptr != NULL)
  {
    MEMORY_COPY(&join, join_ptr, sizeof(join));
  }

  // calculate size & allocate
  size_t sep_count = 0;
  if (list.node_count > 1)
  {
    sep_count = list.node_count - 1;
  }

  String8 result = ZERO_STRUCT;
  result.size = (list.total_size + join.pre.size + sep_count*join.mid.size + join.post.size);
  result.str = MEM_ARENA_PUSH_ARRAY_ZERO(arena, u8, result.size);

  // fill
  u8 *ptr = result.str;
  MEMORY_COPY(ptr, join.pre.str, join.pre.size);
  ptr += join.pre.size;

  for (String8Node *node = list.first; node; node = node->next)
  {
    MEMORY_COPY(ptr, node->string.str, node->string.size);
    ptr += node->string.size;
    if (node != list.last)
    {
      MEMORY_COPY(ptr, join.mid.str, join.mid.size);
      ptr += join.mid.size;
    }
  }

  MEMORY_COPY(ptr, join.post.str, join.post.size);
  ptr += join.post.size;

  return(result);
}

INTERNAL u32
char_to_u32(char ch)
{
  u32 result = 0;

  switch (ch)
  {
    default: break;
    case '0': 
    {
      result = 0; break;
    }
    case '1': 
    {
      result = 1; break;
    }
    case '2': 
    {
      result = 2; break;
    }
    case '3': 
    {
      result = 3; break;
    }
    case '4': 
    {
      result = 4; break;
    }
    case '5': 
    {
      result = 5; break;
    }
    case '6': 
    {
      result = 6; break;
    }
    case '7': 
    {
      result = 7; break;
    }
    case '8': 
    {
      result = 8; break;
    }
    case '9': 
    {
      result = 9; break;
    }
    case 'a':
    case 'A': 
    {
      result = 10; break;
    }
    case 'b':
    case 'B': 
    {
      result = 11; break;
    }
    case 'c':
    case 'C': 
    {
      result = 12; break;
    }
    case 'd':
    case 'D': 
    {
      result = 13; break;
    }
    case 'e':
    case 'E': 
    {
      result = 14; break;
    }
    case 'f':
    case 'F': 
    {
      result = 15; break;
    }
  }

  return result;
}

INTERNAL u32
s8_u32(String8 str, b32 *error)
{
  u32 result = 0;
  u32 base = 10;

  u32 str_i = 0;

  if (str.size > 2)
  {
    if (str.str[str_i] == '0')
    {
      str_i += 1;
      if (str.str[str_i] == 'x')
      {
        base = 16;
        str_i += 1;
      }
    }
  }

  while (str_i < str.size)
  {
    u32 digit = char_to_u32((char)str.str[str_i]);

    if (digit >= base)
    {
      *error = 1;
      break;
    }

    if (result > ((MAX_U32 - digit) / base))
    {
      *error = 1; 
      break;
    }

    result = (result * base) + digit;

    str_i += 1;
  }

  return result;
}


#endif
