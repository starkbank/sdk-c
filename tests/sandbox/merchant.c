/*
 * sdk-c#10: MerchantSession and its log, the merchant card, installment and
 * purchase reads, and the three enum lists with their search filter. There is
 * no merchant_session_purchase: #15 removed it, because the purchase carries
 * raw card data and belongs in the payer's front-end.
 */

#include "harness.h"

typedef int (STARKBANK_CALL *ParamsFn)(starkbank_entity **out);
typedef int (STARKBANK_CALL *QueryFn)(const starkbank_client *client, const starkbank_entity *params,
                                      int limit, starkbank_iter **out);

/* sdk-python's tests/utils/merchantSession.py: debit and credit, two
   installment options, an hour to live. Returns the created session, or NULL. */
static starkbank_entity *createSession(const starkbank_client *client)
{
    const char *name = "merchant session create";
    starkbank_entity *session = NULL;
    starkbank_entity *installment = NULL;
    starkbank_entity *created = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;
    int status;

    starkbank_merchant_session_new(&session);
    starkbank_entity_append_string(session, STARKBANK_MERCHANT_SESSION_ALLOWED_FUNDING_TYPES,
                                   STARKBANK_MERCHANT_SESSION_FUNDING_TYPE_DEBIT);
    starkbank_entity_append_string(session, STARKBANK_MERCHANT_SESSION_ALLOWED_FUNDING_TYPES,
                                   STARKBANK_MERCHANT_SESSION_FUNDING_TYPE_CREDIT);
    starkbank_allowed_installment_new(&installment);
    starkbank_entity_set_amount(installment, STARKBANK_ALLOWED_INSTALLMENT_TOTAL_AMOUNT, 5000);
    starkbank_entity_set_number(installment, STARKBANK_ALLOWED_INSTALLMENT_COUNT, 1);
    starkbank_entity_append_entity(session, STARKBANK_MERCHANT_SESSION_ALLOWED_INSTALLMENTS, installment);
    starkbank_allowed_installment_new(&installment);
    starkbank_entity_set_amount(installment, STARKBANK_ALLOWED_INSTALLMENT_TOTAL_AMOUNT, 5500);
    starkbank_entity_set_number(installment, STARKBANK_ALLOWED_INSTALLMENT_COUNT, 2);
    starkbank_entity_append_entity(session, STARKBANK_MERCHANT_SESSION_ALLOWED_INSTALLMENTS, installment);
    starkbank_entity_set_number(session, STARKBANK_MERCHANT_SESSION_EXPIRATION, 3600);
    starkbank_entity_set_string(session, STARKBANK_MERCHANT_SESSION_CHALLENGE_MODE,
                                STARKBANK_MERCHANT_SESSION_CHALLENGE_MODE_DISABLED);
    status = starkbank_merchant_session_create(client, session, &created, &errors);
    starkbank_entity_free(session);
    if (!requireOk(name, status, errors)) {
        return NULL;
    }
    check(name, starkbank_entity_string(created, STARKBANK_MERCHANT_SESSION_ID, &id) == STARKBANK_OK,
          "the session carries no id");
    return created;
}

static void sessionLog(const starkbank_client *client, const starkbank_entity *session)
{
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    const char *id = NULL;
    int status;

    if (session == NULL || starkbank_entity_string(session, STARKBANK_MERCHANT_SESSION_ID, &id)
            != STARKBANK_OK) {
        return;
    }
    starkbank_merchant_session_log_params_new(&params);
    starkbank_entity_append_string(params, "sessionIds", id);
    status = starkbank_merchant_session_log_query(client, params, 1, &iter);
    expectFirst("merchant session log query", status, iter);
    starkbank_entity_free(params);
}

/* An enum list filtered by sdk-python's own search term: never empty. No
   limit, as sdk-python sends none: these lookups refuse one with
   invalidQueryString. */
static void search(const starkbank_client *client, const char *name, ParamsFn newParams,
                   QueryFn query, const char *term)
{
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    int status;

    newParams(&params);
    status = starkbank_entity_set_string(params, "search", term);
    if (status == STARKBANK_OK) {
        status = query(client, params, 0, &iter);
    }
    expectFirst(name, status, iter);
    starkbank_entity_free(params);
}

static void reads(const starkbank_client *client)
{
    starkbank_iter *iter = NULL;
    int status;

    status = starkbank_merchant_card_query(client, NULL, 1, &iter);
    firstOrSkip("merchant card query", status, iter, "no merchant card in this workspace");
    iter = NULL;
    status = starkbank_merchant_installment_query(client, NULL, 1, &iter);
    firstOrSkip("merchant installment query", status, iter, "no merchant installment in this workspace");
    iter = NULL;
    status = starkbank_merchant_purchase_query(client, NULL, 1, &iter);
    firstOrSkip("merchant purchase query", status, iter, "no merchant purchase in this workspace");
}

void sandboxMerchant(const starkbank_client *client, const starkbank_user *organization)
{
    starkbank_entity *session;

    (void)organization;
    session = createSession(client);
    sessionLog(client, session);
    starkbank_entity_free(session);
    reads(client);
    search(client, "card method query with search", starkbank_card_method_params_new,
           starkbank_card_method_query, "token");
    search(client, "merchant category query with search", starkbank_merchant_category_params_new,
           starkbank_merchant_category_query, "food");
    search(client, "merchant country query with search", starkbank_merchant_country_params_new,
           starkbank_merchant_country_query, "brazil");
}
