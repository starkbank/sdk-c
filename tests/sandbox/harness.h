/*
 * The sandbox suite: the "Na sandbox" checks of the C-family Plane cards,
 * run live against the Stark sandbox through the public header only, so the
 * same sources build against libstarkbank_full_curl.a in this tree and
 * against an unpacked release tarball (make sandbox-dist).
 *
 * Every other file here is one group, and one group is one Plane card. A
 * group gets a client on the project credential and, when the organization
 * variables are set, the organization user; it reports through the four
 * functions below, and its output is the QA evidence for the card.
 */

#ifndef SANDBOX_HARNESS_H
#define SANDBOX_HARNESS_H

#include "starkbank.h"

typedef void (*SandboxGroup)(const starkbank_client *client, const starkbank_user *organization);

/* Prints "\n<name> (<card>)". */
void startGroup(const char *name, const char *card);

/* "  ok   <name>", or "  FAIL <name>: <detail>" when detail is not NULL. */
void check(const char *name, int ok, const char *detail);

/* "  skip <name>: <reason>". Sandbox data or a credential the check needs is
   missing: never counted as passed. */
void skip(const char *name, const char *reason);

/* 1 on STARKBANK_OK, printing nothing. Otherwise prints FAIL with
   starkbank_strerror(status) and each "code: message" the API returned,
   frees errors, and returns 0, so the caller skips what depended on it. */
int requireOk(const char *name, int status, starkbank_errors *errors);

/* One Invoice from sdk-python's tests/utils/invoice.py example: its REQUIRED
   amount, CPF and name, plus fine and interest when non-zero. Returns the
   created Invoice, yours to free, or NULL after printing FAIL under name. */
starkbank_entity *createInvoice(const starkbank_client *client, const char *name, double fine,
                                double interest);

/* Invoices have no delete; canceling is the cleanup the API allows. NULL is
   accepted, so a group can call it whatever its create returned. */
void cancelInvoice(const starkbank_client *client, const starkbank_entity *invoice);

/* Takes the first entity of a query the caller opened (status is what the
   *_query call returned) and passes when there is one; frees iter. */
void expectFirst(const char *name, int status, starkbank_iter *iter);

void sandboxAuth(const starkbank_client *client, const starkbank_user *organization);
void sandboxSignature(const starkbank_client *client, const starkbank_user *organization);
void sandboxCore(const starkbank_client *client, const starkbank_user *organization);
void sandboxWebhookPreview(const starkbank_client *client, const starkbank_user *organization);
void sandboxBoleto(const starkbank_client *client, const starkbank_user *organization);

#endif
