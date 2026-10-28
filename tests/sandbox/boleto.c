/*
 * sdk-c#3: Boleto with its PDF options and logs, BoletoPayment and
 * BrcodePayment with a rule, each with its log. The boleto is paid before it
 * is deleted, because a deleted boleto has no line left to pay.
 */

#include <string.h>

#include "harness.h"

/* sdk-python's tests/utils/boleto.py example, REQUIRED fields only. */
static starkbank_entity *createBoleto(const starkbank_client *client)
{
    starkbank_entity *boleto = NULL;
    starkbank_entity *out = NULL;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_errors *errors = NULL;
    int status;

    starkbank_boleto_new(&boleto);
    starkbank_entity_set_amount(boleto, STARKBANK_BOLETO_AMOUNT, 400);
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_NAME, "Random Company");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_TAX_ID, "012.345.678-90");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_STREET_LINE_1, "Rua ABC");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_STREET_LINE_2, "Ap 123");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_DISTRICT, "Jardim Paulista");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_CITY, "S\303\243o Paulo");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_STATE_CODE, "SP");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_ZIP_CODE, "01234-567");
    starkbank_list_new(&batch);
    starkbank_list_append(batch, boleto);
    status = starkbank_boleto_create(client, batch, &created, &errors);
    starkbank_list_free(batch);
    if (!requireOk("boleto create", status, errors)) {
        return NULL;
    }
    if (starkbank_list_count(created) == 1) {
        starkbank_entity_clone(starkbank_list_at(created, 0), &out);
    }
    starkbank_list_free(created);
    check("boleto create", out != NULL, "the API answered with no boleto");
    return out;
}

static void boletoPdf(const starkbank_client *client, const char *id)
{
    const char *name = "boleto pdf with layout booklet and hiddenFields customerAddress";
    unsigned char *pdf = NULL;
    size_t length = 0;
    starkbank_errors *errors = NULL;

    if (requireOk(name, starkbank_boleto_pdf(client, id, "booklet", "customerAddress", &pdf, &length,
                                             &errors), errors)) {
        check(name, length > 4 && memcmp(pdf, "%PDF", 4) == 0, "the body is not a PDF");
    }
    starkbank_free(pdf);
}

static void boletoLog(const starkbank_client *client, const char *id)
{
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    int status;

    starkbank_boleto_log_params_new(&params);
    starkbank_entity_append_string(params, "boletoIds", id);
    status = starkbank_boleto_log_query(client, params, 1, &iter);
    expectFirst("boleto log query by boletoIds", status, iter);
    starkbank_entity_free(params);
}

static void payBoleto(const starkbank_client *client, const starkbank_entity *boleto)
{
    starkbank_entity *payment = NULL;
    starkbank_entity *params = NULL;
    starkbank_entity *deleted = NULL;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_errors *errors = NULL;
    starkbank_iter *iter = NULL;
    const char *line = NULL;
    const char *id = NULL;
    int status;

    starkbank_entity_string(boleto, STARKBANK_BOLETO_LINE, &line);
    starkbank_boleto_payment_new(&payment);
    starkbank_entity_set_string(payment, STARKBANK_BOLETO_PAYMENT_LINE, line);
    starkbank_entity_set_string(payment, STARKBANK_BOLETO_PAYMENT_TAX_ID, "20.018.183/0001-80");
    starkbank_entity_set_string(payment, STARKBANK_BOLETO_PAYMENT_DESCRIPTION, "sdk-c sandbox suite");
    starkbank_list_new(&batch);
    starkbank_list_append(batch, payment);
    status = starkbank_boleto_payment_create(client, batch, &created, &errors);
    starkbank_list_free(batch);
    if (!requireOk("boleto payment create", status, errors)) {
        return;
    }
    check("boleto payment create", starkbank_list_count(created) == 1
          && starkbank_entity_string(starkbank_list_at(created, 0), STARKBANK_BOLETO_PAYMENT_ID, &id)
              == STARKBANK_OK, "the payment carries no id");
    if (id != NULL) {
        starkbank_boleto_payment_log_params_new(&params);
        starkbank_entity_append_string(params, "paymentIds", id);
        status = starkbank_boleto_payment_log_query(client, params, 1, &iter);
        expectFirst("boleto payment log query", status, iter);
        starkbank_entity_free(params);
        errors = NULL;
        cleanup("the boleto payment is deleted afterwards",
                starkbank_boleto_payment_delete(client, id, &deleted, &errors), errors);
        starkbank_entity_free(deleted);
    }
    starkbank_list_free(created);
}

