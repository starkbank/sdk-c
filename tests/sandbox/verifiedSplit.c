/*
 * sdk-c#14: VerifiedAccount through create, log and delete, a
 * VerifiedTransfer to it, SplitReceiver create and query, the workspace's
 * SplitProfile through put, and Split with its log.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "harness.h"

/* sdk-python's tests/utils/verifiedTransfer.py against the account. */
static void verifiedTransfer(const starkbank_client *client, const char *accountId)
{
    starkbank_entity *transfer = NULL;
    starkbank_entity *created;
    char externalId[48];

    snprintf(externalId, sizeof(externalId), "sdk-c-%ld-%d", (long)time(NULL), rand());
    starkbank_verified_transfer_new(&transfer);
    starkbank_entity_set_amount(transfer, "amount", 1000);
    starkbank_entity_set_string(transfer, STARKBANK_VERIFIED_TRANSFER_ACCOUNT_ID, accountId);
    starkbank_entity_set_string(transfer, STARKBANK_VERIFIED_TRANSFER_EXTERNAL_ID, externalId);
    starkbank_entity_set_string(transfer, "description", "Test description");
    created = createOne(client, "verified transfer create", starkbank_verified_transfer_create, transfer);
    starkbank_entity_free(created);
}

/* sdk-python's generateExamplePixKeyVerifiedAccountJson: a tax id and the
   DICT key that resolves to it in the sandbox. */
static void verifiedAccount(const starkbank_client *client)
{
    const char *name = "verified account create and delete";
    starkbank_entity *account = NULL;
    starkbank_entity *created;
    const char *id = NULL;

    starkbank_verified_account_new(&account);
    starkbank_entity_set_string(account, STARKBANK_VERIFIED_ACCOUNT_TAX_ID, "039.946.040-36");
    starkbank_entity_set_string(account, STARKBANK_VERIFIED_ACCOUNT_KEY_ID, "arya.stark@starkbank.com");
    starkbank_entity_append_string(account, "tags", "verified-account-test");
    created = createOne(client, name, starkbank_verified_account_create, account);
    if (created == NULL || starkbank_entity_string(created, STARKBANK_VERIFIED_ACCOUNT_ID, &id)
            != STARKBANK_OK) {
        starkbank_entity_free(created);
        return;
    }
    verifiedTransfer(client, id);
    expectLog(client, "verified account log query", starkbank_verified_account_log_params_new,
              starkbank_verified_account_log_query, "accountIds", id);
    deleteOne(client, name, starkbank_verified_account_delete, id);
    starkbank_entity_free(created);
}

/* sdk-python's tests/utils/splitReceiver.py example. */
static void splitReceiver(const starkbank_client *client)
{
    const char *name = "split receiver create and query";
    starkbank_entity *receiver = NULL;
    starkbank_entity *created;
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    const char *id = NULL;
    int status;

    starkbank_split_receiver_new(&receiver);
    starkbank_entity_set_string(receiver, "name", "Jo\303\243o");
    starkbank_entity_set_string(receiver, "taxId", "012.345.678-90");
    starkbank_entity_set_string(receiver, "bankCode", "18236120");
    starkbank_entity_set_string(receiver, "branchCode", "0001");
    starkbank_entity_set_string(receiver, "accountNumber", "10000-0");
    starkbank_entity_set_string(receiver, "accountType", "checking");
    created = createOne(client, name, starkbank_split_receiver_create, receiver);
    if (created == NULL || starkbank_entity_string(created, "id", &id) != STARKBANK_OK) {
        starkbank_entity_free(created);
        return;
    }
    starkbank_split_receiver_params_new(&params);
    starkbank_entity_append_string(params, "ids", id);
    status = starkbank_split_receiver_query(client, params, 1, &iter);
    expectFirst(name, status, iter);
    starkbank_entity_free(params);
    starkbank_entity_free(created);
}

/* sdk-python's TestSplitProfilePut: daily, no delay. */
static void splitProfile(const starkbank_client *client)
{
    const char *name = "split profile put";
    starkbank_entity *profile = NULL;
    starkbank_list *batch = NULL;
    starkbank_list *put = NULL;
    starkbank_errors *errors = NULL;
    int status;

    starkbank_split_profile_new(&profile);
    starkbank_entity_set_string(profile, "interval", "day");
    starkbank_entity_set_number(profile, "delay", 0);
    starkbank_list_new(&batch);
    starkbank_list_append(batch, profile);
    status = starkbank_split_profile_put(client, batch, &put, &errors);
    starkbank_list_free(batch);
    if (requireOk(name, status, errors)) {
        check(name, starkbank_list_count(put) == 1, "the API answered with no profile");
    }
    starkbank_list_free(put);
}

/* A Split exists only once a split invoice is paid, so an empty workspace is
   missing data; the log of one that exists must exist too. */
static void splitAndLog(const starkbank_client *client)
{
    const char *name = "split query and log query";
    starkbank_iter *iter = NULL;
    const starkbank_entity *split = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;
    int status = starkbank_split_query(client, NULL, 1, &iter);

    if (status == STARKBANK_OK) {
        status = starkbank_iter_next(iter, &split, &errors);
    }
    if (requireOk(name, status, errors)
            && (split == NULL || starkbank_entity_string(split, "id", &id) != STARKBANK_OK)) {
        skip(name, "no split in this workspace");
    }
    if (id != NULL) {
        expectLog(client, name, starkbank_split_log_params_new, starkbank_split_log_query, "splitIds", id);
    }
    starkbank_iter_free(iter);
}

void sandboxVerifiedSplit(const starkbank_client *client, const starkbank_user *organization)
{
    (void)organization;
    verifiedAccount(client);
    splitReceiver(client);
    splitProfile(client);
    splitAndLog(client);
}
