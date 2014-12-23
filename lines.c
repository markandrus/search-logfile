#include <stdlib.h>
#include "lines.h"

/*
 * Reverse memchr()
 * Find the last occurrence of 'c' in the buffer 's' of size 'n'.
 */
void *memrchr(const void *s, int c, size_t n) {
  const unsigned char *cp;
  if (n != 0) {
    cp = (unsigned char *) s + n;
    do {
      if (*(--cp) == (unsigned char) c) {
        return (void *) cp;
      }
    } while (--n != 0);
  }
  return NULL;
}

bool move_to_start_of_line(file_zipper_t fz) {
  size_t n = fz->focus - fz->start;
  char *newline = memrchr(fz->start, '\n', n);
  if (newline == NULL) {
    return false;
  }

  fz->focus = newline;
  do {
    fz->focus++;
  } while(*fz->focus == '\n');

  return true;
}

bool move_to_previous_line(file_zipper_t fz) {
  char *focus = fz->focus;
  if (!move_to_start_of_line(fz)) {
    goto RESET;
  }
  fz->focus -= 1;
  if (!move_to_start_of_line(fz)) {
    goto RESET;
  }
  return true;
RESET:
  fz->focus = focus;
  return false;
}

bool move_to_next_line(file_zipper_t fz) {
  char *eof = fz->start + fz->length;
  char *newline = memchr(fz->focus, '\n', eof - fz->focus);
  if (newline == NULL) {
    return false;
  }
  fz->focus = newline;
  do {
    fz->focus++;
  } while (*fz->focus == '\n');
  return false;
}

bool move_to_middle_line(file_zipper_t fz) {
  char *lo = fz->start;
  char *hi = fz->start + fz->length;
  // char *focus = fz->focus;
  fz->focus = lo + (hi - lo) / 2;
  move_to_start_of_line(fz);
  /*if (!move_to_start_of_line(fz)) {
    fz->focus = focus;
    return false;
  }*/
  return true;
}

bool find(file_search_t fs, void *value) {
  file_zipper_t fz = &fs->fz;
  if (!move_to_middle_line(fz)) {
    fprintf(stderr, "find: No midpoint\n");
    return false;
  } else if (!fz->length) {
    fprintf(stderr, "find: exhausted\n");
    return false;
  } else {
    fprintf(stderr, "find: %lu\n", fz->length);
  }

  void *line_value = fs->transform((line_t) &fz->focus);
  int res = fs->compare(value, line_value);
  free(line_value);

  if (res == 0) {
    fprintf(stderr, "find: Found\n");
    return true;
  } else if (res < 0) {
    // Recurse left
    fz->length = fz->focus - fz->start;
    fprintf(stderr, "find: Recursing left\n");
    return find(fs, value);
  } else {
    // Recurse right
    /*if (!move_to_next_line(fz)) {
      fprintf(stderr, "find: No next line\n");
      return false;
    }*/
    fz->length = fz->length - (fz->focus - fz->start);
    fz->start = fz->focus;
    fprintf(stderr, "find: Recursing right\n");
    return find(fs, value);
  }
}

char *strdupline(line_t line) {
  char *newline = memchr(line->start, '\n', line->length);
  size_t n = newline == NULL
           ? line->length
           : (size_t) newline - (size_t) line->start;
  char *copy = (char *) calloc(1, sizeof(char) * (n + 1));
  memcpy(copy, line->start, sizeof(char) * n);
  fprintf(stderr, "strdupline: %s\n", copy);
  return copy;
}
