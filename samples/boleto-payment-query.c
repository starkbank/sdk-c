/*
 * GET /v2/boleto-payment
 *
 * Emitted from the BoletoPayment table by tools/emit.py. Edit the table, never this
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
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *found = NULL;
    starkbank_errors *errors = NULL;
    int status;

    client = connect();
    if (client == NULL) {
        return 1;
    }

    starkbank_boleto_payment_params_new(&params);
    starkbank_entity_set_string(params, "status", "paid");
    status = starkbank_boleto_payment_query(client, params, 100, &iter);
    if (status != STARKBANK_OK) {
        starkbank_entity_free(params);
        starkbank_client_free(client);
        return report(status, NULL);
    }
    /* A network error surfaces here and not at the call above, exactly as in
       starkcore_stream: the first page is fetched by the first next(). */
    while ((status = starkbank_iter_next(iter, &found, &errors)) == STARKBANK_OK
           && found != NULL) {
        printf("%s\n", starkbank_entity_id(found));
    }
    starkbank_iter_free(iter);
    starkbank_entity_free(params);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }

    starkbank_client_free(client);
    return 0;
}
