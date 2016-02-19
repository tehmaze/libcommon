#include <unistd.h>
#include <talloc.h>
#include "common/socket.h"
#include "common/platform.h"

PRIVATE socket_t *socket_new(uint8_t v, uint32_t scope_id)
{
    socket_t *s = talloc_zero(NULL, socket_t);
    if (s != NULL) {
        s->v = v;
        s->scope_id = scope_id;
    }
    return s;
}

PRIVATE void socket_free(socket_t *s)
{
    if (s != NULL && s->fd >= 0) {
        close(s->fd);
    }
    TALLOC_FREE(s);
}
