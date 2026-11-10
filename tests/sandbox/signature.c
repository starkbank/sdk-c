/*
 * ecdsa-c#1, core-c#1, sdk-c#6: real webhook bodies verify against the
 * public key the sandbox serves, and the same bodies tampered do not. The
 * overlong rewrite is the one the core-c#1 review used: 'i' (0x69) re-encoded
 * as the two bytes C1 A9, which a lenient UTF-8 decoder reads back as 'i'.
 */

#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "signed.h"

/* A heap copy of the signed event body with one edit, so the original stays
   the bytes Stark signed. NULL when the edit has nothing to apply to. */
static char *flipLastByte(void)
{
    size_t length = strlen(signedEventContent);
    char *copy = malloc(length + 1);

    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, signedEventContent, length + 1);
    copy[length - 1] = (char)(copy[length - 1] ^ 0x01);
    return copy;
}

static char *overlongFirstI(void)
{
    size_t length = strlen(signedEventContent);
    const char *found = strchr(signedEventContent, 'i');
    size_t offset;
    char *copy;

    if (found == NULL) {
        return NULL;
    }
    offset = (size_t)(found - signedEventContent);
    copy = malloc(length + 2);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, signedEventContent, offset);
    copy[offset] = (char)0xC1;
    copy[offset + 1] = (char)0xA9;
    memcpy(copy + offset + 2, found + 1, length - offset);
    return copy;
}

static void realEvent(const starkbank_client *client)
{
    starkbank_entity *event = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;

    if (!requireOk("a real sandbox Event verifies and hydrates",
                   starkbank_event_parse(client, signedEventContent, strlen(signedEventContent),
                                         signedEventSignature, &event, &errors), errors)) {
        return;
    }
    check("a real sandbox Event verifies and hydrates",
          starkbank_entity_string(event, STARKBANK_EVENT_ID, &id) == STARKBANK_OK && id[0] != '\0',
          "the event carries no id");
    starkbank_entity_free(event);
}

static void tamperedEvent(const starkbank_client *client, const char *name, char *content)
{
    starkbank_entity *event = NULL;
    starkbank_errors *errors = NULL;
    int status;

    if (content == NULL) {
        skip(name, "could not build the tampered body");
        return;
    }
    status = starkbank_event_parse(client, content, strlen(content), signedEventSignature,
                                   &event, &errors);
    check(name, status == STARKCORE_ERROR_SIGNATURE || status == STARKCORE_ERROR_ENCODING,
          starkbank_strerror(status));
    starkbank_entity_free(event);
    starkbank_errors_free(errors);
    free(content);
}

static void realPurchase(const starkbank_client *client)
{
    starkbank_entity *purchase = NULL;
    starkbank_errors *errors = NULL;

    if (requireOk("a real corporate purchase verifies",
                  starkbank_corporate_purchase_parse(client, signedPurchaseContent,
                                                     strlen(signedPurchaseContent),
                                                     signedPurchaseSignature, &purchase, &errors),
                  errors)) {
        check("a real corporate purchase verifies", 1, NULL);
    }
    starkbank_entity_free(purchase);
}

static void responseBody(const char *name, const char *status, const char *reason,
                         const char *expected)
{
    char *body = NULL;
    int code = starkbank_corporate_purchase_response(status, 0, 0, reason, NULL, &body);

    check(name, code == STARKBANK_OK && body != NULL && strstr(body, expected) != NULL,
          body != NULL ? body : starkbank_strerror(code));
    starkbank_free(body);
}

void sandboxSignature(const starkbank_client *client, const starkbank_user *organization)
{
    (void)organization;
    realEvent(client);
    tamperedEvent(client, "the same Event with one byte flipped is refused", flipLastByte());
    tamperedEvent(client, "an overlong UTF-8 rewrite of the body is refused", overlongFirstI());
    realPurchase(client);
    responseBody("approval body is built", "approved", NULL, "\"status\":\"approved\"");
    responseBody("denial body carries the reason", "denied", "other", "\"reason\":\"other\"");
}
