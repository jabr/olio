#ifndef _OLIO_CONFIG_H_
#define _OLIO_CONFIG_H_

#include <inttypes.h>

typedef struct _olio_config {
	uint16_t entry_count;
	char ** entry_names;
	char ** entry_values;
} olio_config;

#ifdef __cplusplus
extern "C" {
#endif

void olio_config_init(olio_config *);
void olio_config_free(olio_config *);
int olio_config_read(olio_config *, const char * filename);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* _OLIO_CONFIG_H_ */
