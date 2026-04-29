#ifndef HTTP_PARSING_H
#define HTTP_PARSING_H

#include "chunked_string.h"
#include <stdlib.h>
#include <string.h>

int validate_request(char *, struct chunked_string *, size_t);
int parse_uri(struct chunked_string *, struct chunked_string *,
              struct chunked_string *, int *);
#endif
