/*
 * sdk-c#4: UtilityPayment, TaxPayment and DarfPayment, a utility payment from
 * a line and from a bar code alone, and each log walked two pages through
 * *_page and its cursor. Every payment is deleted once its check ran.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "harness.h"

typedef int (STARKBANK_CALL *CreateFn)(const starkbank_client *client, const starkbank_list *items,
                                       starkbank_list **out, starkbank_errors **errors);
typedef int (STARKBANK_CALL *DeleteFn)(const starkbank_client *client, const char *id,
                                       starkbank_entity **out, starkbank_errors **errors);
typedef int (STARKBANK_CALL *PageFn)(const starkbank_client *client, const starkbank_entity *params,
                                     starkbank_list **out, char **out_cursor,
                                     starkbank_errors **errors);
typedef int (STARKBANK_CALL *ParamsFn)(starkbank_entity **out);

/* sdk-python's tests/utils/nonBoletoPayment.py: one example bar code with the
   business segment at 1, an 11-digit amount at 4 and the business code at 15.
   The amount varies with the clock so a rerun is not a duplicate payment. */
static void barCode(char out[45], char segment, const char *business, int offset)
{
    char amount[12];

    snprintf(amount, sizeof(amount), "%011ld", 100L + (long)(time(NULL) % 90000) + offset);
    memcpy(out, "83660000001084301380074119002551100010601813", 45);
    out[1] = segment;
    memcpy(out + 4, amount, 11);
    memcpy(out + 15, business, 4);
}

/* Creates the one payment it is handed, checks it has an id and deletes it. */
static void createAndDelete(const starkbank_client *client, const char *name, CreateFn create,
                            DeleteFn remove, starkbank_entity *payment)
{
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_entity *deleted = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;
    int status;

    starkbank_list_new(&batch);
    starkbank_list_append(batch, payment);
    status = create(client, batch, &created, &errors);
    starkbank_list_free(batch);
    if (!requireOk(name, status, errors)) {
        return;
    }
    check(name, starkbank_list_count(created) == 1
          && starkbank_entity_string(starkbank_list_at(created, 0), "id", &id) == STARKBANK_OK,
          "the payment carries no id");
    if (id != NULL) {
        errors = NULL;
        requireOk("the payment is deleted afterwards", remove(client, id, &deleted, &errors), errors);
        starkbank_entity_free(deleted);
    }
    starkbank_list_free(created);
}

static void walkTwoPages(const starkbank_client *client, const char *name, ParamsFn newParams,
                         PageFn page)
{
    starkbank_entity *params = NULL;
    starkbank_list *first = NULL;
    starkbank_list *second = NULL;
    starkbank_errors *errors = NULL;
    char *cursor = NULL;
    int status;

    newParams(&params);
    starkbank_entity_set_number(params, "limit", 1);
    status = page(client, params, &first, &cursor, &errors);
    if (requireOk(name, status, errors) && cursor == NULL) {
        skip(name, "one page of logs only; nothing to walk");
    }
    if (cursor != NULL) {
        starkbank_entity_set_string(params, "cursor", cursor);
        errors = NULL;
        if (requireOk(name, page(client, params, &second, NULL, &errors), errors)) {
            check(name, starkbank_list_count(first) == 1 && starkbank_list_count(second) == 1,
                  "a page did not hold the one log asked for");
        }
    }
    starkbank_free(cursor);
    starkbank_list_free(second);
    starkbank_list_free(first);
    starkbank_entity_free(params);
}

