/*
 * POST /v2/merchant-session
 *
 * Emitted from the MerchantSession table by tools/emit.py. Edit the table, never this
 * file: make check-emit regenerates it and fails on any difference.
 */
#include <stdio.h>
#include <stdlib.h>

#include "starkbank.h"

/*<*/
static starkbank_client *connect(void)
{
    starkbank_user *project = NULL;
    starkbank_client *client = NULL;

    if (starkbank_project_new("5656565656565656", STARKBANK_ENVIRONMENT_SANDBOX,
                              getenv("STARK_PRIVATE_KEY"), &project) != STARKBANK_OK) {
        return NULL;
    }
    if (starkbank_client_new(project, &client) != STARKBANK_OK) {
        return NULL;
    }
    starkbank_client_set_curl_transport(client);
    return client;
}

static int report(int status, starkbank_errors *errors)
{
    int index;

    fprintf(stderr, "%s\n", starkbank_strerror(status));
    for (index = 0; index < starkbank_errors_count(errors); index++) {
        fprintf(stderr, "  %s: %s\n", starkbank_errors_code_at(errors, index),
                starkbank_errors_message_at(errors, index));
    }
    starkbank_errors_free(errors);
    return 1;
}
/*>*/

int main(void)
{
    starkbank_client *client = NULL;
    starkbank_entity *merchant_session = NULL;
    starkbank_entity *created = NULL;
    starkbank_errors *errors = NULL;
    int status;

    client = connect();
    if (client == NULL) {
        return 1;
    }

    starkbank_merchant_session_new(&merchant_session);
    starkbank_entity_append_string(merchant_session, "allowedFundingTypes", "war");
    starkbank_entity_set_number(merchant_session, "expiration", 5);
    status = starkbank_merchant_session_create(client, merchant_session, &created, &errors);
    starkbank_entity_free(merchant_session);       /* post_single borrows it, unlike a list */
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    printf("created %s\n", starkbank_entity_id(created));
    starkbank_entity_free(created);

    starkbank_client_free(client);
    return 0;
}
