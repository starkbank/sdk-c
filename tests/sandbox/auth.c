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

/* sdk-python's tests/utils/invoice.py example: an amount, a CPF and a name are
   the REQUIRED fields; the rest of its example is optional. */
static void signedPost(const starkbank_client *client)
{
    starkbank_entity *invoice = NULL;
    starkbank_entity *patch = NULL;
    starkbank_entity *canceled = NULL;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;

    starkbank_invoice_new(&invoice);
    starkbank_entity_set_amount(invoice, STARKBANK_INVOICE_AMOUNT, 400);
    starkbank_entity_set_string(invoice, STARKBANK_INVOICE_TAX_ID, "012.345.678-90");
    starkbank_entity_set_string(invoice, STARKBANK_INVOICE_NAME, "Iron Bank S.A.");
    starkbank_list_new(&batch);
    starkbank_list_append(batch, invoice);
    if (!requireOk("a signed POST is accepted",
                   starkbank_invoice_create(client, batch, &created, &errors), errors)) {
        starkbank_list_free(batch);
        return;
    }
    check("a signed POST is accepted",
          starkbank_list_count(created) == 1
          && starkbank_entity_string(starkbank_list_at(created, 0), STARKBANK_INVOICE_ID, &id)
              == STARKBANK_OK, "the created invoice carries no id");
    /* Invoices have no delete: canceling is the cleanup the API allows. */
    if (id != NULL) {
        starkbank_invoice_params_new(&patch);
        starkbank_entity_set_string(patch, STARKBANK_INVOICE_STATUS, STARKBANK_INVOICE_STATUS_CANCELED);
        errors = NULL;
        requireOk("the invoice is canceled afterwards",
                  starkbank_invoice_update(client, id, patch, &canceled, &errors), errors);
        starkbank_entity_free(patch);
        starkbank_entity_free(canceled);
    }
    starkbank_list_free(created);
    starkbank_list_free(batch);
}

void sandboxAuth(const starkbank_client *client, const starkbank_user *organization)
{
    (void)organization;
    signedGet(client);
    signedPost(client);
}