static void deleteBoleto(const starkbank_client *client, const char *id)
{
    starkbank_entity *deleted = NULL;
    starkbank_errors *errors = NULL;
    const char *status = NULL;

    if (requireOk("boleto delete", starkbank_boleto_delete(client, id, &deleted, &errors), errors)) {
        check("boleto delete",
              starkbank_entity_string(deleted, STARKBANK_BOLETO_STATUS, &status) == STARKBANK_OK
              && strcmp(status, STARKBANK_BOLETO_STATUS_CANCELED) == 0, status);
    }
    starkbank_entity_free(deleted);
}

/* sdk-python's tests/utils/brcodePayment.py: a fresh Invoice's brcode, paid
   with a resendingLimit rule, then canceled with the invoice. */
static void payBrcode(const starkbank_client *client)
{
    const char *name = "brcode payment create with rule";
    starkbank_entity *invoice = createInvoice(client, name, 0.0, 0.0);
    starkbank_entity *payment = NULL;
    starkbank_entity *rule = NULL;
    starkbank_entity *params = NULL;
    starkbank_entity *patch = NULL;
    starkbank_entity *canceled = NULL;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_errors *errors = NULL;
    starkbank_iter *iter = NULL;
    const char *brcode = NULL;
    const char *id = NULL;
    int status;

    if (invoice == NULL) {
        return;
    }
    starkbank_entity_string(invoice, STARKBANK_INVOICE_BRCODE, &brcode);
    starkbank_brcode_payment_new(&payment);
    starkbank_entity_set_string(payment, STARKBANK_BRCODE_PAYMENT_BRCODE, brcode);
    starkbank_entity_set_string(payment, STARKBANK_BRCODE_PAYMENT_TAX_ID, "20.018.183/0001-80");
    starkbank_entity_set_string(payment, STARKBANK_BRCODE_PAYMENT_DESCRIPTION, "sdk-c sandbox suite");
    starkbank_brcode_payment_rule_new(&rule);
    starkbank_entity_set_string(rule, STARKBANK_BRCODE_PAYMENT_RULE_KEY, "resendingLimit");
    starkbank_entity_set_number(rule, STARKBANK_BRCODE_PAYMENT_RULE_VALUE, 5);
    starkbank_entity_append_entity(payment, STARKBANK_BRCODE_PAYMENT_RULES, rule);
    starkbank_list_new(&batch);
    starkbank_list_append(batch, payment);
    status = starkbank_brcode_payment_create(client, batch, &created, &errors);
    starkbank_list_free(batch);
    if (requireOk(name, status, errors)) {
        check(name, starkbank_list_count(created) == 1
              && starkbank_entity_string(starkbank_list_at(created, 0), STARKBANK_BRCODE_PAYMENT_ID,
                                         &id) == STARKBANK_OK, "the payment carries no id");
    }
    if (id != NULL) {
        starkbank_brcode_payment_log_params_new(&params);
        starkbank_entity_append_string(params, "paymentIds", id);
        status = starkbank_brcode_payment_log_query(client, params, 1, &iter);
        expectFirst("brcode payment log query", status, iter);
        starkbank_entity_free(params);
        starkbank_brcode_payment_params_new(&patch);
        starkbank_entity_set_string(patch, STARKBANK_BRCODE_PAYMENT_STATUS, "canceled");
        errors = NULL;
        cleanup("the brcode payment is canceled afterwards",
                starkbank_brcode_payment_update(client, id, patch, &canceled, &errors), errors);
        starkbank_entity_free(patch);
        starkbank_entity_free(canceled);
    }
    starkbank_list_free(created);
    cancelInvoice(client, invoice);
    starkbank_entity_free(invoice);
}

void sandboxBoleto(const starkbank_client *client, const starkbank_user *organization)
{
    starkbank_entity *boleto;
    const char *id = NULL;

    (void)organization;
    boleto = createBoleto(client);
    if (boleto != NULL && starkbank_entity_string(boleto, "id", &id) == STARKBANK_OK) {
        boletoPdf(client, id);
        boletoLog(client, id);
        payBoleto(client, boleto);
        deleteBoleto(client, id);
    }
    starkbank_entity_free(boleto);
    payBrcode(client);
}
