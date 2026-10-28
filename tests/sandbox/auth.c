/*
 * ecdsa-c#1: a request signed by this build's ECDSA is one the sandbox
 * accepts, on a read and on a write. A signature the API rejects comes back
 * as invalidSignature or invalidAccessSignature, which requireOk prints.
 */

#include <stdio.h>

#include "harness.h"

static void signedGet(const starkbank_client *client)
{
    starkbank_entity *balance = NULL;
    starkbank_errors *errors = NULL;
    double amount = 0.0;

    if (!requireOk("a signed GET is accepted", starkbank_balance_get(client, &balance, &errors),
                   errors)) {
        return;
    }
    check("a signed GET is accepted",
          starkbank_entity_amount(balance, STARKBANK_BALANCE_AMOUNT, &amount) == STARKBANK_OK,
          "the balance carries no amount");
    starkbank_entity_free(balance);
}

static void signedPost(const starkbank_client *client)
{
    starkbank_entity *invoice = createInvoice(client, "a signed POST is accepted", 0.0, 0.0);
    const char *id = NULL;

    if (invoice == NULL) {
        return;
    }
    check("a signed POST is accepted",
          starkbank_entity_string(invoice, STARKBANK_INVOICE_ID, &id) == STARKBANK_OK,
          "the created invoice carries no id");
    cancelInvoice(client, invoice);
    starkbank_entity_free(invoice);
}

void sandboxAuth(const starkbank_client *client, const starkbank_user *organization)
{
    (void)organization;
    signedGet(client);
    signedPost(client);
}
