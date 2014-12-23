#ifndef __LINES_H
#define __LINES_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct file_zipper {
  char *start;
  char *focus;
  size_t length;
} *file_zipper_t;

typedef struct line {
  char *start;
  size_t length;
} *line_t;

typedef void *(*transform_fn_t)(line_t);
typedef int (*compare_fn_t)(void *, void *);

typedef struct file_search {
  // void *(*transform)(line_t);
  transform_fn_t transform;
  // int (*compare)(void *, void *);
  compare_fn_t compare;
  struct file_zipper fz;
} *file_search_t;

bool move_to_start_of_line(file_zipper_t);

bool move_to_previous_line(file_zipper_t);

bool move_to_next_line(file_zipper_t);

bool move_to_middle_line(file_zipper_t);

bool find(file_search_t, void *);

char *strdupline(line_t);

#endif // __LINES_H
