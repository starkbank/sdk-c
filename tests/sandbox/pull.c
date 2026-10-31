/*
 * sdk-c#13: InvoicePullSubscription and InvoicePullRequest through create,
 * log and delete, BoletoHolmes, DynamicBrcode by uuid, and PaymentRequest
 * with two payment types, whose payment leg is written as raw JSON. A
 * PaymentRequest needs a cost center, so it skips without SANDBOX_CENTER_ID.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "harness.h"

static void setDay(starkbank_entity *entity, const char *field, int daysFromToday)
{
    time_t when = time(NULL) + (time_t)daysFromToday * 24 * 3600;
    struct tm day = *localtime(&when);

    starkbank_entity_set_date(entity, field, day.tm_year + 1900, day.tm_mon + 1, day.tm_mday);
}

/* sdk-python's tests/utils/invoicePullSubscription.py, type qrcodeAndPayment,
   the type its TestInvoicePullRequestCreate pulls an invoice against. */
static starkbank_entity *createSubscription(const starkbank_client *client)
{
    starkbank_entity *subscription = NULL;
    char externalId[48];

    snprintf(externalId, sizeof(externalId), "sdk-c-%ld-%d", (long)time(NULL), rand());
    starkbank_invoice_pull_subscription_new(&subscription);
    setDay(subscription, "start", 5);
    setDay(subscription, "end", 35);
    starkbank_entity_set_string(subscription, "interval", "month");
    starkbank_entity_set_string(subscription, "pullMode", "manual");
    starkbank_entity_set_string(subscription, "type", "qrcodeAndPayment");
    starkbank_entity_set_number(subscription, "pullRetryLimit", 3);
    starkbank_entity_set_amount(subscription, "amount", 0);
    starkbank_entity_set_amount(subscription, "amountMinLimit", 5000);
    starkbank_entity_set_string(subscription, "displayDescription", "Dragon Travel Fare");
    starkbank_entity_set_string(subscription, "externalId", externalId);
    starkbank_entity_set_string(subscription, "referenceCode", "contract-12345");
    starkbank_entity_set_string(subscription, "name", "John Snow");
    starkbank_entity_set_string(subscription, "taxId", "012.345.678-90");
    starkbank_entity_set_json_raw(subscription, "data", "{\"amount\": 400000}");
    return createOne(client, "invoice pull subscription create and delete",
                     starkbank_invoice_pull_subscription_create, subscription);
}

/* sdk-python's tests/utils/invoicePullRequest.py against a fresh invoice. */
static void pullRequest(const starkbank_client *client, const char *subscriptionId)
{
    const char *name = "invoice pull request create and delete";
    starkbank_entity *invoice = createInvoice(client, name, 0.0, 0.0);
    starkbank_entity *request = NULL;
    starkbank_entity *created;
    const char *invoiceId = NULL;
    const char *id = NULL;

    if (invoice == NULL) {
        return;
    }
    starkbank_entity_string(invoice, STARKBANK_INVOICE_ID, &invoiceId);
    starkbank_invoice_pull_request_new(&request);
    starkbank_entity_set_string(request, "subscriptionId", subscriptionId);
    starkbank_entity_set_string(request, "invoiceId", invoiceId);
    starkbank_entity_set_string(request, "attemptType", "default");
    setDay(request, "due", 5);
    created = createOne(client, name, starkbank_invoice_pull_request_create, request);
    if (created != NULL && starkbank_entity_string(created, "id", &id) == STARKBANK_OK) {
        expectLog(client, "invoice pull request log query", starkbank_invoice_pull_request_log_params_new,
                  starkbank_invoice_pull_request_log_query, "requestIds", id);
        deleteOne(client, name, starkbank_invoice_pull_request_delete, id);
    }
    starkbank_entity_free(created);
    cancelInvoice(client, invoice);
    starkbank_entity_free(invoice);
}

static void subscriptionAndRequest(const starkbank_client *client)
{
    starkbank_entity *subscription = createSubscription(client);
    const char *id = NULL;

    if (subscription == NULL) {
        return;
    }
    starkbank_entity_string(subscription, "id", &id);
    pullRequest(client, id);
    expectLog(client, "invoice pull subscription log query",
              starkbank_invoice_pull_subscription_log_params_new,
              starkbank_invoice_pull_subscription_log_query, "subscriptionIds", id);
    deleteOne(client, "invoice pull subscription create and delete",
              starkbank_invoice_pull_subscription_delete, id);
    starkbank_entity_free(subscription);
}

/* A registered boleto's id or line, borrowed from iter, which the caller frees;
   NULL after a skip or a FAIL under name. */
static const char *registeredBoleto(const starkbank_client *client, const char *name,
                                    const char *field, starkbank_iter **iter)
{
    starkbank_entity *params = NULL;
    const starkbank_entity *boleto = NULL;
    starkbank_errors *errors = NULL;
    const char *value = NULL;
    int status;

    starkbank_boleto_params_new(&params);
    starkbank_entity_set_string(params, STARKBANK_BOLETO_STATUS, STARKBANK_BOLETO_STATUS_REGISTERED);
    status = starkbank_boleto_query(client, params, 1, iter);
    starkbank_entity_free(params);
    if (status == STARKBANK_OK) {
        status = starkbank_iter_next(*iter, &boleto, &errors);
    }
    if (requireOk(name, status, errors)
            && (boleto == NULL || starkbank_entity_string(boleto, field, &value) != STARKBANK_OK)) {
        skip(name, "no registered boleto in this workspace");
    }
    return value;
}

