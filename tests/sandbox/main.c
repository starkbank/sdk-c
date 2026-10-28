/*
 * Reads the credentials sdk-python's own suite reads, connects one client to
 * the sandbox, and runs the groups SANDBOX_ONLY names, in table order.
 *
 * Exit 0 when nothing failed (skips allowed), 1 when a check failed, 2 when a
 * credential is missing or invalid or SANDBOX_ONLY names no group. Exit 2
 * sends no request, and no path prints a credential.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"

typedef struct {
    const char *name;
    const char *card;
    SandboxGroup run;
} GroupRow;

static const GroupRow groups[] = {
    {"auth", "ecdsa-c#1", sandboxAuth},
    {"signature", "ecdsa-c#1 core-c#1 sdk-c#6", sandboxSignature},
    {"core", "core-c#1", sandboxCore},
    {"webhookPreview", "sdk-c#1", sandboxWebhookPreview},
    {"boleto", "sdk-c#3", sandboxBoleto}
};

#define GROUP_COUNT ((int)(sizeof(groups) / sizeof(groups[0])))

static int passed;
static int failed;
static int skipped;

void startGroup(const char *name, const char *card)
{
    printf("\n%s (%s)\n", name, card);
}

void check(const char *name, int ok, const char *detail)
{
    if (ok) {
        passed++;
        printf("  ok   %s\n", name);
        return;
    }
    failed++;
    printf("  FAIL %s%s%s\n", name, detail != NULL ? ": " : "", detail != NULL ? detail : "");
}

void skip(const char *name, const char *reason)
{
    skipped++;
    printf("  skip %s: %s\n", name, reason);
}

int requireOk(const char *name, int status, starkbank_errors *errors)
{
    int index;

    if (status == STARKBANK_OK) {
        starkbank_errors_free(errors);
        return 1;
    }
    failed++;
    printf("  FAIL %s: %s\n", name, starkbank_strerror(status));
    for (index = 0; index < starkbank_errors_count(errors); index++) {
        printf("         %s: %s\n", starkbank_errors_code_at(errors, index),
               starkbank_errors_message_at(errors, index));
    }
    starkbank_errors_free(errors);
    return 0;
}

starkbank_entity *createInvoice(const starkbank_client *client, const char *name, double fine,
                                double interest)
{
    starkbank_entity *invoice = NULL;
    starkbank_entity *out = NULL;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_errors *errors = NULL;
    int status;

    starkbank_invoice_new(&invoice);
    starkbank_entity_set_amount(invoice, STARKBANK_INVOICE_AMOUNT, 400);
    starkbank_entity_set_string(invoice, STARKBANK_INVOICE_TAX_ID, "012.345.678-90");
    starkbank_entity_set_string(invoice, STARKBANK_INVOICE_NAME, "Iron Bank S.A.");
    if (fine != 0.0) {
        starkbank_entity_set_number(invoice, STARKBANK_INVOICE_FINE, fine);
    }
    if (interest != 0.0) {
        starkbank_entity_set_number(invoice, STARKBANK_INVOICE_INTEREST, interest);
    }
    starkbank_list_new(&batch);
    starkbank_list_append(batch, invoice);
    status = starkbank_invoice_create(client, batch, &created, &errors);
    starkbank_list_free(batch);
    if (!requireOk(name, status, errors)) {
        return NULL;
    }
    if (starkbank_list_count(created) == 1) {
        starkbank_entity_clone(starkbank_list_at(created, 0), &out);
    }
    starkbank_list_free(created);
    if (out == NULL) {
        check(name, 0, "the API answered with no invoice");
    }
    return out;
}

void cancelInvoice(const starkbank_client *client, const starkbank_entity *invoice)
{
    starkbank_entity *patch = NULL;
    starkbank_entity *canceled = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;

    if (invoice == NULL
            || starkbank_entity_string(invoice, STARKBANK_INVOICE_ID, &id) != STARKBANK_OK) {
        return;
    }
    starkbank_invoice_params_new(&patch);
    starkbank_entity_set_string(patch, STARKBANK_INVOICE_STATUS, STARKBANK_INVOICE_STATUS_CANCELED);
    requireOk("the invoice is canceled afterwards",
              starkbank_invoice_update(client, id, patch, &canceled, &errors), errors);
    starkbank_entity_free(patch);
    starkbank_entity_free(canceled);
}

void expectFirst(const char *name, int status, starkbank_iter *iter)
{
    const starkbank_entity *item = NULL;
    starkbank_errors *errors = NULL;

    if (status == STARKBANK_OK) {
        status = starkbank_iter_next(iter, &item, &errors);
    }
    if (requireOk(name, status, errors)) {
        check(name, item != NULL, "the query came back empty");
    }
    starkbank_iter_free(iter);
}

static const char *readEnv(const char *name)
{
    const char *value = getenv(name);

    if (value == NULL || value[0] == '\0') {
        return NULL;
    }
    return value;
}

static int setupError(const char *message, const char *name)
{
    fprintf(stderr, "sandbox: %s%s\n", name != NULL ? name : "", message);
    return 2;
}

static int findGroup(const char *name, size_t length)
{
    int index;

    for (index = 0; index < GROUP_COUNT; index++) {
        if (strlen(groups[index].name) == length && strncmp(groups[index].name, name, length) == 0) {
            return index;
        }
    }
    return -1;
}

/* Marks selected[i] for every group SANDBOX_ONLY names; all of them when it is
   unset. Returns 0, or prints the unknown name and returns 2. */
