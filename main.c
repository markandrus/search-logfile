#define _POSIX_C_SOURCE 1
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "lines.h"

#ifndef MAP_FILE
#define MAP_FILE 0
#endif

/* Timestamp Parsing */

/* `timestamp_t` is a string containing only numbers and spaces. `timestamp_t`s
   are compared lexicographically.  */
typedef char *timestamp_t;

/* `line_t` is a string representing a (log) line, typically beginning with a
   timestamp that we could compare to a `timestamp_t`.  */
// typedef char *line_t;

bool is_digit(char);
timestamp_t parse_start_time(char *);
char *strndup(const char *, size_t);
// int line_compare(timestamp_t, line_t, char *);

/* Simply check if the given `char` is a digit.  */
bool is_digit(char c) {
  return c >= 0 && c <= 9 ? true : false;
}

/* Convert a string representing a timestamp into a `timestamp_t`.  */
timestamp_t parse_start_time(char *start_time) {
  size_t len = strlen(start_time);
  timestamp_t timestamp = strndup(start_time, len);
  if (timestamp == NULL) {
    fprintf(stderr, "Error: Could not strndup %lu characters of the string `%s'"
      " (%s).\n", len, start_time, strerror(errno));
  } else {
    for (size_t i = 0; i < len; i++)
      if (!is_digit(timestamp[i]))
        timestamp[i] = ' ';
  }
  return timestamp;
}

/* Given a `timestamp_t`, check if a `line_t` comes before (-1) or after (1)
   it.  */
/*
int line_compare(timestamp_t timestamp, line_t line, char *hi) {
  char a, b;
  bool c, d;
  while (line < hi && (a = *line) != '\n' && (b = *timestamp) != '\0') {
    c = is_digit(a);
    d = is_digit(b);
    if (c ^ b) {
      // Bad format; skip.
      return 1;
    } else if (c && b) {
      // Both digits; check.
      if (a > b)
        return -1;
      else if (a < b)
        return 1;
    }
    timestamp++;
    line++;
  }
  if ((line == hi || a == '\n') && b != '\0')
    // Line was short; skip.
    return 1;
  return 0;
}
*/

/* Binary Search */

char *md_to_newline(char *, char *);
char *_locate_word(char *, size_t, char *, char *);
char *locate_word(char *, char *, size_t);

/* This function returns the start of a new line to the right of our binary
   search's midpoint `md`, not exceeding the pointer `hi`.  If the start of a
   new line is not found, we return the pointer `hi`.  */
char *md_to_newline(char *md, char *hi) {
  char *newline = memchr(md, '\n', (size_t) hi - (size_t) md);
  if (newline != NULL && newline < hi) {
    return newline < hi ? newline + 1 : newline;
  } else {
    return hi;
  }
}

/* Recursive binary search function: we look for the string `word` of length
   `wordlen` in the address space demarcated by pointers `lo` (inclusive) and
   `hi` (exclusive).  */
char *_locate_word(char *word, size_t wordlen, char *lo, char *hi) {
  char *md = lo + (hi - lo) / 2;
  bool last = false;
  if ((md = md_to_newline(md, hi)) == NULL) {
    // Could not find word.
    return NULL;
  } else if (md == hi) {
    md = lo;
    last = true;
  }
  char *end = md_to_newline(md, hi);
  size_t len = (size_t) end;
  if (len != (size_t) NULL) {
    len -= (size_t) md + 1;
  } else {
    len = (size_t) hi - (size_t) md;
  }
  int res = strncasecmp(word, md, len > wordlen ? len : wordlen);
  fprintf(stderr, "%p %p %p %.*s (%d)\n", lo, md, hi, (int) len, md, res);
  if (res == 0 && len == wordlen) {
    // Found the word.
    return md;
  } else if (res < 0) {
    return _locate_word(word, wordlen, lo, md);
  } else if (last == false) {
    return _locate_word(word, wordlen, md, hi);
  } else {
    // Could not find word.
    return NULL;
  }
}

/* Entry point for our binary search function: we look for the string `word` in
   a newline-separated dictionary `dict` of filesize `filesize`.  */
