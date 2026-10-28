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

void sandboxAuth(const starkbank_client *client, const starkbank_user *organization);

#endif
