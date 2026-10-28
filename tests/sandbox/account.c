/*
 * sdk-c#5: Deposit and its log, Deposit's amount patch, a DICT key read,
 * Institution's page, Transaction's query, and Workspace's update of name,
 * username and picture. The workspace checks need the organization
 * credential and skip without it.
 */

#include <stdlib.h>
#include <string.h>

#include "harness.h"

/* A 1x1 PNG: sdk-python uploads tests/utils/logo.png, 158 KB that would add
   nothing here the API treats differently from 69 bytes. */
#define PICTURE_DATA_URI \
    "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAIAAACQd1PeAAAADElEQVR4nGP4//8/AAX+Av4N70a4AAAAAElFTkSuQmCC"

static void depositAndLog(const starkbank_client *client)
{
    const char *name = "deposit query and log query";
    starkbank_iter *iter = NULL;
    starkbank_iter *logs = NULL;
    starkbank_entity *params = NULL;
    const starkbank_entity *deposit = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;
    int status;

    status = starkbank_deposit_query(client, NULL, 1, &iter);
    if (status == STARKBANK_OK) {
        status = starkbank_iter_next(iter, &deposit, &errors);
    }
    if (requireOk(name, status, errors) && deposit == NULL) {
        skip(name, "no deposit in this workspace");
    }
    if (deposit != NULL && starkbank_entity_string(deposit, STARKBANK_DEPOSIT_ID, &id) == STARKBANK_OK) {
        starkbank_deposit_log_params_new(&params);
        starkbank_entity_append_string(params, "depositIds", id);
        status = starkbank_deposit_log_query(client, params, 1, &logs);
        expectFirst(name, status, logs);
        starkbank_entity_free(params);
    }
    starkbank_iter_free(iter);
}

/* sdk-python's TestDepositInfoPatch: the first created deposit that is not a
   TED, reversed in full by patching its amount to 0. */
static void patchDeposit(const starkbank_client *client, const char *id)
{
    const char *name = "deposit update amount";
    starkbank_entity *patch = NULL;
    starkbank_entity *updated = NULL;
    starkbank_errors *errors = NULL;
    double amount = -1.0;

    starkbank_deposit_params_new(&patch);
    starkbank_entity_set_amount(patch, STARKBANK_DEPOSIT_AMOUNT, 0);
    if (requireOk(name, starkbank_deposit_update(client, id, patch, &updated, &errors), errors)) {
        check(name, starkbank_entity_amount(updated, STARKBANK_DEPOSIT_AMOUNT, &amount) == STARKBANK_OK
              && amount == 0.0, "the deposit did not come back with amount 0");
    }
    starkbank_entity_free(updated);
    starkbank_entity_free(patch);
}

static void depositUpdate(const starkbank_client *client)
{
    const char *name = "deposit update amount";
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *deposit = NULL;
    starkbank_errors *errors = NULL;
    const char *type = NULL;
    const char *id = NULL;
    int status;

    starkbank_deposit_params_new(&params);
    starkbank_entity_set_string(params, STARKBANK_DEPOSIT_STATUS, "created");
    status = starkbank_deposit_query(client, params, 20, &iter);
    while (status == STARKBANK_OK && id == NULL) {
        status = starkbank_iter_next(iter, &deposit, &errors);
        if (status != STARKBANK_OK || deposit == NULL) {
            break;
        }
        if (starkbank_entity_string(deposit, STARKBANK_DEPOSIT_TYPE, &type) == STARKBANK_OK
                && strcmp(type, "ted") == 0) {
            continue;
        }
        starkbank_entity_string(deposit, STARKBANK_DEPOSIT_ID, &id);
    }
    if (requireOk(name, status, errors) && id == NULL) {
        skip(name, "no created, non-TED deposit to patch");
    }
    if (id != NULL) {
        patchDeposit(client, id);
    }
    starkbank_iter_free(iter);
    starkbank_entity_free(params);
}

static void dictKey(const starkbank_client *client)
{
    const char *name = "dict key get";
    starkbank_entity *key = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;

    if (requireOk(name, starkbank_dict_key_get(client, "valid@sandbox.com", &key, &errors), errors)) {
        check(name, starkbank_entity_string(key, STARKBANK_DICT_KEY_ID, &id) == STARKBANK_OK,
              "the key carries no id");
    }
    starkbank_entity_free(key);
}

