#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "string.h"
#include "config.h"

static int _add_entry(olio_config * c, char * name, char * value)
{
  c->entry_count++;

  c->entry_names = (char**) realloc(c->entry_names,
				    sizeof(char*) * c->entry_count);
  
  c->entry_values = (char**) realloc(c->entry_values,
				     sizeof(char*) * c->entry_count);

  if (c->entry_names == NULL || c->entry_values == NULL)
    return -1;
  
  c->entry_names[c->entry_count-1] = name;
  c->entry_values[c->entry_count-1] = value;
  
  return 0;
}

static int _parse_config_line(olio_config * c, olio_string * str)
{
  uint16_t lpos;
  uint16_t tpos;
  char * name;
  char * value;
  const char * data = olio_string_contents(str);
  uint16_t len = olio_string_length(str);
  
  if (len == 0) return 0;
  
  name = (char *) malloc(sizeof(char) * len);
  value = (char *) malloc(sizeof(char) * len);
  
  if (name == NULL || value == NULL)
    return -1;
  
  lpos = 0;
  while (lpos < len && isspace(data[lpos])) lpos++;
  
  tpos = 0;
  while (lpos < len && 
	 !isspace(data[lpos]) && data[lpos] != ':')
    name[tpos++] = data[lpos++];
  name[tpos] = 0;
  
  while (lpos < len && 
	 (isspace(data[lpos]) || data[lpos] == ':'))
    lpos++;
  
  tpos = 0;
  while (lpos < len)
    value[tpos++] = data[lpos++];
  
  while (tpos > 0 && isspace(value[tpos-1])) tpos--;
  value[tpos] = 0;
  
  return _add_entry(c, name, value);
}

void olio_config_init(olio_config * c)
{
  c->entry_count = 0;
  c->entry_names = NULL;
  c->entry_values = NULL;
}

void olio_config_free(olio_config * c)
{
  uint16_t i;
  
  for (i = 0; i < c->entry_count; i++) {
    if (c->entry_names[i] != NULL) free(c->entry_names[i]);
    if (c->entry_values[i] != NULL) free(c->entry_values[i]);
  }
  
  if (c->entry_names != NULL) free(c->entry_names);
  if (c->entry_values != NULL) free(c->entry_values);
}

int olio_config_read(olio_config * c, const char * filename)
{
  OLIO_STRING_STACK(str, 128);
  int to_eol;
  FILE * fp;

  fp = fopen(filename, "r");
  if (fp == NULL) return -1;

  to_eol = 0;
  while (!feof(fp)) {
    int ci = getc(fp);
    if (ci == '#') to_eol = 1;
    else if (ci == '\n') {
      _parse_config_line(c, str);
      olio_string_clear(str);
      to_eol = 0;
    } 
    else if (!to_eol) {
      if (olio_string_append_char(str, ci))
	return -1;
    }
  }
  
  olio_string_free(str);
  
  fclose(fp);
  
  return 0;
}