char *locate_word(char *word, char *dict, size_t filesize) {
  if (word == NULL || dict == NULL || filesize == 0) {
    return NULL;
  }
  return _locate_word(word, strlen(word), dict, dict + filesize);
}

/*line_t _locate_line(timestamp_t start_time, line_t lo, line_t hi) {
  line_t md = lo + (hi - lo) / 2;
  bool last = false;
  if ((md = md_to_newline(md, hi)) == NULL) {
    // Could not find line.
    return NULL;
  } else if (md == hi) {
    md = lo;
    last = true;
  }
  line_t end = md_to_newline(md, hi);
  size_t len = (size_t) end;
  if (len != (size_t) NULL) {
    len -= (size_t) md + 1;
  } else {
    len = (size_t) hi - (size_t) md;
  }
  int res = line_compare(start_time, md, hi);
  fprintf(stderr, "%p %p %p %.*s (%d)\n", lo, md, hi, (int) len, md, res);
  if (res == 0) {
  } else {
  }
}

line_t locate_line(timestamp_t start_time, char *log, size_t filesize) {
  if (start_time == NULL || log == NULL || filesize == 0) {
    return NULL;
  }
  return _locate_line(start_time, log, log + filesize);
}*/

/* Main */

/* This main function is monolothic; but, once we've successfully loaded our
   file (or handled the error conditions), we call out to the binary search
   function `locate_word` defined above.  */
int main(int argc, char *argv[]) {

  int retval = -1;
  char *filename = NULL;
  FILE *stream = NULL;
  char *file = NULL;
  size_t filesize = 0;

  // Check the arguments.
  if (argc != 3) {
    fprintf(stderr, "Usage: %s WORD FILE\n", argv[0]);
    goto ERROR;
  }
  // timestamp_t start_time = parse_start_time(argv[1]);
  char *word = argv[1];
  filename = argv[2];

  // Attempt to fopen the file.
  if ((stream = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Error: Could not fopen file `%s' (%s).\n", filename,
      strerror(errno));
    goto ERROR;
  }

  // Attempt to fstat the open file.
  int fildes = fileno(stream);
  struct stat filestat;
  if (fstat(fildes, &filestat) == -1) {
    fprintf(stderr, "Error: Could not fstat open file `%s' (%s).\n", filename,
      strerror(errno));
    goto ERROR;
  }
  filesize = (size_t) filestat.st_size;

  // Attempt to mmap the open file.
  if ((file = mmap(NULL, filesize, PROT_READ, MAP_FILE | MAP_SHARED, fildes,
      0)) == MAP_FAILED) {
    fprintf(stderr, "Error: Could not mmap %lu bytes of open file `%s' "
      "(%s).\n", filesize, filename, strerror(errno));
    goto ERROR;
  }

  fprintf(stderr, "%p %s\n\n", file, filename);
  /*if (locate_word(word, file, filesize) == NULL) {
    fprintf(stderr, "Error: Could not find word \"%s\" in open file `%s'.\n",
      word, filename);
    goto ERROR;
  }*/

  struct file_search fs = {
    .transform = (transform_fn_t) strdupline,
    .compare = (compare_fn_t) strcasecmp,
    .fz = {
      .start = file,
      .focus = NULL,
      .length = filesize
    }
  };

  if (find(&fs, word)) {
    // Do nothing
  } else {
    fprintf(stderr, "Error: Could not find word \"%s\" in open file `%s'.\n",
      word, filename);
    goto ERROR;
  }

  retval = 0;
ERROR:
  if (file != NULL && file != MAP_FAILED) {
    if (munmap(file, (size_t) filesize) == -1) {
      fprintf(stderr, "Error: Could not munmap %lu bytes of open file `%s' "
        "(%s).\n", filesize, filename, strerror(errno));
    }
  }
  if (stream != NULL) {
    if (fclose(stream) == EOF) {
      fprintf(stderr, "Error: Could not fclose open file `%s' (%s).\n",
        filename, strerror(errno));
    }
  }
  return retval;
}