static void institutionPage(const starkbank_client *client)
{
    const char *name = "institution page";
    starkbank_list *institutions = NULL;
    starkbank_errors *errors = NULL;

    if (requireOk(name, starkbank_institution_page(client, NULL, &institutions, NULL, &errors),
                  errors)) {
        check(name, starkbank_list_count(institutions) > 0, "the page came back empty");
    }
    starkbank_list_free(institutions);
}

/* A client on the organization, scoped to workspaceId. NULL after printing
   FAIL under name. */
static starkbank_client *organizationClient(const starkbank_user *organization,
                                            const char *workspaceId, const char *name)
{
    starkbank_user *scoped = NULL;
    starkbank_client *client = NULL;

    if (starkbank_organization_replace(organization, workspaceId, &scoped) != STARKBANK_OK
            || starkbank_client_new(scoped, &client) != STARKBANK_OK
            || starkbank_client_set_curl_transport(client) != STARKBANK_OK) {
        starkbank_client_free(client);
        check(name, 0, "could not build a client on the organization");
        return NULL;
    }
    return client;
}

static void patchWorkspace(const starkbank_client *client, const char *name, const char *id,
                           const char *field, const char *value, const char *field2,
                           const char *value2)
{
    starkbank_entity *patch = NULL;
    starkbank_entity *updated = NULL;
    starkbank_errors *errors = NULL;
    const char *updatedId = NULL;

    starkbank_workspace_params_new(&patch);
    starkbank_entity_set_string(patch, field, value);
    if (field2 != NULL) {
        starkbank_entity_set_string(patch, field2, value2);
    }
    if (requireOk(name, starkbank_workspace_update(client, id, patch, &updated, &errors), errors)) {
        check(name, starkbank_entity_string(updated, STARKBANK_WORKSPACE_ID, &updatedId) == STARKBANK_OK
              && strcmp(updatedId, id) == 0, "a different workspace came back");
    }
    starkbank_entity_free(updated);
    starkbank_entity_free(patch);
}

/* sdk-python's TestWorkspaceUpdate sends the workspace's own name and username
   back, so the workspace keeps its identity. The picture it replaces is not
   restored, which is why the workspace is named explicitly. */
static void updateWorkspace(const starkbank_client *client, const char *id)
{
    const char *name = "workspace update name and username";
    starkbank_entity *workspace = NULL;
    starkbank_errors *errors = NULL;
    const char *username = NULL;
    const char *workspaceName = NULL;

    if (!requireOk(name, starkbank_workspace_get(client, id, &workspace, &errors), errors)) {
        return;
    }
    starkbank_entity_string(workspace, STARKBANK_WORKSPACE_USERNAME, &username);
    starkbank_entity_string(workspace, STARKBANK_WORKSPACE_NAME, &workspaceName);
    patchWorkspace(client, name, id, STARKBANK_WORKSPACE_NAME, workspaceName,
                   STARKBANK_WORKSPACE_USERNAME, username);
    patchWorkspace(client, "workspace update picture as data URI", id, STARKBANK_WORKSPACE_PICTURE,
                   PICTURE_DATA_URI, NULL, NULL);
    starkbank_entity_free(workspace);
}

/* Only the workspace SANDBOX_WORKSPACE_ID names: the first one an organization
   lists may be a workspace QA shares, and its picture would be overwritten. */
static void workspaces(const starkbank_user *organization)
{
    const char *name = "workspace update name and username";
    const char *picture = "workspace update picture as data URI";
    const char *id = getenv("SANDBOX_WORKSPACE_ID");
    starkbank_client *scoped;

    if (organization == NULL) {
        skip(name, "SANDBOX_ORGANIZATION_ID is not set");
        skip(picture, "SANDBOX_ORGANIZATION_ID is not set");
        return;
    }
    if (id == NULL || id[0] == '\0') {
        skip(name, "SANDBOX_WORKSPACE_ID is not set");
        skip(picture, "SANDBOX_WORKSPACE_ID is not set");
        return;
    }
    scoped = organizationClient(organization, id, name);
    if (scoped == NULL) {
        return;
    }
    updateWorkspace(scoped, id);
    starkbank_client_free(scoped);
}

static void transactionQuery(const starkbank_client *client)
{
    starkbank_iter *iter = NULL;
    int status = starkbank_transaction_query(client, NULL, 1, &iter);

    firstOrSkip("transaction query", status, iter, "no transaction in this workspace");
}

void sandboxAccount(const starkbank_client *client, const starkbank_user *organization)
{
    depositAndLog(client);
    depositUpdate(client);
    dictKey(client);
    institutionPage(client);
    transactionQuery(client);
    workspaces(organization);
}
