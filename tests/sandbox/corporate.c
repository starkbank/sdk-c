/*
 * sdk-c#6: CorporateHolder and CorporateCard through create and cancel, and
 * the corporate balance, invoice, withdrawal and transaction reads. Corporate
 * is a product a workspace may not have: when the balance read is refused,
 * every check skips with the reason the API gave.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "harness.h"

static const char *const checks[] = {
    "corporate balance get", "corporate holder create", "corporate card create",
    "corporate invoice query", "corporate withdrawal query", "corporate transaction query",
    "corporate card delete cancels it", "corporate holder delete cancels it"
};

#define CHECK_COUNT ((int)(sizeof(checks) / sizeof(checks[0])))

/* A refusal over who is asking is a FAIL anywhere in the suite; only the
   refusals left over can mean the workspace has no corporate product. */
static const char *const credentialCodes[] = {
    "invalidProject", "invalidOrganization", "invalidWorkspace", "invalidSignature",
    "invalidAccessSignature", "invalidAccessId", "invalidAccessTime", "invalidCredentials"
};

static int credentialError(const starkbank_errors *errors)
{
    const char *code;
    int index;
    int known;

    for (index = 0; index < starkbank_errors_count(errors); index++) {
        code = starkbank_errors_code_at(errors, index);
        for (known = 0; known < (int)(sizeof(credentialCodes) / sizeof(credentialCodes[0])); known++) {
            if (code != NULL && strcmp(code, credentialCodes[known]) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

/* 1 when corporate answers here. A credential error FAILs; any other refusal
   skips every check with the API's first message, and this returns 0. */
static int corporateEnabled(const starkbank_client *client)
{
    starkbank_entity *balance = NULL;
    starkbank_errors *errors = NULL;
    const char *reason;
    int status = starkbank_corporate_balance_get(client, &balance, &errors);
    int index;

    starkbank_entity_free(balance);
    if (status == STARKBANK_OK) {
        starkbank_errors_free(errors);
        check(checks[0], 1, NULL);
        return 1;
    }
    if (starkbank_errors_count(errors) == 0 || credentialError(errors)) {
        return requireOk(checks[0], status, errors);
    }
    reason = starkbank_errors_message_at(errors, 0);
    for (index = 0; index < CHECK_COUNT; index++) {
        skip(checks[index], reason);
    }
    starkbank_errors_free(errors);
    return 0;
}

/* sdk-python's tests/utils/holder.py: a name, a tag and a permission for the
   project the suite runs as. Returns the created holder, or NULL. */
static starkbank_entity *createHolder(const starkbank_client *client)
{
    starkbank_entity *holder = NULL;
    starkbank_entity *permission = NULL;
    starkbank_entity *out = NULL;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_errors *errors = NULL;
    char name[64];
    int status;

    snprintf(name, sizeof(name), "Iron Bank S.A. sdk-c %ld", (long)time(NULL));
    starkbank_corporate_holder_new(&holder);
    starkbank_entity_set_string(holder, STARKBANK_CORPORATE_HOLDER_NAME, name);
    starkbank_entity_append_string(holder, STARKBANK_CORPORATE_HOLDER_TAGS, "Traveler Employee");
    starkbank_permission_new(&permission);
    starkbank_entity_set_string(permission, STARKBANK_PERMISSION_OWNER_ID, getenv("SANDBOX_ID"));
    starkbank_entity_set_string(permission, STARKBANK_PERMISSION_OWNER_TYPE, "project");
    starkbank_entity_append_entity(holder, STARKBANK_CORPORATE_HOLDER_PERMISSIONS, permission);
    starkbank_list_new(&batch);
    starkbank_list_append(batch, holder);
    status = starkbank_corporate_holder_create(client, batch, &created, &errors);
    starkbank_list_free(batch);
    if (!requireOk(checks[1], status, errors)) {
        return NULL;
    }
    if (starkbank_list_count(created) == 1) {
        starkbank_entity_clone(starkbank_list_at(created, 0), &out);
    }
    starkbank_list_free(created);
    check(checks[1], out != NULL, "the API answered with no holder");
    return out;
}

static starkbank_entity *createCard(const starkbank_client *client, const char *holderId)
{
    starkbank_entity *card = NULL;
    starkbank_entity *created = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;
    int status;

    starkbank_corporate_card_new(&card);
    starkbank_entity_set_string(card, STARKBANK_CORPORATE_CARD_HOLDER_ID, holderId);
    status = starkbank_corporate_card_create(client, card, &created, &errors);
    starkbank_entity_free(card);
    if (!requireOk(checks[2], status, errors)) {
        return NULL;
    }
    check(checks[2], starkbank_entity_string(created, STARKBANK_CORPORATE_CARD_ID, &id) == STARKBANK_OK,
          "the card carries no id");
    return created;
}

/* A delete that the API answers with the entity, now canceled. */
typedef int (STARKBANK_CALL *DeleteFn)(const starkbank_client *client, const char *id,
                                       starkbank_entity **out, starkbank_errors **errors);

static void deleteCancels(const starkbank_client *client, const char *name, DeleteFn remove,
                          const starkbank_entity *entity)
{
    starkbank_entity *deleted = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;
    const char *status = NULL;

    if (entity == NULL || starkbank_entity_string(entity, "id", &id) != STARKBANK_OK) {
        return;
    }
    if (requireOk(name, remove(client, id, &deleted, &errors), errors)) {
        check(name, starkbank_entity_string(deleted, "status", &status) == STARKBANK_OK
              && strcmp(status, "canceled") == 0, status);
    }
    starkbank_entity_free(deleted);
}

static void reads(const starkbank_client *client)
{
    starkbank_iter *iter = NULL;
    int status;

    status = starkbank_corporate_invoice_query(client, NULL, 1, &iter);
    firstOrSkip(checks[3], status, iter, "no corporate invoice in this workspace");
    iter = NULL;
    status = starkbank_corporate_withdrawal_query(client, NULL, 1, &iter);
    firstOrSkip(checks[4], status, iter, "no corporate withdrawal in this workspace");
    iter = NULL;
    status = starkbank_corporate_transaction_query(client, NULL, 1, &iter);
    firstOrSkip(checks[5], status, iter, "no corporate transaction in this workspace");
}

void sandboxCorporate(const starkbank_client *client, const starkbank_user *organization)
{
    starkbank_entity *holder;
    starkbank_entity *card = NULL;
    const char *holderId = NULL;

    (void)organization;
    if (!corporateEnabled(client)) {
        return;
    }
    holder = createHolder(client);
    if (holder != NULL
            && starkbank_entity_string(holder, STARKBANK_CORPORATE_HOLDER_ID, &holderId) == STARKBANK_OK) {
        card = createCard(client, holderId);
    }
    reads(client);
    deleteCancels(client, checks[6], starkbank_corporate_card_delete, card);
    deleteCancels(client, checks[7], starkbank_corporate_holder_delete, holder);
    starkbank_entity_free(card);
    starkbank_entity_free(holder);
}
