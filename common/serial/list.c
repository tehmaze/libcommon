#include <talloc.h>
#include "common/serial.h"
#include "common/debug.h"

PRIVATE serial_t **serial_list_append(serial_t **list, const char *portname)
{
	void *tmp;
	unsigned int count;

	for (count = 0; list[count] != NULL; count++) {
        (void)list[count];
    }
	//if (!(tmp = realloc(list, sizeof(serial_t *) * (count + 2))))
    if (!(tmp = talloc_realloc(NULL, list, serial_t, count + 2)))
		goto fail;
    //if (!(tmp = talloc_realloc_size(NULL, *list, sizeof(serial_t *) * (count + 2))))
	list = tmp;
	if (serial_by_name(portname, &list[count]) != 0)
		goto fail;
	list[count + 1] = NULL;
	return list;

fail:
	serial_list_free(list);
	return NULL;
}

PRIVATE void serial_list_free(serial_t **list)
{
	unsigned int i;

	if (!list) {
		DEBUG("NULL list");
		return;
	}

	DEBUG("freeing port list");
	for (i = 0; list[i]; i++)
		serial_free(list[i]);
	TALLOC_FREE(list);
}
