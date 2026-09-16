/*
 * POST /v2/payment-preview
 *
 * Emitted from the PaymentPreview table by tools/emit.py. Edit the table, never this
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
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_entity *payment_preview = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;
    int status;

    client = connect();
    if (client == NULL) {
        return 1;
    }

    starkbank_payment_preview_new(&payment_preview);
    starkbank_entity_set_string(payment_preview, "id", "34191.09008 63571.277308 71444.640008 5 81960000000062");
    starkbank_list_new(&batch);
    starkbank_list_append(batch, payment_preview);        /* the list owns it from here */

    status = starkbank_payment_preview_create(client, batch, &created, &errors);
    starkbank_list_free(batch);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    starkbank_entity_string(starkbank_list_at(created, 0), "id", &id);
    printf("created %s\n", id);
    starkbank_list_free(created);

    starkbank_client_free(client);
    return 0;
}
