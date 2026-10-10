#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "mem.h"


#define SECTIONS_COUNT 100


typedef struct Section {
  void *ptr;
  size_t size;

  struct Section *prev;
  struct Section *next;
} Section;


void *arena_ptr = NULL;
int arena_len = 0;
int sections_len = 0;

Section sections[SECTIONS_COUNT];
Section *first_section;

void init_sections();
void *init_arena();
Section *new_section(size_t size, void *ptr);


void *mem(size_t size) {
  if (arena_len == 0) {
    if (!init_arena()) {
      return NULL;
    }
  }

  if (!first_section) {
    first_section = new_section(
      0,
      arena_ptr
    );

    Section *last_section = new_section(
      0, 
      arena_ptr + arena_len - 1
    );

    first_section->next = last_section;
    last_section->prev = first_section;
  }

  Section *prev_section = first_section;
  Section *cur_section = first_section->next;
  while (cur_section != NULL) {
    void *prev_section_end = prev_section->ptr + prev_section->size;

    if (prev_section_end + size <= cur_section->ptr) {
      Section *section = new_section(size, prev_section_end);
      section->prev = prev_section;
      section->next = cur_section;
      prev_section->next = section;
      cur_section->prev = section;

      printf("Alloc at %p, size %zu\n\tPrev: %p\n\tNext: %p\n",
        section->ptr,
        size,
        prev_section->ptr,
        cur_section->ptr
      );

      return section->ptr;
    }

    prev_section = cur_section;
    cur_section = cur_section->next;
  }

  return NULL;
}

int unmem(void *ptr) {
  Section *cur_section = first_section;
  while (cur_section != NULL) {
    if (cur_section->ptr == ptr) {
      if (cur_section->next == NULL || cur_section->prev == NULL) {
        return -1;
      }

      cur_section->prev->next = cur_section->next;
      cur_section->next->prev = cur_section->prev;

      printf("Freed at %p\n", ptr);
      return 1;
    }

    cur_section = cur_section->next;
  }

  printf("Failed to free at %p", ptr);
  return -1;
}


void init_sections() {
  for (int i = 0; i < SECTIONS_COUNT; i++) {
    sections[i] = (Section) { NULL, 0 };
  }

  sections_len = 0;
  first_section = NULL;
}

void *init_arena() {
  init_sections();

  arena_len = 4096 * 4;
  void *pa = mmap(NULL, arena_len, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
  if (pa == MAP_FAILED) {
    perror("mmap");
    arena_ptr = NULL;
    return NULL;
  }

  arena_ptr = pa;

  return arena_ptr;
}

Section *new_section(size_t size, void *ptr) {
  if (sections_len >= SECTIONS_COUNT) {
    return NULL;
  }

  Section *section = &sections[sections_len++];
  *section = (Section) {
    .ptr = ptr,
    .size = size,
    .prev = NULL,
    .next = NULL,
  };

  return section;
}
