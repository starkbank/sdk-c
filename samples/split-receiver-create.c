/*
 * POST /v2/split-receiver
 *
 * Emitted from the SplitReceiver table by tools/emit.py. Edit the table, never this
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
    starkbank_entity *split_receiver = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;
    int status;

    client = connect();
    if (client == NULL) {
        return 1;
    }

    starkbank_split_receiver_new(&split_receiver);
    starkbank_entity_set_string(split_receiver, "name", "Arya Stark");
    starkbank_entity_set_string(split_receiver, "taxId", "012.345.678-90");
    starkbank_entity_set_string(split_receiver, "bankCode", "20018183");
    starkbank_entity_set_string(split_receiver, "branchCode", "1357-9");
    starkbank_entity_set_string(split_receiver, "accountNumber", "876543-2");
    starkbank_entity_set_string(split_receiver, "accountType", "checking");
    starkbank_list_new(&batch);
    starkbank_list_append(batch, split_receiver);        /* the list owns it from here */

    status = starkbank_split_receiver_create(client, batch, &created, &errors);
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