static int selectGroups(const char *only, int *selected)
{
    const char *cursor = only;
    const char *comma;
    size_t length;
    int index;

    for (index = 0; index < GROUP_COUNT; index++) {
        selected[index] = only == NULL;
    }
    while (cursor != NULL && *cursor != '\0') {
        comma = strchr(cursor, ',');
        length = comma != NULL ? (size_t)(comma - cursor) : strlen(cursor);
        index = findGroup(cursor, length);
        if (index < 0) {
            fprintf(stderr, "sandbox: unknown group %.*s\n", (int)length, cursor);
            return 2;
        }
        selected[index] = 1;
        cursor = comma != NULL ? comma + 1 : NULL;
    }
    return 0;
}

/* The organization is optional, but half of it is a mistake worth naming. */
static int readOrganization(starkbank_user **out)
{
    const char *id = readEnv("SANDBOX_ORGANIZATION_ID");
    const char *pem = readEnv("SANDBOX_ORGANIZATION_PRIVATE_KEY");

    *out = NULL;
    if (id == NULL && pem == NULL) {
        return 0;
    }
    if (id == NULL) {
        return setupError(" is not set", "SANDBOX_ORGANIZATION_ID");
    }
    if (pem == NULL) {
        return setupError(" is not set", "SANDBOX_ORGANIZATION_PRIVATE_KEY");
    }
    if (starkbank_organization_new(id, STARKBANK_ENVIRONMENT_SANDBOX, pem, NULL, out)
            != STARKBANK_OK) {
        return setupError(" is not a valid PEM", "SANDBOX_ORGANIZATION_PRIVATE_KEY");
    }
    return 0;
}

static int connectProject(starkbank_client **out)
{
    const char *id = readEnv("SANDBOX_ID");
    const char *pem = readEnv("SANDBOX_PRIVATE_KEY");
    starkbank_user *project = NULL;

    *out = NULL;
    if (id == NULL) {
        return setupError(" is not set", "SANDBOX_ID");
    }
    if (pem == NULL) {
        return setupError(" is not set", "SANDBOX_PRIVATE_KEY");
    }
    if (starkbank_project_new(id, STARKBANK_ENVIRONMENT_SANDBOX, pem, &project) != STARKBANK_OK) {
        return setupError(" is not a valid PEM", "SANDBOX_PRIVATE_KEY");
    }
    if (starkbank_client_new(project, out) != STARKBANK_OK) {
        return setupError("could not build a client", NULL);
    }
    if (starkbank_client_set_curl_transport(*out) != STARKBANK_OK) {
        starkbank_client_free(*out);
        *out = NULL;
        return setupError("this build has no libcurl transport", NULL);
    }
    return 0;
}

int main(void)
{
    starkbank_client *client = NULL;
    starkbank_user *organization = NULL;
    int selected[GROUP_COUNT];
    int status;
    int index;

    status = connectProject(&client);
    if (status != 0) {
        return status;
    }
    status = readOrganization(&organization);
    if (status == 0) {
        status = selectGroups(readEnv("SANDBOX_ONLY"), selected);
    }
    if (status != 0) {
        starkbank_user_free(organization);
        starkbank_client_free(client);
        return status;
    }
    for (index = 0; index < GROUP_COUNT; index++) {
        if (!selected[index]) {
            continue;
        }
        startGroup(groups[index].name, groups[index].card);
        groups[index].run(client, organization);
    }
    starkbank_user_free(organization);
    starkbank_client_free(client);
    printf("\n%d passed, %d failed, %d skipped\n", passed, failed, skipped);
    return failed == 0 ? 0 : 1;
}
