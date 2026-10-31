/*
 * core-c#1: two fixes that stayed invisible offline. Under a comma-decimal
 * LC_NUMERIC the request body must still spell 2.5 with a dot, or the API
 * rejects it; the offline suite was green on a machine without pt_BR, which
 * is why a missing locale is a skip here and never an ok. And a query walk
 * must end on the API's empty cursor instead of asking for page two forever.
 */

#include <locale.h>
#include <stdio.h>

#include "harness.h"

/* The walk asks for one id, so it ends after a handful of calls at most; past
   this many it is the endless-cursor bug, reported instead of hanging. */
#define WALK_GUARD 10

static starkbank_entity *decimalUnderPtBr(const starkbank_client *client)
{
    const char *name = "a decimal field signs with a dot under pt_BR";
    starkbank_entity *invoice;
    double fine = 0.0;

    if (setlocale(LC_NUMERIC, "pt_BR.UTF-8") == NULL) {
        skip(name, "pt_BR.UTF-8 not installed");
        return NULL;
    }
    invoice = createInvoice(client, name, 2.5, 1.3);
    setlocale(LC_NUMERIC, "C");
    if (invoice == NULL) {
        return NULL;
    }
    check(name, starkbank_entity_number(invoice, STARKBANK_INVOICE_FINE, &fine) == STARKBANK_OK
          && fine == 2.5, "the API did not echo fine 2.5");
    return invoice;
}

static void queryWalkEnds(const starkbank_client *client, const starkbank_entity *invoice)
{
    const char *name = "a query walk ends";
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;
    int calls = 0;
    int status = STARKBANK_OK;

    starkbank_entity_string(invoice, STARKBANK_INVOICE_ID, &id);
    starkbank_invoice_params_new(&params);
    starkbank_entity_append_string(params, "ids", id);
    status = starkbank_invoice_query(client, params, 0, &iter);
    while (status == STARKBANK_OK && calls < WALK_GUARD) {
        calls++;
        status = starkbank_iter_next(iter, &item, &errors);
        if (status != STARKBANK_OK || item == NULL) {
            break;
        }
    }
    if (requireOk(name, status, errors)) {
        check(name, item == NULL, "the walk was still yielding after the guard");
    }
    starkbank_iter_free(iter);
    starkbank_entity_free(params);
}

void sandboxCore(const starkbank_client *client, const starkbank_user *organization)
{
    starkbank_entity *invoice;

    (void)organization;
    invoice = decimalUnderPtBr(client);
    if (invoice == NULL) {
        invoice = createInvoice(client, "a query walk ends", 0.0, 0.0);
    }
    if (invoice == NULL) {
        return;
    }
    queryWalkEnds(client, invoice);
    cancelInvoice(client, invoice);
    starkbank_entity_free(invoice);
}
