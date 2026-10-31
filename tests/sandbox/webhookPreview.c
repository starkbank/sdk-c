/*
 * sdk-c#1: Webhook's create, get and delete, and PaymentPreview, whose
 * payment field hydrates as a different table for each type. Each preview is
 * its own call, so one id the sandbox refuses does not hide the other three.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "harness.h"

/* sdk-python's tests/utils/nonBoletoPayment.py builds these from one example
   bar code, with the business segment at 1, the amount at 4 and the business
   code at 15; here its random picks are fixed to the first tax business
   (5701, segment 1), a utility one the sandbox preview recognises (19,
   segment 2; it answers 846, the first, with "Unknown payment type") and
   1000 cents. */
#define TAX_BAR_CODE     "81660000000100057010074119002551100010601813"
#define UTILITY_BAR_CODE "82660000000100000190074119002551100010601813"

static void webhookLifecycle(const starkbank_client *client)
{
    starkbank_entity *webhook = NULL;
    starkbank_entity *created = NULL;
    starkbank_entity *fetched = NULL;
    starkbank_entity *deleted = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;
    const char *fetchedId = NULL;
    char url[64];

    snprintf(url, sizeof(url), "https://webhook.site/sdk-c-%ld", (long)time(NULL));
    starkbank_webhook_new(&webhook);
    starkbank_entity_set_string(webhook, STARKBANK_WEBHOOK_URL, url);
    starkbank_entity_append_string(webhook, STARKBANK_WEBHOOK_SUBSCRIPTIONS,
                                   STARKBANK_WEBHOOK_SUBSCRIPTION_TRANSFER);
    if (!requireOk("webhook create", starkbank_webhook_create(client, webhook, &created, &errors),
                   errors)) {
        starkbank_entity_free(webhook);
        return;
    }
    starkbank_entity_free(webhook);
    check("webhook create", starkbank_entity_string(created, STARKBANK_WEBHOOK_ID, &id) == STARKBANK_OK,
          "the webhook carries no id");
    if (id == NULL) {
        starkbank_entity_free(created);
        return;
    }
    errors = NULL;
    if (requireOk("webhook get by id", starkbank_webhook_get(client, id, &fetched, &errors), errors)) {
        check("webhook get by id",
              starkbank_entity_string(fetched, STARKBANK_WEBHOOK_ID, &fetchedId) == STARKBANK_OK
              && strcmp(fetchedId, id) == 0, "a different webhook came back");
    }
    errors = NULL;
    if (requireOk("webhook delete", starkbank_webhook_delete(client, id, &deleted, &errors), errors)) {
        check("webhook delete", 1, NULL);
    }
    starkbank_entity_free(deleted);
    starkbank_entity_free(fetched);
    starkbank_entity_free(created);
}

static void preview(const starkbank_client *client, const char *name, const char *id,
                    const char *resource)
{
    starkbank_entity *request = NULL;
    starkbank_list *batch = NULL;
    starkbank_list *previews = NULL;
    starkbank_errors *errors = NULL;
    const starkbank_entity *payment = NULL;
    const char *tag;
    int status;

    starkbank_payment_preview_new(&request);
    starkbank_entity_set_string(request, STARKBANK_PAYMENT_PREVIEW_ID, id);
    starkbank_list_new(&batch);
    starkbank_list_append(batch, request);
    status = starkbank_payment_preview_create(client, batch, &previews, &errors);
    starkbank_list_free(batch);
    if (!requireOk(name, status, errors)) {
        return;
    }
    if (starkbank_list_count(previews) == 1) {
        starkbank_entity_entity(starkbank_list_at(previews, 0), STARKBANK_PAYMENT_PREVIEW_PAYMENT,
                                &payment);
    }
    tag = payment != NULL ? starkbank_entity_resource(payment) : NULL;
    check(name, tag != NULL && strcmp(tag, resource) == 0, tag != NULL ? tag : "no payment came back");
    starkbank_list_free(previews);
}

static void previewBoleto(const starkbank_client *client, const char *name,
                          const starkbank_entity *boleto)
{
    const char *line = NULL;

    if (boleto == NULL || starkbank_entity_string(boleto, STARKBANK_BOLETO_LINE, &line) != STARKBANK_OK) {
        skip(name, "no registered boleto in this workspace");
        return;
    }
    preview(client, name, line, "PaymentPreview.BoletoPreview");
}

static void boletoPreview(const starkbank_client *client)
{
    const char *name = "preview hydrates BoletoPreview";
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *boleto = NULL;
    starkbank_errors *errors = NULL;
    int status;

    starkbank_boleto_params_new(&params);
    starkbank_entity_set_string(params, STARKBANK_BOLETO_STATUS, STARKBANK_BOLETO_STATUS_REGISTERED);
    status = starkbank_boleto_query(client, params, 1, &iter);
    if (status == STARKBANK_OK) {
        status = starkbank_iter_next(iter, &boleto, &errors);
    }
    if (requireOk(name, status, errors)) {
        previewBoleto(client, name, boleto);
    }
    starkbank_iter_free(iter);
    starkbank_entity_free(params);
}

static void previewInvoice(const starkbank_client *client, const char *name,
                           const starkbank_entity *invoice)
{
    const char *brcode = NULL;

    if (starkbank_entity_string(invoice, STARKBANK_INVOICE_BRCODE, &brcode) != STARKBANK_OK) {
        check(name, 0, "the invoice carries no brcode");
        return;
    }
    preview(client, name, brcode, "PaymentPreview.BrcodePreview");
}

static void brcodePreview(const starkbank_client *client)
{
    const char *name = "preview hydrates BrcodePreview";
    starkbank_entity *invoice = createInvoice(client, name, 0.0, 0.0);

    if (invoice == NULL) {
        return;
    }
    previewInvoice(client, name, invoice);
    cancelInvoice(client, invoice);
    starkbank_entity_free(invoice);
}

void sandboxWebhookPreview(const starkbank_client *client, const starkbank_user *organization)
{
    (void)organization;
    webhookLifecycle(client);
    brcodePreview(client);
    boletoPreview(client);
    preview(client, "preview hydrates TaxPreview", TAX_BAR_CODE, "PaymentPreview.TaxPreview");
    preview(client, "preview hydrates UtilityPreview", UTILITY_BAR_CODE,
            "PaymentPreview.UtilityPreview");
}