static void utilityPayments(const starkbank_client *client)
{
    starkbank_entity *payment = NULL;
    char code[45];

    barCode(code, '2', "0846", 0);
    starkbank_utility_payment_new(&payment);
    starkbank_entity_set_string(payment, STARKBANK_UTILITY_PAYMENT_BAR_CODE, code);
    starkbank_entity_set_string(payment, STARKBANK_UTILITY_PAYMENT_DESCRIPTION, "sdk-c sandbox suite");
    starkbank_entity_append_string(payment, STARKBANK_UTILITY_PAYMENT_TAGS, "test1");
    createAndDelete(client, "utility payment create", starkbank_utility_payment_create,
                    starkbank_utility_payment_delete, payment);

    barCode(code, '2', "0846", 1);
    starkbank_utility_payment_new(&payment);
    starkbank_entity_set_string(payment, STARKBANK_UTILITY_PAYMENT_BAR_CODE, code);
    starkbank_entity_set_string(payment, STARKBANK_UTILITY_PAYMENT_DESCRIPTION, "sdk-c sandbox suite");
    createAndDelete(client, "utility payment create with barCode only",
                    starkbank_utility_payment_create, starkbank_utility_payment_delete, payment);

    /* sdk-python's TestUtilityPaymentDelete line, spaces included. */
    starkbank_utility_payment_new(&payment);
    starkbank_entity_set_string(payment, STARKBANK_UTILITY_PAYMENT_LINE,
                                "83640000001 1 08740138007 0 61053026111 0 08067159411 9");
    starkbank_entity_set_string(payment, STARKBANK_UTILITY_PAYMENT_DESCRIPTION, "sdk-c sandbox suite");
    createAndDelete(client, "utility payment create with line only",
                    starkbank_utility_payment_create, starkbank_utility_payment_delete, payment);
}

static void taxPayment(const starkbank_client *client)
{
    starkbank_entity *payment = NULL;
    char code[45];

    barCode(code, '1', "5701", 2);
    starkbank_tax_payment_new(&payment);
    starkbank_entity_set_string(payment, STARKBANK_TAX_PAYMENT_BAR_CODE, code);
    starkbank_entity_set_string(payment, STARKBANK_TAX_PAYMENT_DESCRIPTION, "sdk-c sandbox suite");
    createAndDelete(client, "tax payment create", starkbank_tax_payment_create,
                    starkbank_tax_payment_delete, payment);
}

/* sdk-python's tests/utils/taxPayment.py example_darf_payment: a competence
   in the past and a due date in the future, both relative to today. */
static void darfPayment(const starkbank_client *client)
{
    starkbank_entity *payment = NULL;
    time_t now = time(NULL);
    time_t past = now - 40 * 24 * 3600;
    time_t future = now + 30 * 24 * 3600;
    struct tm day;

    starkbank_darf_payment_new(&payment);
    starkbank_entity_set_string(payment, STARKBANK_DARF_PAYMENT_DESCRIPTION, "sdk-c sandbox suite");
    starkbank_entity_set_string(payment, STARKBANK_DARF_PAYMENT_REVENUE_CODE, "1240");
    starkbank_entity_set_string(payment, STARKBANK_DARF_PAYMENT_TAX_ID, "012.345.678-90");
    starkbank_entity_set_string(payment, STARKBANK_DARF_PAYMENT_REFERENCE_NUMBER, "2340978970");
    starkbank_entity_set_amount(payment, STARKBANK_DARF_PAYMENT_NOMINAL_AMOUNT, 1234);
    starkbank_entity_set_amount(payment, STARKBANK_DARF_PAYMENT_FINE_AMOUNT, 12);
    starkbank_entity_set_amount(payment, STARKBANK_DARF_PAYMENT_INTEREST_AMOUNT, 34);
    day = *localtime(&past);
    starkbank_entity_set_date(payment, STARKBANK_DARF_PAYMENT_COMPETENCE, day.tm_year + 1900,
                              day.tm_mon + 1, day.tm_mday);
    day = *localtime(&future);
    starkbank_entity_set_date(payment, STARKBANK_DARF_PAYMENT_DUE, day.tm_year + 1900,
                              day.tm_mon + 1, day.tm_mday);
    createAndDelete(client, "darf payment create", starkbank_darf_payment_create,
                    starkbank_darf_payment_delete, payment);
}

void sandboxNonBoleto(const starkbank_client *client, const starkbank_user *organization)
{
    (void)organization;
    utilityPayments(client);
    taxPayment(client);
    darfPayment(client);
    walkTwoPages(client, "utility payment log query paged", starkbank_utility_payment_log_params_new,
                 starkbank_utility_payment_log_page);
    walkTwoPages(client, "tax payment log query paged", starkbank_tax_payment_log_params_new,
                 starkbank_tax_payment_log_page);
    walkTwoPages(client, "darf payment log query paged", starkbank_darf_payment_log_params_new,
                 starkbank_darf_payment_log_page);
}
