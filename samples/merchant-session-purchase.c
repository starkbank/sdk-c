/*
 * POST /v2/merchant-session/:id/purchase
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
    starkbank_entity *purchase = NULL;
    starkbank_entity *created = NULL;
    starkbank_errors *errors = NULL;
    int status;

    client = connect();
    if (client == NULL) {
        return 1;
    }

    starkbank_purchase_new(&purchase);
    starkbank_entity_set_amount(purchase, "amount", 400000);
    starkbank_entity_set_string(purchase, "cardExpiration", "example");
    starkbank_entity_set_string(purchase, "cardNumber", "example");
    starkbank_entity_set_string(purchase, "cardSecurityCode", "example");
    starkbank_entity_set_string(purchase, "holderName", "example");
    starkbank_entity_set_string(purchase, "fundingType", "example");
    status = starkbank_merchant_session_purchase(client, "5656565656565656", purchase, &created, &errors);
    starkbank_entity_free(purchase);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    printf("created %s\n", starkbank_entity_id(created));
    starkbank_entity_free(created);

    starkbank_client_free(client);
    return 0;
}