static void boletoHolmes(const starkbank_client *client)
{
    const char *name = "boleto holmes create";
    starkbank_iter *iter = NULL;
    starkbank_entity *holmes = NULL;
    starkbank_entity *created;
    const char *boletoId = registeredBoleto(client, name, STARKBANK_BOLETO_ID, &iter);
    const char *id = NULL;

    if (boletoId != NULL) {
        starkbank_boleto_holmes_new(&holmes);
        starkbank_entity_set_string(holmes, "boletoId", boletoId);
        starkbank_entity_append_string(holmes, "tags", "elementary");
        created = createOne(client, name, starkbank_boleto_holmes_create, holmes);
        if (created != NULL && starkbank_entity_string(created, "id", &id) == STARKBANK_OK) {
            expectLog(client, "boleto holmes log query", starkbank_boleto_holmes_log_params_new,
                      starkbank_boleto_holmes_log_query, "holmesIds", id);
        }
        starkbank_entity_free(created);
    }
    starkbank_iter_free(iter);
}

static void dynamicBrcode(const starkbank_client *client)
{
    const char *name = "dynamic brcode create and get by uuid";
    starkbank_entity *brcode = NULL;
    starkbank_entity *created;
    starkbank_entity *fetched = NULL;
    starkbank_errors *errors = NULL;
    const char *uuid = NULL;
    const char *fetchedUuid = NULL;

    starkbank_dynamic_brcode_new(&brcode);
    starkbank_entity_set_amount(brcode, "amount", 400);
    starkbank_entity_set_number(brcode, "expiration", 3600);
    starkbank_entity_set_string(brcode, "displayDescription", "Payment for service #1234");
    created = createOne(client, name, starkbank_dynamic_brcode_create, brcode);
    if (created == NULL || starkbank_entity_string(created, STARKBANK_DYNAMIC_BRCODE_UUID, &uuid)
            != STARKBANK_OK) {
        starkbank_entity_free(created);
        return;
    }
    if (requireOk(name, starkbank_dynamic_brcode_get(client, uuid, &fetched, &errors), errors)) {
        check(name, starkbank_entity_string(fetched, STARKBANK_DYNAMIC_BRCODE_UUID, &fetchedUuid)
                  == STARKBANK_OK && strcmp(fetchedUuid, uuid) == 0, "a different brcode came back");
    }
    starkbank_entity_free(fetched);
    starkbank_entity_free(created);
}

/* Takes leg, dumps it into a PaymentRequest of type and creates that. */
static void paymentRequest(const starkbank_client *client, const char *name, const char *centerId,
                           const char *type, starkbank_entity *leg)
{
    starkbank_entity *request = NULL;
    starkbank_entity *created;
    char *json = NULL;
    size_t length = 0;

    starkbank_entity_dump(leg, &json, &length);
    starkbank_entity_free(leg);
    starkbank_payment_request_new(&request);
    starkbank_entity_set_string(request, STARKBANK_PAYMENT_REQUEST_CENTER_ID, centerId);
    starkbank_entity_set_string(request, STARKBANK_PAYMENT_REQUEST_TYPE, type);
    starkbank_entity_set_json_raw(request, STARKBANK_PAYMENT_REQUEST_PAYMENT, json);
    starkbank_free(json);
    created = createOne(client, name, starkbank_payment_request_create, request);
    starkbank_entity_free(created);
}

static void paymentRequests(const starkbank_client *client)
{
    const char *withTransfer = "payment request create with Transfer";
    const char *withBoleto = "payment request create with BoletoPayment";
    const char *centerId = getenv("SANDBOX_CENTER_ID");
    starkbank_entity *leg = NULL;
    starkbank_iter *iter = NULL;
    const char *line;

    if (centerId == NULL || centerId[0] == '\0') {
        skip(withTransfer, "SANDBOX_CENTER_ID is not set");
        skip(withBoleto, "SANDBOX_CENTER_ID is not set");
        return;
    }
    /* sdk-python's tests/utils/transfer.py example. */
    starkbank_transfer_new(&leg);
    starkbank_entity_set_amount(leg, "amount", 1000);
    starkbank_entity_set_string(leg, "name", "Jo\303\243o");
    starkbank_entity_set_string(leg, "taxId", "012.345.678-90");
    starkbank_entity_set_string(leg, "bankCode", "18236120");
    starkbank_entity_set_string(leg, "branchCode", "0001");
    starkbank_entity_set_string(leg, "accountNumber", "10000-0");
    starkbank_entity_set_string(leg, "accountType", "checking");
    paymentRequest(client, withTransfer, centerId, "transfer", leg);
    line = registeredBoleto(client, withBoleto, STARKBANK_BOLETO_LINE, &iter);
    if (line != NULL) {
        starkbank_boleto_payment_new(&leg);
        starkbank_entity_set_string(leg, STARKBANK_BOLETO_PAYMENT_LINE, line);
        starkbank_entity_set_string(leg, STARKBANK_BOLETO_PAYMENT_TAX_ID, "20.018.183/0001-80");
        starkbank_entity_set_string(leg, STARKBANK_BOLETO_PAYMENT_DESCRIPTION, "sdk-c sandbox suite");
        paymentRequest(client, withBoleto, centerId, "boleto-payment", leg);
    }
    starkbank_iter_free(iter);
}

void sandboxPull(const starkbank_client *client, const starkbank_user *organization)
{
    (void)organization;
    subscriptionAndRequest(client);
    boletoHolmes(client);
    dynamicBrcode(client);
    paymentRequests(client);
}
