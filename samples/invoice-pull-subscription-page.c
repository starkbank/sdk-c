/*
 * GET /v2/invoice-pull-subscription
 *
 * Emitted from the InvoicePullSubscription table by tools/emit.py. Edit the table, never this
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
    starkbank_list *page = NULL;
    starkbank_errors *errors = NULL;
    char *cursor = NULL;
    int status;
    int index;

    client = connect();
    if (client == NULL) {
        return 1;
    }

    starkbank_invoice_pull_subscription_params_new(&params);
    starkbank_entity_set_number(params, "limit", 10);

    status = starkbank_invoice_pull_subscription_page(client, params, &page, &cursor, &errors);
    starkbank_entity_free(params);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    for (index = 0; index < starkbank_list_count(page); index++) {
        printf("%s\n", starkbank_entity_id(starkbank_list_at(page, index)));
    }
    printf("cursor %s\n", cursor != NULL ? cursor : "");
    starkbank_free(cursor);
    starkbank_list_free(page);

    starkbank_client_free(client);
    return 0;
}
