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
    {"signature", "ecdsa-c#1 core-c#1 sdk-c#6", sandboxSignature}
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
