/*
 * The smallest program a tarball user writes: the public header, one call
 * into each layer the full bundle carries. check-dist builds it against the
 * unpacked dist tree alone, so a header or an archive member the tarball
 * forgot fails the release instead of the first person who downloads it.
 */

#include "starkbank.h"

int main(void)
{
    starkbank_user *user = NULL;

    /* An empty PEM reaches ecdsa-c's parser and fails there; strerror reaches
       starkcore. Neither result matters, only that both link. */
    if (starkbank_project_new("1", STARKBANK_ENVIRONMENT_SANDBOX, "", &user) == STARKBANK_OK) {
        starkbank_user_free(user);
    }
    return starkbank_strerror(STARKCORE_ERROR_TRANSPORT) == NULL;
}
