/*
 * The slice suite: Invoice, Transfer, Event, Balance against the goldens
 * tests/tools/record_fixtures.py recorded out of sdk-python.
 *
 * Two layers, and they fail for different reasons.
 *
 *   request    what this library would have put on the wire, compared with
 *              what python put on the wire for the same call. This is where
 *              the pagination arithmetic, the comma-joined lists, the
 *              camelCasing and the dropped nulls are proven, and none of it
 *              is implemented here - it is core-c's, exercised through it.
 *   hydration  starkbank_entity_dump against json.dumps(api_json(obj)) over
 *              the same response body. A field python models and the table
 *              lacks is a diff; so is a field the table has and python does
 *              not.
 *
 * Both sides are canonicalised - sorted keys, compact separators, an integral
 * float spelled as an integer - because the two tiers sign the bytes they each
 * send and need not agree on key order, and because starkcore's DOM has one
 * number type. Everything else, including percent-encoding in a query string,
 * is compared exactly.
 *
 * This suite uses the real registry (starkbank/resources.h), which is why it
 * is a second binary rather than more of tests/run.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "starkbank.h"
#include "internal.h"

static int passed;
static int failed;

static void startGroup(const char *name)
{
    printf("\n%s\n", name);
}

static void check(const char *name, int ok, const char *detail)
{
    if (ok) {
        passed++;
        printf("  ok   %s\n", name);
        return;
    }
    failed++;
    printf("  FAIL %s\n", name);
    if (detail != NULL && detail[0] != '\0') {
        printf("       %s\n", detail);
    }
}

static int equalStrings(const char *left, const char *right)
{
    return left != NULL && right != NULL && strcmp(left, right) == 0;
}

static char *readFile(const char *name)
{
    char path[512];
    FILE *handle;
    char *buffer;
    long size;

    snprintf(path, sizeof(path), "%s/%s", TEST_FIXTURE_DIR, name);
    handle = fopen(path, "rb");
    if (handle == NULL) {
        return NULL;
    }
    fseek(handle, 0, SEEK_END);
    size = ftell(handle);
    fseek(handle, 0, SEEK_SET);
    buffer = (char *)malloc((size_t)size + 1);
    if (buffer == NULL) {
        fclose(handle);
        return NULL;
    }
    if (fread(buffer, 1, (size_t)size, handle) != (size_t)size) {
        free(buffer);
        fclose(handle);
        return NULL;
    }
    fclose(handle);
    buffer[size] = '\0';
    return buffer;
}

/* ------------------------------------------------------------- goldens */

static starkcore_json *goldenDocument;
static const starkcore_json *goldenCases;
static const starkcore_json *goldenResponses;

static const char *responseBody(const char *name)
{
    static char buffer[16][8192];
    static int slot;
    const starkcore_json *node = starkcore_json_get(goldenResponses, name);
    char *text = NULL;
    size_t length = 0;

    if (node == NULL || starkcore_json_print(node, &text, &length) != STARKCORE_OK) {
        return "{}";
    }
    slot = (slot + 1) % 16;
    snprintf(buffer[slot], sizeof(buffer[0]), "%s", text);
    starkcore_free(text);
    return buffer[slot];
}

/*
 * Sorted keys and compact separators, and nothing else: no field filtering and
 * no null dropping. starkbank_entity_dump is deliberately NOT used here even
 * though it sorts, because it drops nulls the way python's api_json does - and
 * that is right for a hydration golden and wrong for a request golden. A
 * request body that carries a stray null (cJSON prints a non-finite number as
 * one) must be able to fail against python's normalise_body, which keeps it.
 */
static int canonicalNode(const starkcore_json *node, starkcore_json **out);

static int canonicalObject(const starkcore_json *object, starkcore_json **out)
{
    starkcore_json *result = NULL;
    starkcore_json *copy = NULL;
    int count = starkcore_json_size(object);
    int order[128];
    int used = 0;
    int index;
    int scan;
    int slot;
    int status = starkcore_json_new_object(&result);

    if (status != STARKCORE_OK) {
        return status;
    }
    if (count > (int)(sizeof(order) / sizeof(order[0]))) {
        starkcore_json_free(result);
        return STARKCORE_ERROR_ARGUMENT;
    }
    for (index = 0; index < count; index++) {
        slot = used;
        for (scan = 0; scan < used; scan++) {
            if (strcmp(starkcore_json_key_at(object, index),
                       starkcore_json_key_at(object, order[scan])) < 0) {
                slot = scan;
                break;
            }
        }
        for (scan = used; scan > slot; scan--) {
            order[scan] = order[scan - 1];
        }
        order[slot] = index;
        used++;
    }
    for (index = 0; index < used; index++) {
        status = canonicalNode(starkcore_json_at(object, order[index]), &copy);
        if (status == STARKCORE_OK) {
            status = starkcore_json_set_json(result,
                starkcore_json_key_at(object, order[index]), copy);
        }
        if (status != STARKCORE_OK) {
            starkcore_json_free(result);
            return status;
        }
    }
    *out = result;
    return STARKCORE_OK;
}

static int canonicalNode(const starkcore_json *node, starkcore_json **out)
{
    starkcore_json *array = NULL;
    starkcore_json *element = NULL;
    int status;
    int index;

    if (node == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (starkcore_json_type(node) == STARKCORE_JSON_OBJECT) {
        return canonicalObject(node, out);
    }
    if (starkcore_json_type(node) != STARKCORE_JSON_ARRAY) {
        return starkcore_json_clone(node, out);
    }
    status = starkcore_json_new_array(&array);
    if (status != STARKCORE_OK) {
        return status;
    }
    for (index = 0; index < starkcore_json_size(node); index++) {
        status = canonicalNode(starkcore_json_at(node, index), &element);
        if (status == STARKCORE_OK) {
            status = starkcore_json_append(array, element);
        }
        if (status != STARKCORE_OK) {
            starkcore_json_free(array);
            return status;
        }
    }
    *out = array;
    return STARKCORE_OK;
}

static char *canonical(const char *text)
{
    starkcore_json *document = NULL;
    starkcore_json *sorted = NULL;
    char *dumped = NULL;

    if (text == NULL || text[0] == '\0') {
        return NULL;
    }
    if (starkcore_json_parse(text, strlen(text), &document) != STARKCORE_OK) {
        return NULL;
    }
    if (canonicalNode(document, &sorted) != STARKCORE_OK) {
        starkcore_json_free(document);
        return NULL;
    }
    starkcore_json_free(document);
    starkcore_json_print(sorted, &dumped, NULL);
    starkcore_json_free(sorted);
    return dumped;
}

/* Query pairs sorted, values untouched: tags=war%2Csupply must stay encoded. */
static void sortedQuery(const char *query, char *out, size_t size)
{
    char work[1024];
    const char *pairs[32];
    size_t used = 0;
    size_t length;
    int count = 0;
    int index;
    int scan;
    char *cursor;
    char *token;
    const char *swap;

    out[0] = '\0';
    if (query == NULL || query[0] == '\0') {
        return;
    }
    snprintf(work, sizeof(work), "%s", query);
    token = work;
    for (cursor = work; count < 32; cursor++) {
        if (*cursor != '&' && *cursor != '\0') {
            continue;
        }
        if (*cursor == '&') {
            *cursor = '\0';
            pairs[count++] = token;
            token = cursor + 1;
            continue;
        }
        pairs[count++] = token;
        break;
    }
    for (index = 1; index < count; index++) {
        for (scan = index; scan > 0 && strcmp(pairs[scan - 1], pairs[scan]) > 0; scan--) {
            swap = pairs[scan - 1];
            pairs[scan - 1] = pairs[scan];
            pairs[scan] = swap;
        }
    }
    for (index = 0; index < count; index++) {
        length = strlen(pairs[index]);
        if (used + length + 2 >= size) {
            break;
        }
        if (used > 0) {
            out[used++] = '&';
        }
        memcpy(out + used, pairs[index], length);
        used += length;
    }
    out[used] = '\0';
}

/* ------------------------------------------------------- fake transport */

#define FAKE_MAX_CALLS 4
#define FAKE_URL 1024
#define FAKE_BODY 8192

typedef struct {
    int calls;
    int method[FAKE_MAX_CALLS];
    char url[FAKE_MAX_CALLS][FAKE_URL];
    char body[FAKE_MAX_CALLS][FAKE_BODY];
    int status;
    const char *replies[FAKE_MAX_CALLS];
    int replyCount;
} Fake;

static int STARKBANK_CALL fakeTransport(void *context, int method, const char *url,
                                        const starkbank_headers *headers, const char *body,
                                        size_t bodyLength, int timeoutSeconds,
                                        starkbank_response **out)
{
    Fake *fake = (Fake *)context;
    const char *reply;
    size_t copyLength;

    (void)headers;
    (void)timeoutSeconds;
    if (fake->calls >= FAKE_MAX_CALLS) {
        return STARKCORE_ERROR_TRANSPORT;
    }
    fake->method[fake->calls] = method;
    snprintf(fake->url[fake->calls], FAKE_URL, "%s", url);
    copyLength = bodyLength < FAKE_BODY - 1 ? bodyLength : FAKE_BODY - 1;
    memcpy(fake->body[fake->calls], body, copyLength);
    fake->body[fake->calls][copyLength] = '\0';
    /* The last queued reply repeats. A verify that falls back to refreshing
       the public key makes a second fetch, and it must get the same key
       rather than an empty body. */
    reply = fake->calls < fake->replyCount ? fake->replies[fake->calls]
          : (fake->replyCount > 0 ? fake->replies[fake->replyCount - 1] : "{}");
    fake->calls++;
    return starkbank_response_new(fake->status, (const unsigned char *)reply, strlen(reply),
                                  NULL, out);
}

static char *privatePem;

static starkbank_client *newClient(Fake *fake)
{
    starkbank_user *project = NULL;
    starkbank_client *client = NULL;

    memset(fake, 0, sizeof(*fake));
    fake->status = 200;
    if (starkbank_project_new("5656565656565656", STARKBANK_ENVIRONMENT_SANDBOX,
                              privatePem, &project) != STARKBANK_OK) {
        return NULL;
    }
    if (starkbank_client_new(project, &client) != STARKBANK_OK) {
        return NULL;
    }
    starkbank_client_set_transport(client, fakeTransport, fake);
    return client;
}

static void replies(Fake *fake, const char *first, const char *second)
{
    fake->replies[0] = first;
    fake->replies[1] = second;
    fake->replyCount = second != NULL ? 2 : 1;
}

/* ------------------------------------------------- golden comparisons */

static const char *methodNames[] = { "GET", "POST", "PUT", "PATCH", "DELETE" };

static void checkRequests(const char *name, const Fake *fake)
{
    const starkcore_json *recorded = starkcore_json_get(goldenCases, name);
    const starkcore_json *list;
    const starkcore_json *entry;
    char label[160];
    char detail[2048];
    char query[1024];
    char path[FAKE_URL];
    char *split;
    char *body;
    const char *expected;
    int index;

    if (recorded == NULL) {
        snprintf(label, sizeof(label), "%s: golden exists", name);
        check(label, 0, "not in tests/reference/slice.json; rerun the recorder");
        return;
    }
    list = starkcore_json_get(recorded, "requests");
    snprintf(label, sizeof(label), "%s: request count", name);
    snprintf(detail, sizeof(detail), "python made %d, this library made %d",
             starkcore_json_size(list), fake->calls);
    check(label, starkcore_json_size(list) == fake->calls, detail);
    if (starkcore_json_size(list) != fake->calls) {
        return;
    }
    for (index = 0; index < fake->calls; index++) {
        entry = starkcore_json_at(list, index);

        snprintf(path, sizeof(path), "%s", fake->url[index]);
        split = strchr(path, '?');
        if (split != NULL) {
            *split = '\0';
            sortedQuery(split + 1, query, sizeof(query));
        } else {
            query[0] = '\0';
        }

        expected = starkcore_json_string(starkcore_json_get(entry, "method"));
        snprintf(label, sizeof(label), "%s[%d]: method", name, index);
        check(label, equalStrings(methodNames[fake->method[index]], expected),
              methodNames[fake->method[index]]);

        expected = starkcore_json_string(starkcore_json_get(entry, "url"));
        snprintf(label, sizeof(label), "%s[%d]: url", name, index);
        snprintf(detail, sizeof(detail), "sent %s, python sent %s", path, expected);
        check(label, equalStrings(path, expected), detail);

        expected = starkcore_json_string(starkcore_json_get(entry, "query"));
        snprintf(label, sizeof(label), "%s[%d]: query", name, index);
        snprintf(detail, sizeof(detail), "sent [%s], python sent [%s]", query, expected);
        check(label, equalStrings(query, expected), detail);

        body = canonical(fake->body[index]);
        expected = starkcore_json_string(starkcore_json_get(entry, "body"));
        snprintf(label, sizeof(label), "%s[%d]: body", name, index);
        snprintf(detail, sizeof(detail), "sent [%s], python sent [%s]",
                 body != NULL ? body : "", expected != NULL ? expected : "");
        check(label, equalStrings(body != NULL ? body : "", expected != NULL ? expected : ""),
              detail);
        starkbank_free(body);
    }
}

static void checkHydration(const char *name, int index, const starkbank_entity *entity)
{
    const starkcore_json *recorded = starkcore_json_get(goldenCases, name);
    const starkcore_json *list;
    const char *expected;
    char label[160];
    char detail[4096];
    char *dumped = NULL;

    snprintf(label, sizeof(label), "%s: hydration matches python", name);
    if (recorded == NULL) {
        check(label, 0, "no golden");
        return;
    }
    list = starkcore_json_get(recorded, "hydrated");
    expected = starkcore_json_type(list) == STARKCORE_JSON_ARRAY
             ? starkcore_json_string(starkcore_json_at(list, index))
             : starkcore_json_string(list);
    if (starkbank_entity_dump(entity, &dumped, NULL) != STARKBANK_OK) {
        check(label, 0, "dump failed");
        return;
    }
    snprintf(detail, sizeof(detail), "\n       C      %s\n       python %s",
             dumped, expected != NULL ? expected : "(none)");
    check(label, equalStrings(dumped, expected), detail);
    starkbank_free(dumped);
}

/* ============================================================= registry */

/*
 * The registry is non-empty for the first time here, and it is what a Delphi
 * or .NET host reads to regenerate its own wrappers - tools/emit.py reads the
 * same rows. A resource whose table exists but whose starkbank/resources.h
 * line someone forgot is invisible to every one of those, and to nested
 * hydration, while its own verbs keep working; so the registry needs an
 * assertion of its own rather than being covered by accident.
 */
/* The comparator itself, because a canonicaliser that quietly drops a key can
   never make a golden fail and the whole slice rests on it. */
static void testCanonical(void)
{
    char *text;

    startGroup("request canonicaliser");
    text = canonical("{\"b\":1,\"a\":{\"d\":[2,1],\"c\":true}}");
    check("keys sort at every depth, list order is left alone",
          equalStrings(text, "{\"a\":{\"c\":true,\"d\":[2,1]},\"b\":1}"), text);
    starkbank_free(text);

    text = canonical("{\"amount\":400000,\"fine\":null}");
    check("a null in a request body survives, so it can fail a golden",
          equalStrings(text, "{\"amount\":400000,\"fine\":null}"), text);
    starkbank_free(text);
}

static void testRegistry(void)
{
    static const char *const expected[] = {
        "Balance", "Boleto", "BoletoLog", "BoletoPayment", "BoletoPaymentLog",
        "BrcodePayment", "BrcodePaymentLog", "BrcodePayment.Rule",
        "Event", "EventAttempt", "Invoice", "InvoiceLog",
        "Invoice.Payment", "Invoice.Rule", "PaymentPreview",
        "PaymentPreview.BoletoPreview", "PaymentPreview.BrcodePreview",
        "PaymentPreview.TaxPreview", "PaymentPreview.UtilityPreview",
        "Split", "Transfer", "TransferLog", "Transfer.Rule", "Webhook"
    };
    char label[160];
    size_t index;
    int count = (int)(sizeof(expected) / sizeof(expected[0]));

    startGroup("registry");
    check("every slice resource is registered, and nothing else is",
          starkbank_resource_count() == count, NULL);
    for (index = 0; index < sizeof(expected) / sizeof(expected[0]); index++) {
        snprintf(label, sizeof(label), "%s is reachable by name and reflects its fields",
                 expected[index]);
        check(label, starkbankRegistryFind(expected[index]) != NULL
              && starkbank_resource_field_count(expected[index]) > 0, NULL);
    }
    check("a params view is not in the registry: it hydrates nothing",
          starkbankRegistryFind("Invoice.Params") == NULL
          && starkbank_resource_field_count("Invoice.Params") == -1, NULL);
    check("Invoice reflects its table in order, types and flags included",
          starkbank_resource_field_count("Invoice") == 25
          && equalStrings(starkbank_resource_field_name_at("Invoice", 0), "amount")
          && starkbank_resource_field_type_at("Invoice", 0) == STARKBANK_FIELD_AMOUNT
          && starkbank_resource_field_flags_at("Invoice", 0)
              == (STARKBANK_FLAG_REQUIRED | STARKBANK_FLAG_CREATE | STARKBANK_FLAG_PATCH), NULL);
    check("Invoice.due is the DATE_OR_DATETIME the scheduled-invoice case needs",
          starkbank_resource_field_type_at("Invoice", 3) == STARKBANK_FIELD_DATE_OR_DATETIME,
          NULL);
    check("an Invoice.Rule value is a list and a Transfer.Rule value is a number",
          starkbank_resource_field_type_at("Invoice.Rule", 1) == STARKBANK_FIELD_LIST_STRING
          && starkbank_resource_field_type_at("Transfer.Rule", 1) == STARKBANK_FIELD_NUMBER,
          NULL);
}

/* ============================================================== Invoice */

static starkbank_entity * buildInvoice(void)
{
    starkbank_entity *invoice = NULL;
    starkbank_entity *child = NULL;

    if (starkbank_invoice_new(&invoice) != STARKBANK_OK) {
        return NULL;
    }
    starkbank_entity_set_amount(invoice, STARKBANK_INVOICE_AMOUNT, 400000);
    starkbank_entity_set_string(invoice, STARKBANK_INVOICE_TAX_ID, "012.345.678-90");
    starkbank_entity_set_string(invoice, STARKBANK_INVOICE_NAME, "Iron Bank S.A.");
    /* A date rather than a datetime: this is the scheduled-invoice semantic. */
    starkbank_entity_set_date(invoice, STARKBANK_INVOICE_DUE, 2026, 10, 28);
    starkbank_entity_set_seconds(invoice, STARKBANK_INVOICE_EXPIRATION, 123456789);
    starkbank_entity_set_number(invoice, STARKBANK_INVOICE_FINE, 2.5);
    starkbank_entity_set_number(invoice, STARKBANK_INVOICE_INTEREST, 1.3);
    starkbank_entity_append_string(invoice, STARKBANK_INVOICE_TAGS, "war");
    starkbank_entity_append_string(invoice, STARKBANK_INVOICE_TAGS, "supply");

    starkbank_object_new(&child);
    starkbank_entity_set_string(child, "key", "service");
    starkbank_entity_set_string(child, "value", "swords");
    starkbank_entity_append_entity(invoice, STARKBANK_INVOICE_DESCRIPTIONS, child);

    starkbank_object_new(&child);
    starkbank_entity_set_number(child, "percentage", 10.0);
    starkbank_entity_set_string(child, "due", "2026-10-01");
    starkbank_entity_append_entity(invoice, STARKBANK_INVOICE_DISCOUNTS, child);

    starkbank_invoice_rule_new(&child);
    starkbank_entity_set_string(child, STARKBANK_INVOICE_RULE_KEY, "allowedTaxIds");
    starkbank_entity_append_string(child, STARKBANK_INVOICE_RULE_VALUE, "012.345.678-90");
    starkbank_entity_append_string(child, STARKBANK_INVOICE_RULE_VALUE, "45.059.493/0001-73");
    starkbank_entity_append_entity(invoice, STARKBANK_INVOICE_RULES, child);

    starkbank_split_new(&child);
    starkbank_entity_set_amount(child, STARKBANK_SPLIT_AMOUNT, 141);
    starkbank_entity_set_string(child, STARKBANK_SPLIT_RECEIVER_ID, "5706627130851328");
    starkbank_entity_append_entity(invoice, STARKBANK_INVOICE_SPLITS, child);
    return invoice;
}

static void testInvoice(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_list *page = NULL;
    starkbank_entity *invoice = NULL;
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    unsigned char *blob = NULL;
    char *cursor = NULL;
    size_t blobLength = 0;
    int seen = 0;

    startGroup("Invoice");
    starkbank_client_free(client);
    client = newClient(&fake);

    replies(&fake, responseBody("invoices"), NULL);
    starkbank_list_new(&batch);
    starkbank_list_append(batch, buildInvoice());
    check("create returns the created list",
          starkbank_invoice_create(client, batch, &created, NULL) == STARKBANK_OK
          && starkbank_list_count(created) == 1, NULL);
    checkRequests("invoice.create", &fake);
    checkHydration("invoice.create", 0, starkbank_list_at(created, 0));
    check("a created Invoice carries no unknown key: the table is complete",
          starkbank_entity_unknown_count(starkbank_list_at(created, 0)) == 0, NULL);
    starkbank_list_free(batch);
    starkbank_list_free(created);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("invoice"), NULL);
    check("get hydrates one Invoice",
          starkbank_invoice_get(client, "5155165527080960", &invoice, NULL) == STARKBANK_OK, NULL);
    checkRequests("invoice.get", &fake);
    checkHydration("invoice.get", 0, invoice);
    starkbank_entity_free(invoice);
    invoice = NULL;

    /* Two pages and a limit of 150: the second request must ask for 50. That
       arithmetic is core-c's, and this is what proves we did not redo it. */
    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"invoices\":[{\"id\":\"1\"}],\"cursor\":\"c1\"}",
            "{\"invoices\":[{\"id\":\"2\"}],\"cursor\":\"\"}");
    starkbank_invoice_params_new(&params);
    starkbank_entity_set_string(params, "status", "paid");
    starkbank_entity_append_string(params, "tags", "war");
    starkbank_entity_append_string(params, "tags", "supply");
    starkbank_entity_set_date(params, "after", 2026, 1, 1);
    starkbank_entity_set_date(params, "before", 2026, 12, 31);
    starkbank_invoice_query(client, params, 150, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        seen++;
    }
    check("the stream walks both pages", seen == 2, NULL);
    checkRequests("invoice.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("invoices"), NULL);
    starkbank_invoice_params_new(&params);
    starkbank_entity_set_string(params, "cursor", "c1");
    starkbank_entity_set_number(params, "limit", 10);
    starkbank_entity_set_string(params, "status", "paid");
    check("page returns a list and the next cursor",
          starkbank_invoice_page(client, params, &page, &cursor, NULL) == STARKBANK_OK
          && starkbank_list_count(page) == 1, NULL);
    checkRequests("invoice.page", &fake);
    starkbank_free(cursor);
    starkbank_list_free(page);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("invoice"), NULL);
    starkbank_invoice_params_new(&params);
    starkbank_entity_set_string(params, STARKBANK_INVOICE_STATUS, "canceled");
    starkbank_entity_set_amount(params, STARKBANK_INVOICE_AMOUNT, 123);
    starkbank_entity_set_datetime(params, STARKBANK_INVOICE_DUE, 2026, 11, 1, 12, 0, 0);
    starkbank_entity_set_seconds(params, STARKBANK_INVOICE_EXPIRATION, 7200);
    check("update patches",
          starkbank_invoice_update(client, "5155165527080960", params, &invoice, NULL)
              == STARKBANK_OK, NULL);
    checkRequests("invoice.update", &fake);
    starkbank_entity_free(invoice);
    invoice = NULL;
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "%PDF-1.4 fake", NULL);
    check("pdf returns bytes",
          starkbank_invoice_pdf(client, "5155165527080960", &blob, &blobLength, NULL)
              == STARKBANK_OK && blobLength == 13, NULL);
    checkRequests("invoice.pdf", &fake);
    starkbank_free(blob);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "png", NULL);
    check("qrcode sends the size it was given",
          starkbank_invoice_qrcode(client, "5155165527080960", 12, &blob, &blobLength, NULL)
              == STARKBANK_OK, NULL);
    checkRequests("invoice.qrcode", &fake);
    starkbank_free(blob);

    check("a size outside 1..50 is refused locally, with nothing sent",
          starkbank_invoice_qrcode(client, "1", 51, &blob, &blobLength, NULL)
              == STARKCORE_ERROR_ARGUMENT && fake.calls == 1 && blob == NULL, NULL);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("invoicePayment"), NULL);
    check("payment hydrates the sub-resource under its own tag",
          starkbank_invoice_payment(client, "5155165527080960", &invoice, NULL) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(invoice), "Invoice.Payment"), NULL);
    checkRequests("invoice.payment", &fake);
    checkHydration("invoice.payment", 0, invoice);
    starkbank_entity_free(invoice);
    invoice = NULL;
    starkbank_client_free(client);
}

static void testInvoiceLog(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_entity *log = NULL;
    starkbank_entity *params = NULL;
    starkbank_list *page = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    const starkbank_entity *nested = NULL;
    unsigned char *blob = NULL;
    size_t blobLength = 0;
    const char *text = NULL;

    startGroup("invoice.Log");
    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("invoiceLog"), NULL);
    check("log get hydrates",
          starkbank_invoice_log_get(client, "6341320293482496", &log, NULL) == STARKBANK_OK, NULL);
    checkRequests("invoice.log.get", &fake);
    checkHydration("invoice.log.get", 0, log);
    check("the nested invoice is a tagged Invoice read with the same accessors",
          starkbank_entity_entity(log, STARKBANK_INVOICE_LOG_INVOICE, &nested) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(nested), "Invoice")
          && starkbank_entity_string(nested, STARKBANK_INVOICE_NAME, &text) == STARKBANK_OK
          && equalStrings(text, "Iron Bank S.A."), NULL);
    starkbank_entity_free(log);
    log = NULL;

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"logs\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_invoice_log_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_entity_append_string(params, "types", "paid");
    starkbank_entity_append_string(params, "invoiceIds", "5155165527080960");
    starkbank_invoice_log_query(client, params, 5, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        ;
    }
    checkRequests("invoice.log.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"logs\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_invoice_log_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_invoice_log_page(client, params, &page, NULL, NULL);
    checkRequests("invoice.log.page", &fake);
    starkbank_list_free(page);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "%PDF-1.4 fake", NULL);
    check("the log has its own pdf, which python has and Transfer's log does not",
          starkbank_invoice_log_pdf(client, "6341320293482496", &blob, &blobLength, NULL)
              == STARKBANK_OK, NULL);
    checkRequests("invoice.log.pdf", &fake);
    starkbank_free(blob);
    starkbank_client_free(client);
}

/* ============================================================= Transfer */

static void testTransfer(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_list *page = NULL;
    starkbank_entity *transfer = NULL;
    starkbank_entity *rule = NULL;
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    const starkbank_entity *nested = NULL;
    unsigned char *blob = NULL;
    size_t blobLength = 0;
    double number = 0.0;
    const char *text = NULL;

    startGroup("Transfer");
    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("transfers"), NULL);

    starkbank_transfer_new(&transfer);
    starkbank_entity_set_amount(transfer, STARKBANK_TRANSFER_AMOUNT, 1000);
    starkbank_entity_set_string(transfer, STARKBANK_TRANSFER_NAME, "Daenerys Targaryen Stormborn");
    starkbank_entity_set_string(transfer, STARKBANK_TRANSFER_TAX_ID, "594.739.480-42");
    starkbank_entity_set_string(transfer, STARKBANK_TRANSFER_BANK_CODE, "20018183");
    starkbank_entity_set_string(transfer, STARKBANK_TRANSFER_BRANCH_CODE, "1357-9");
    starkbank_entity_set_string(transfer, STARKBANK_TRANSFER_ACCOUNT_NUMBER, "876543-2");
    starkbank_entity_set_string(transfer, STARKBANK_TRANSFER_ACCOUNT_TYPE, "checking");
    /* The idempotency key the standards require of every funds-movement call. */
    starkbank_entity_set_string(transfer, STARKBANK_TRANSFER_EXTERNAL_ID, "my-internal-id-123456");
    starkbank_entity_set_datetime(transfer, STARKBANK_TRANSFER_SCHEDULED, 2026, 10, 28, 17, 59, 26);
    starkbank_entity_set_string(transfer, STARKBANK_TRANSFER_DESCRIPTION,
                                "Payment for service #1234");
    starkbank_entity_set_string(transfer, STARKBANK_TRANSFER_DISPLAY_DESCRIPTION,
                                "Sword sharpening");
    starkbank_entity_append_string(transfer, STARKBANK_TRANSFER_TAGS, "daenerys");
    starkbank_entity_append_string(transfer, STARKBANK_TRANSFER_TAGS, "invoice/1234");
    starkbank_transfer_rule_new(&rule);
    starkbank_entity_set_string(rule, STARKBANK_TRANSFER_RULE_KEY, "resendingLimit");
    /* A Transfer.Rule value is a number where an Invoice.Rule value is a list
       of strings, which is why the two sub-resources have separate tables. */
    starkbank_entity_set_number(rule, STARKBANK_TRANSFER_RULE_VALUE, 5);
    starkbank_entity_append_entity(transfer, STARKBANK_TRANSFER_RULES, rule);

    starkbank_list_new(&batch);
    starkbank_list_append(batch, transfer);
    check("create returns the created list",
          starkbank_transfer_create(client, batch, &created, NULL) == STARKBANK_OK
          && starkbank_list_count(created) == 1, NULL);
    checkRequests("transfer.create", &fake);
    checkHydration("transfer.create", 0, starkbank_list_at(created, 0));
    check("metadata - the key app-docs does not model - reads as an OBJECT",
          starkbank_entity_entity(starkbank_list_at(created, 0), STARKBANK_TRANSFER_METADATA,
                                  &nested) == STARKBANK_OK
          && starkbank_entity_string(nested, "tracker", &text) == STARKBANK_OK
          && equalStrings(text, "abc")
          && starkbank_entity_number(nested, "attempt", &number) == STARKBANK_OK
          && number == 2.0, NULL);
    check("and the table knows it, so nothing counts as unknown",
          starkbank_entity_unknown_count(starkbank_list_at(created, 0)) == 0, NULL);
    starkbank_list_free(batch);
    starkbank_list_free(created);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("transfer"), NULL);
    starkbank_transfer_get(client, "5155165527080960", &transfer, NULL);
    checkRequests("transfer.get", &fake);
    checkHydration("transfer.get", 0, transfer);
    starkbank_entity_free(transfer);
    transfer = NULL;

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("transfer"), NULL);
    check("delete cancels and returns the Transfer as it stands",
          starkbank_transfer_delete(client, "5155165527080960", &transfer, NULL)
              == STARKBANK_OK, NULL);
    checkRequests("transfer.delete", &fake);
    starkbank_entity_free(transfer);
    transfer = NULL;

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"transfers\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_transfer_params_new(&params);
    starkbank_entity_set_string(params, "status", "processing");
    starkbank_entity_set_string(params, "taxId", "594.739.480-42");
    starkbank_entity_set_string(params, "sort", "-created");
    starkbank_transfer_query(client, params, 5, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        ;
    }
    checkRequests("transfer.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"transfers\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_transfer_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_transfer_page(client, params, &page, NULL, NULL);
    checkRequests("transfer.page", &fake);
    starkbank_list_free(page);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "%PDF-1.4 fake", NULL);
    starkbank_transfer_pdf(client, "5155165527080960", &blob, &blobLength, NULL);
    checkRequests("transfer.pdf", &fake);
    starkbank_free(blob);
    starkbank_client_free(client);
}

static void testTransferLog(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_entity *log = NULL;
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    const starkbank_entity *nested = NULL;

    startGroup("transfer.Log");
    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("transferLog"), NULL);
    starkbank_transfer_log_get(client, "6341320293482496", &log, NULL);
    checkRequests("transfer.log.get", &fake);
    checkHydration("transfer.log.get", 0, log);
    check("the nested transfer is tagged",
          starkbank_entity_entity(log, STARKBANK_TRANSFER_LOG_TRANSFER, &nested) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(nested), "Transfer"), NULL);
    starkbank_entity_free(log);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"logs\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_transfer_log_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_entity_append_string(params, "types", "success");
    starkbank_entity_append_string(params, "transferIds", "5155165527080960");
    starkbank_transfer_log_query(client, params, 5, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        ;
    }
    checkRequests("transfer.log.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);
    starkbank_client_free(client);
}

/* ================================================================ Boleto */

static void testBoleto(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_list *page = NULL;
    starkbank_entity *boleto = NULL;
    starkbank_entity *params = NULL;
    starkbank_entity *child = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    unsigned char *blob = NULL;
    size_t blobLength = 0;

    startGroup("Boleto");
    client = newClient(&fake);
    replies(&fake, responseBody("boletos"), NULL);

    starkbank_boleto_new(&boleto);
    starkbank_entity_set_amount(boleto, STARKBANK_BOLETO_AMOUNT, 23456);
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_NAME, "Anthony Edward Stark");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_TAX_ID, "012.345.678-90");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_STREET_LINE_1, "Av. Paulista, 200");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_STREET_LINE_2, "Apto. 123");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_DISTRICT, "Bela Vista");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_CITY, "Sao Paulo");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_STATE_CODE, "SP");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_ZIP_CODE, "01311-200");
    /* A plain DATE: sdk-python's Boleto has no scheduled-invoice equivalent. */
    starkbank_entity_set_date(boleto, STARKBANK_BOLETO_DUE, 2026, 10, 28);
    starkbank_entity_set_number(boleto, STARKBANK_BOLETO_FINE, 2.5);
    starkbank_entity_set_number(boleto, STARKBANK_BOLETO_INTEREST, 1.0);
    starkbank_entity_set_number(boleto, STARKBANK_BOLETO_OVERDUE_LIMIT, 59);
    starkbank_entity_append_string(boleto, STARKBANK_BOLETO_TAGS, "war");
    starkbank_entity_append_string(boleto, STARKBANK_BOLETO_TAGS, "supply");

    starkbank_object_new(&child);
    starkbank_entity_set_string(child, "text", "sword sharpening");
    starkbank_entity_set_amount(child, "amount", 1234);
    starkbank_entity_append_entity(boleto, STARKBANK_BOLETO_DESCRIPTIONS, child);

    starkbank_object_new(&child);
    starkbank_entity_set_number(child, "percentage", 10.0);
    starkbank_entity_set_string(child, "date", "2026-10-01");
    starkbank_entity_append_entity(boleto, STARKBANK_BOLETO_DISCOUNTS, child);

    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_RECEIVER_NAME, "Iron Bank S.A.");
    starkbank_entity_set_string(boleto, STARKBANK_BOLETO_RECEIVER_TAX_ID, "20.018.183/0001-80");

    starkbank_list_new(&batch);
    starkbank_list_append(batch, boleto);
    check("create returns the created list",
          starkbank_boleto_create(client, batch, &created, NULL) == STARKBANK_OK
          && starkbank_list_count(created) == 1, NULL);
    checkRequests("boleto.create", &fake);
    checkHydration("boleto.create", 0, starkbank_list_at(created, 0));
    check("a created Boleto carries no unknown key: the table is complete",
          starkbank_entity_unknown_count(starkbank_list_at(created, 0)) == 0, NULL);
    starkbank_list_free(batch);
    starkbank_list_free(created);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("boleto"), NULL);
    starkbank_boleto_get(client, "5155165527080960", &boleto, NULL);
    checkRequests("boleto.get", &fake);
    checkHydration("boleto.get", 0, boleto);
    starkbank_entity_free(boleto);
    boleto = NULL;

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("boleto"), NULL);
    check("delete cancels and returns the Boleto as it stands",
          starkbank_boleto_delete(client, "5155165527080960", &boleto, NULL) == STARKBANK_OK,
          NULL);
    checkRequests("boleto.delete", &fake);
    starkbank_entity_free(boleto);
    boleto = NULL;

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"boletos\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_boleto_params_new(&params);
    starkbank_entity_set_string(params, "status", "registered");
    starkbank_entity_append_string(params, "tags", "war");
    starkbank_entity_append_string(params, "tags", "supply");
    starkbank_boleto_query(client, params, 5, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        ;
    }
    checkRequests("boleto.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"boletos\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_boleto_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_boleto_page(client, params, &page, NULL, NULL);
    checkRequests("boleto.page", &fake);
    starkbank_list_free(page);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "%PDF-1.4 fake", NULL);
    check("pdf sends the optional layout and hiddenFields, and returns bytes",
          starkbank_boleto_pdf(client, "5155165527080960", "booklet",
                               "customerAddress,customerTaxId",
                               &blob, &blobLength, NULL) == STARKBANK_OK, NULL);
    checkRequests("boleto.pdf", &fake);
    starkbank_free(blob);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "%PDF-1.4 fake", NULL);
    check("pdf sends nothing when layout and hiddenFields are both omitted",
          starkbank_boleto_pdf(client, "5155165527080960", NULL, NULL,
                               &blob, &blobLength, NULL) == STARKBANK_OK
          && strchr(fake.url[0], '?') == NULL, NULL);
    starkbank_free(blob);
    starkbank_client_free(client);
}

static void testBoletoLog(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_entity *log = NULL;
    starkbank_entity *params = NULL;
    starkbank_list *page = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    const starkbank_entity *nested = NULL;

    startGroup("boleto.Log");
    client = newClient(&fake);
    replies(&fake, responseBody("boletoLog"), NULL);
    check("log get hydrates",
          starkbank_boleto_log_get(client, "6341320293482496", &log, NULL) == STARKBANK_OK,
          NULL);
    checkRequests("boleto.log.get", &fake);
    checkHydration("boleto.log.get", 0, log);
    check("the nested boleto is a tagged Boleto read with the same accessors",
          starkbank_entity_entity(log, STARKBANK_BOLETO_LOG_BOLETO, &nested) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(nested), "Boleto"), NULL);
    starkbank_entity_free(log);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"logs\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_boleto_log_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_entity_append_string(params, "types", "registered");
    starkbank_entity_append_string(params, "boletoIds", "5155165527080960");
    starkbank_boleto_log_query(client, params, 5, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        ;
    }
    checkRequests("boleto.log.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"logs\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_boleto_log_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_boleto_log_page(client, params, &page, NULL, NULL);
    checkRequests("boleto.log.page", &fake);
    starkbank_list_free(page);
    starkbank_entity_free(params);
    starkbank_client_free(client);
}

/* ========================================================= BoletoPayment */

static void testBoletoPayment(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_list *page = NULL;
    starkbank_entity *payment = NULL;
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    unsigned char *blob = NULL;
    size_t blobLength = 0;

    startGroup("BoletoPayment");
    client = newClient(&fake);
    replies(&fake, responseBody("boletoPayments"), NULL);

    starkbank_boleto_payment_new(&payment);
    starkbank_entity_set_string(payment, STARKBANK_BOLETO_PAYMENT_TAX_ID, "20.018.183/0001-80");
    starkbank_entity_set_string(payment, STARKBANK_BOLETO_PAYMENT_DESCRIPTION,
                                "sword sharpening");
    starkbank_entity_set_string(payment, STARKBANK_BOLETO_PAYMENT_LINE,
        "34191.09008 63571.277308 71444.640008 5 81960000000062");
    starkbank_entity_set_date(payment, STARKBANK_BOLETO_PAYMENT_SCHEDULED, 2026, 10, 28);
    starkbank_entity_append_string(payment, STARKBANK_BOLETO_PAYMENT_TAGS, "war");
    starkbank_entity_append_string(payment, STARKBANK_BOLETO_PAYMENT_TAGS, "supply");

    starkbank_list_new(&batch);
    starkbank_list_append(batch, payment);
    check("create returns the created list",
          starkbank_boleto_payment_create(client, batch, &created, NULL) == STARKBANK_OK
          && starkbank_list_count(created) == 1, NULL);
    checkRequests("boletopayment.create", &fake);
    checkHydration("boletopayment.create", 0, starkbank_list_at(created, 0));
    check("a created BoletoPayment carries no unknown key",
          starkbank_entity_unknown_count(starkbank_list_at(created, 0)) == 0, NULL);
    starkbank_list_free(batch);
    starkbank_list_free(created);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("boletoPayment"), NULL);
    starkbank_boleto_payment_get(client, "5155165527080960", &payment, NULL);
    checkRequests("boletopayment.get", &fake);
    checkHydration("boletopayment.get", 0, payment);
    starkbank_entity_free(payment);
    payment = NULL;

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("boletoPayment"), NULL);
    check("delete cancels an unprocessed payment",
          starkbank_boleto_payment_delete(client, "5155165527080960", &payment, NULL)
              == STARKBANK_OK, NULL);
    checkRequests("boletopayment.delete", &fake);
    starkbank_entity_free(payment);
    payment = NULL;

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"payments\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_boleto_payment_params_new(&params);
    starkbank_entity_set_string(params, "status", "success");
    starkbank_entity_append_string(params, "tags", "war");
    starkbank_entity_append_string(params, "tags", "supply");
    starkbank_boleto_payment_query(client, params, 5, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        ;
    }
    checkRequests("boletopayment.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"payments\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_boleto_payment_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_boleto_payment_page(client, params, &page, NULL, NULL);
    checkRequests("boletopayment.page", &fake);
    starkbank_list_free(page);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "%PDF-1.4 fake", NULL);
    starkbank_boleto_payment_pdf(client, "5155165527080960", &blob, &blobLength, NULL);
    checkRequests("boletopayment.pdf", &fake);
    starkbank_free(blob);
    starkbank_client_free(client);
}

static void testBoletoPaymentLog(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_entity *log = NULL;
    starkbank_entity *params = NULL;
    starkbank_list *page = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    const starkbank_entity *nested = NULL;

    startGroup("boletopayment.Log");
    client = newClient(&fake);
    replies(&fake, responseBody("boletoPaymentLog"), NULL);
    starkbank_boleto_payment_log_get(client, "6341320293482497", &log, NULL);
    checkRequests("boletopayment.log.get", &fake);
    checkHydration("boletopayment.log.get", 0, log);
    check("the nested payment is a tagged BoletoPayment",
          starkbank_entity_entity(log, STARKBANK_BOLETO_PAYMENT_LOG_PAYMENT, &nested)
              == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(nested), "BoletoPayment"), NULL);
    starkbank_entity_free(log);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"logs\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_boleto_payment_log_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_entity_append_string(params, "types", "success");
    starkbank_entity_append_string(params, "paymentIds", "5155165527080960");
    starkbank_boleto_payment_log_query(client, params, 5, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        ;
    }
    checkRequests("boletopayment.log.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"logs\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_boleto_payment_log_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_boleto_payment_log_page(client, params, &page, NULL, NULL);
    checkRequests("boletopayment.log.page", &fake);
    starkbank_list_free(page);
    starkbank_entity_free(params);
    starkbank_client_free(client);
}

/* ========================================================= BrcodePayment */

static void testBrcodePayment(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_list *page = NULL;
    starkbank_entity *payment = NULL;
    starkbank_entity *rule = NULL;
    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    unsigned char *blob = NULL;
    size_t blobLength = 0;

    startGroup("BrcodePayment");
    client = newClient(&fake);
    replies(&fake, responseBody("brcodePayments"), NULL);

    starkbank_brcode_payment_new(&payment);
    starkbank_entity_set_string(payment, STARKBANK_BRCODE_PAYMENT_BRCODE,
        "00020126580014br.gov.bcb.pix0136a629532e-7693-4846-852d-1bbff817b5a8"
        "520400005303986540510.005802BR5908T'Challa6009Sao Paulo62090505123456304B14A");
    starkbank_entity_set_string(payment, STARKBANK_BRCODE_PAYMENT_TAX_ID, "012.345.678-90");
    starkbank_entity_set_string(payment, STARKBANK_BRCODE_PAYMENT_DESCRIPTION,
                                "sword sharpening");
    starkbank_entity_set_amount(payment, STARKBANK_BRCODE_PAYMENT_AMOUNT, 23456);
    starkbank_entity_set_date(payment, STARKBANK_BRCODE_PAYMENT_SCHEDULED, 2026, 10, 28);
    starkbank_entity_append_string(payment, STARKBANK_BRCODE_PAYMENT_TAGS, "war");
    starkbank_entity_append_string(payment, STARKBANK_BRCODE_PAYMENT_TAGS, "supply");

    starkbank_brcode_payment_rule_new(&rule);
    starkbank_entity_set_string(rule, STARKBANK_BRCODE_PAYMENT_RULE_KEY, "resendingLimit");
    /* A BrcodePayment.Rule value is a number, like Transfer.Rule's. */
    starkbank_entity_set_number(rule, STARKBANK_BRCODE_PAYMENT_RULE_VALUE, 5);
    starkbank_entity_append_entity(payment, STARKBANK_BRCODE_PAYMENT_RULES, rule);

    starkbank_list_new(&batch);
    starkbank_list_append(batch, payment);
    check("create returns the created list",
          starkbank_brcode_payment_create(client, batch, &created, NULL) == STARKBANK_OK
          && starkbank_list_count(created) == 1, NULL);
    checkRequests("brcodepayment.create", &fake);
    checkHydration("brcodepayment.create", 0, starkbank_list_at(created, 0));
    check("a created BrcodePayment carries no unknown key",
          starkbank_entity_unknown_count(starkbank_list_at(created, 0)) == 0, NULL);
    starkbank_list_free(batch);
    starkbank_list_free(created);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("brcodePayment"), NULL);
    starkbank_brcode_payment_get(client, "5155165527080960", &payment, NULL);
    checkRequests("brcodepayment.get", &fake);
    checkHydration("brcodepayment.get", 0, payment);
    starkbank_entity_free(payment);
    payment = NULL;

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"payments\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_brcode_payment_params_new(&params);
    starkbank_entity_set_string(params, "status", "success");
    starkbank_entity_append_string(params, "tags", "war");
    starkbank_entity_append_string(params, "tags", "supply");
    starkbank_brcode_payment_query(client, params, 5, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        ;
    }
    checkRequests("brcodepayment.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"payments\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_brcode_payment_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_brcode_payment_page(client, params, &page, NULL, NULL);
    checkRequests("brcodepayment.page", &fake);
    starkbank_list_free(page);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("brcodePayment"), NULL);
    starkbank_brcode_payment_params_new(&params);
    starkbank_entity_set_string(params, STARKBANK_BRCODE_PAYMENT_STATUS, "canceled");
    check("update patches only status",
          starkbank_brcode_payment_update(client, "5155165527080960", params, &payment, NULL)
              == STARKBANK_OK, NULL);
    checkRequests("brcodepayment.update", &fake);
    starkbank_entity_free(payment);
    payment = NULL;
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "%PDF-1.4 fake", NULL);
    starkbank_brcode_payment_pdf(client, "5155165527080960", &blob, &blobLength, NULL);
    checkRequests("brcodepayment.pdf", &fake);
    starkbank_free(blob);
    starkbank_client_free(client);
}

static void testBrcodePaymentLog(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_entity *log = NULL;
    starkbank_entity *params = NULL;
    starkbank_list *page = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    const starkbank_entity *nested = NULL;

    startGroup("brcodepayment.Log");
    client = newClient(&fake);
    replies(&fake, responseBody("brcodePaymentLog"), NULL);
    starkbank_brcode_payment_log_get(client, "6341320293482498", &log, NULL);
    checkRequests("brcodepayment.log.get", &fake);
    checkHydration("brcodepayment.log.get", 0, log);
    check("the nested payment is a tagged BrcodePayment",
          starkbank_entity_entity(log, STARKBANK_BRCODE_PAYMENT_LOG_PAYMENT, &nested)
              == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(nested), "BrcodePayment"), NULL);
    starkbank_entity_free(log);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"logs\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_brcode_payment_log_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_entity_append_string(params, "types", "success");
    starkbank_entity_append_string(params, "paymentIds", "5155165527080960");
    starkbank_brcode_payment_log_query(client, params, 5, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        ;
    }
    checkRequests("brcodepayment.log.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"logs\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_brcode_payment_log_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_brcode_payment_log_page(client, params, &page, NULL, NULL);
    checkRequests("brcodepayment.log.page", &fake);
    starkbank_list_free(page);
    starkbank_entity_free(params);
    starkbank_client_free(client);
}

/* ================================================================ Event */

static void testEvent(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_entity *event = NULL;
    starkbank_entity *params = NULL;
    starkbank_list *page = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    const starkbank_entity *log = NULL;
    const starkbank_entity *invoice = NULL;
    const char *text = NULL;

    startGroup("Event, and the polymorphic log");
    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("event"), NULL);
    check("get hydrates", starkbank_event_get(client, "5764898044149760", &event, NULL)
          == STARKBANK_OK, NULL);
    checkRequests("event.get", &fake);
    checkHydration("event.get", 0, event);
    check("an invoice subscription makes log an InvoiceLog, two levels down",
          starkbank_entity_entity(event, STARKBANK_EVENT_LOG, &log) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(log), "InvoiceLog")
          && starkbank_entity_entity(log, STARKBANK_INVOICE_LOG_INVOICE, &invoice)
              == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(invoice), "Invoice")
          && starkbank_entity_string(invoice, STARKBANK_INVOICE_BRCODE, &text) == STARKBANK_OK,
          NULL);
    starkbank_entity_free(event);
    event = NULL;

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("eventTransfer"), NULL);
    starkbank_event_get(client, "4823", &event, NULL);
    checkHydration("event.transfer", 0, event);
    check("a transfer subscription makes the same field a TransferLog",
          starkbank_entity_entity(event, STARKBANK_EVENT_LOG, &log) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(log), "TransferLog"), NULL);
    starkbank_entity_free(event);
    event = NULL;

    /* The case the design cares about: a subscription this build predates
       must still hydrate, readable and counted, rather than fail. */
    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("eventUnknown"), NULL);
    starkbank_event_get(client, "4824", &event, NULL);
    checkHydration("event.unknown", 0, event);
    check("an unknown subscription leaves log untagged and permissive",
          starkbank_entity_entity(event, STARKBANK_EVENT_LOG, &log) == STARKBANK_OK
          && starkbank_entity_resource(log) == NULL
          && starkbank_entity_string(log, "id", &text) == STARKBANK_OK
          && equalStrings(text, "1"), NULL);
    check("and the gap is one unknown, so the counter tracks the gap not the object",
          starkbank_entity_unknown_count(event) == 1, NULL);
    starkbank_entity_free(event);
    event = NULL;

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"events\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_event_params_new(&params);
    starkbank_entity_set_bool(params, "isDelivered", 0);
    starkbank_event_query(client, params, 5, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        ;
    }
    checkRequests("event.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"events\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_event_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_event_page(client, params, &page, NULL, NULL);
    checkRequests("event.page", &fake);
    starkbank_list_free(page);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("event"), NULL);
    starkbank_event_params_new(&params);
    starkbank_entity_set_bool(params, STARKBANK_EVENT_IS_DELIVERED, 1);
    check("update sends only isDelivered",
          starkbank_event_update(client, "5764898044149760", params, &event, NULL)
              == STARKBANK_OK, NULL);
    checkRequests("event.update", &fake);
    starkbank_entity_free(event);
    event = NULL;
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("event"), NULL);
    starkbank_event_delete(client, "5764898044149760", &event, NULL);
    checkRequests("event.delete", &fake);
    starkbank_entity_free(event);
    starkbank_client_free(client);
}

static void testEventAttempt(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_entity *attempt = NULL;
    starkbank_entity *params = NULL;
    starkbank_list *page = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;

    startGroup("event.Attempt");
    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("eventAttempt"), NULL);
    starkbank_event_attempt_get(client, "1616161616161616", &attempt, NULL);
    /* "EventAttempt" becoming /v2/event/attempt/<id> is core-c's rewrite,
       not a path anyone spelled in this repo. */
    checkRequests("event.attempt.get", &fake);
    checkHydration("event.attempt.get", 0, attempt);
    starkbank_entity_free(attempt);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"attempts\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_event_attempt_params_new(&params);
    starkbank_entity_append_string(params, "eventIds", "5764898044149760");
    starkbank_event_attempt_query(client, params, 5, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        ;
    }
    checkRequests("event.attempt.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, "{\"attempts\":[{\"id\":\"1\"}],\"cursor\":\"\"}", NULL);
    starkbank_event_attempt_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_event_attempt_page(client, params, &page, NULL, NULL);
    checkRequests("event.attempt.page", &fake);
    starkbank_list_free(page);
    starkbank_entity_free(params);
    starkbank_client_free(client);
}

static void buildKeyReply(char *out, size_t capacity, const char *publicPem)
{
    starkcore_json *reply = NULL;
    starkcore_json *keys = NULL;
    starkcore_json *key = NULL;
    char *text = NULL;
    size_t length = 0;

    starkcore_json_new_object(&reply);
    starkcore_json_new_array(&keys);
    starkcore_json_new_object(&key);
    starkcore_json_set_string(key, "content", publicPem);
    starkcore_json_append(keys, key);
    starkcore_json_set_json(reply, "publicKeys", keys);
    starkcore_json_print(reply, &text, &length);
    snprintf(out, capacity, "%s", text);
    starkcore_free(text);
    starkcore_json_free(reply);
}

static void testEventParse(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_entity *event = NULL;
    const starkbank_entity *log = NULL;
    starkcore_user *signer = NULL;
    char *publicPem;
    char *signature = NULL;
    char keyReply[4096];
    const char *body =
        "{\"event\":{\"id\":\"5764898044149760\",\"subscription\":\"invoice\","
        "\"isDelivered\":false,\"log\":{\"id\":\"9\",\"type\":\"paid\","
        "\"invoice\":{\"id\":\"3\",\"amount\":100}}}}";

    startGroup("event.parse - the hand-written verb");
    publicPem = readFile("publicKey.pem");
    if (publicPem == NULL) {
        check("publicKey.pem fixture", 0, NULL);
        return;
    }
    starkcore_project_new("1", STARKCORE_ENVIRONMENT_SANDBOX, privatePem, &signer);
    starkcore_auth_sign(signer, body, strlen(body), &signature);
    starkcore_user_free(signer);

    starkbank_client_free(client);
    client = newClient(&fake);
    buildKeyReply(keyReply, sizeof(keyReply), publicPem);
    replies(&fake, keyReply, NULL);
    check("a genuine signature verifies and the event envelope is unwrapped",
          starkbank_event_parse(client, body, strlen(body), signature, &event, NULL)
              == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(event), "Event")
          && equalStrings(starkbank_entity_id(event), "5764898044149760"), NULL);
    check("the polymorphic log resolves on a body that never touched the API",
          starkbank_entity_entity(event, STARKBANK_EVENT_LOG, &log) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(log), "InvoiceLog"), NULL);
    starkbank_entity_free(event);
    event = NULL;

    check("a tampered body does not verify and hands back nothing",
          starkbank_event_parse(client, "{\"event\":{\"id\":\"8\"}}",
                                strlen("{\"event\":{\"id\":\"8\"}}"), signature, &event, NULL)
              == STARKCORE_ERROR_SIGNATURE && event == NULL, NULL);
    check("NULL arguments are rejected rather than dereferenced",
          starkbank_event_parse(NULL, body, strlen(body), signature, &event, NULL)
              == STARKCORE_ERROR_ARGUMENT
          && starkbank_event_parse(client, body, strlen(body), signature, NULL, NULL)
              == STARKCORE_ERROR_ARGUMENT, NULL);

    starkcore_free(signature);
    free(publicPem);
    starkbank_client_free(client);
}

/* ============================================================== Balance */

static void testBalance(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_entity *balance = NULL;
    double amount = 0.0;

    startGroup("Balance - the degenerate shape");
    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("balances"), NULL);
    check("get takes the head of the listing, as python's next() does",
          starkbank_balance_get(client, &balance, NULL) == STARKBANK_OK
          && starkbank_entity_amount(balance, STARKBANK_BALANCE_AMOUNT, &amount) == STARKBANK_OK
          && amount == 1234567.0, NULL);
    checkRequests("balance.get", &fake);
    checkHydration("balance.get", 0, balance);
    starkbank_entity_free(balance);
    starkbank_client_free(client);
}

/* ============================================================== Webhook */

/*
 * post_single, the last starkcore_rest_* write shape the bank SDK uses.
 *
 * The case that matters is webhook.create's body. python's rest.post_single
 * sends api_json(entity) as the whole body - no plural list, and no singular
 * wrapper either - and the golden beside this suite carries python's own
 * bytes for it. The assertions below therefore prove two different things:
 * checkRequests proves we send what python sends, and the explicit check
 * proves what that is, so a reader does not have to open slice.json to learn
 * that {"webhooks": [...]} is wrong here.
 */
static void testWebhook(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_entity *webhook = NULL;
    starkbank_entity *created = NULL;
    starkbank_entity *params = NULL;
    starkbank_list *page = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *item = NULL;
    const char *text = NULL;
    int seen = 0;

    startGroup("Webhook, and the post_single shape");
    starkbank_client_free(client);
    client = newClient(&fake);

    replies(&fake, responseBody("webhook"), NULL);
    starkbank_webhook_new(&webhook);
    starkbank_entity_set_string(webhook, STARKBANK_WEBHOOK_URL,
                                "https://webhook.site/60e9c18e-4b5c-4369-bda1-ab5fcd8e1b29");
    starkbank_entity_append_string(webhook, STARKBANK_WEBHOOK_SUBSCRIPTIONS,
                                   STARKBANK_WEBHOOK_SUBSCRIPTION_TRANSFER);
    starkbank_entity_append_string(webhook, STARKBANK_WEBHOOK_SUBSCRIPTIONS,
                                   STARKBANK_WEBHOOK_SUBSCRIPTION_INVOICE);
    check("create takes one entity and returns one entity",
          starkbank_webhook_create(client, webhook, &created, NULL) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(created), "Webhook"), NULL);
    checkRequests("webhook.create", &fake);
    checkHydration("webhook.create", 0, created);
    check("the body is one object, not a list and not an envelope",
          strstr(fake.body[0], "\"webhooks\"") == NULL
          && strstr(fake.body[0], "\"webhook\"") == NULL
          && strstr(fake.body[0], "\"subscriptions\":[\"transfer\",\"invoice\"]") != NULL,
          fake.body[0]);
    check("a created Webhook carries no unknown key: the table is complete",
          starkbank_entity_unknown_count(created) == 0, NULL);
    check("and the entity handed to create is still the caller's",
          starkbank_entity_string(webhook, STARKBANK_WEBHOOK_URL, &text) == STARKBANK_OK, NULL);
    starkbank_entity_free(webhook);
    webhook = NULL;
    starkbank_entity_free(created);
    created = NULL;

    /* A required key missing costs a local error and no round trip, exactly
       as it does for the batch shape. */
    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("webhook"), NULL);
    starkbank_webhook_new(&webhook);
    starkbank_entity_set_string(webhook, STARKBANK_WEBHOOK_URL, "https://example.test/hook");
    check("a Webhook with no subscriptions fails here, with nothing sent",
          starkbank_webhook_create(client, webhook, &created, NULL) == STARKBANK_ERROR_FIELD
          && fake.calls == 0 && created == NULL, NULL);
    starkbank_entity_free(webhook);
    webhook = NULL;

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("webhook"), NULL);
    check("get hydrates one Webhook",
          starkbank_webhook_get(client, "6178044066660352", &webhook, NULL) == STARKBANK_OK,
          NULL);
    checkRequests("webhook.get", &fake);
    checkHydration("webhook.get", 0, webhook);
    check("subscriptions reads as a list of strings",
          starkbank_entity_list_string_at(webhook, STARKBANK_WEBHOOK_SUBSCRIPTIONS, 1, &text)
              == STARKBANK_OK && equalStrings(text, "invoice"), NULL);
    starkbank_entity_free(webhook);
    webhook = NULL;

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("webhooks"), NULL);
    starkbank_webhook_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_webhook_query(client, params, 5, &iter);
    while (starkbank_iter_next(iter, &item, NULL) == STARKBANK_OK && item != NULL) {
        seen++;
    }
    check("query streams the page", seen == 1, NULL);
    checkRequests("webhook.query", &fake);
    starkbank_iter_free(iter);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("webhooks"), NULL);
    starkbank_webhook_params_new(&params);
    starkbank_entity_set_number(params, "limit", 5);
    starkbank_webhook_page(client, params, &page, NULL, NULL);
    checkRequests("webhook.page", &fake);
    checkHydration("webhook.page", 0, starkbank_list_at(page, 0));
    starkbank_list_free(page);
    starkbank_entity_free(params);

    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("webhook"), NULL);
    check("delete returns the deleted subscription",
          starkbank_webhook_delete(client, "6178044066660352", &webhook, NULL) == STARKBANK_OK,
          NULL);
    checkRequests("webhook.delete", &fake);
    starkbank_entity_free(webhook);
    starkbank_client_free(client);
}

/* ======================================================= PaymentPreview */

static starkbank_entity * buildPreview(const char *code)
{
    starkbank_entity *preview = NULL;

    if (starkbank_payment_preview_new(&preview) != STARKBANK_OK) {
        return NULL;
    }
    starkbank_entity_set_string(preview, STARKBANK_PAYMENT_PREVIEW_ID, code);
    starkbank_entity_set_date(preview, STARKBANK_PAYMENT_PREVIEW_SCHEDULED, 2026, 10, 28);
    return preview;
}

/*
 * The polymorphic response, and the reason it is a mixed batch: resolution is
 * per item, off each item's own type, so four single-type calls would pass
 * against an engine that resolved once for the whole reply.
 */
static void testPaymentPreview(void)
{
    static const char *const tags[] = {
        "PaymentPreview.BrcodePreview", "PaymentPreview.BoletoPreview",
        "PaymentPreview.TaxPreview", "PaymentPreview.UtilityPreview"
    };
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_entity *bare = NULL;
    const starkbank_entity *preview = NULL;
    const starkbank_entity *payment = NULL;
    char label[160];
    const char *text = NULL;
    double amount = 0.0;
    int delivered = 0;
    int index;

    startGroup("PaymentPreview, and a table chosen by a sibling field");
    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("previews"), NULL);

    starkbank_list_new(&batch);
    starkbank_list_append(batch, buildPreview(
        "00020126580014br.gov.bcb.pix0136a629532e-7693-4846-852d-1bbff817b5a8"
        "520400005303986540510.005802BR5908T'Challa6009Sao Paulo62090505123456304B14A"));
    starkbank_list_append(batch, buildPreview(
        "34191.09008 63571.277308 71444.640008 5 81960000000062"));
    starkbank_list_append(batch, buildPreview(
        "85660000006 6 67940064007 5 41190025511 7 00010601813 8"));
    starkbank_list_append(batch, buildPreview(
        "82660000002 8 44361143007 7 41190025511 7 00010601813 8"));
    check("create posts the batch and returns one preview per code",
          starkbank_payment_preview_create(client, batch, &created, NULL) == STARKBANK_OK
          && starkbank_list_count(created) == 4, NULL);
    checkRequests("paymentpreview.create", &fake);
    starkbank_list_free(batch);

    for (index = 0; index < starkbank_list_count(created); index++) {
        preview = starkbank_list_at(created, index);
        checkHydration("paymentpreview.create", index, preview);
        snprintf(label, sizeof(label), "item %d hydrates payment as %s", index, tags[index]);
        check(label, starkbank_entity_entity(preview, STARKBANK_PAYMENT_PREVIEW_PAYMENT,
                                             &payment) == STARKBANK_OK
              && equalStrings(starkbank_entity_resource(payment), tags[index]), NULL);
        snprintf(label, sizeof(label), "item %d carries no unknown key, preview included", index);
        check(label, starkbank_entity_unknown_count(preview) == 0, NULL);
    }

    preview = starkbank_list_at(created, 0);
    starkbank_entity_entity(preview, STARKBANK_PAYMENT_PREVIEW_PAYMENT, &payment);
    check("a BrcodePreview is read with the same accessors as any resource",
          starkbank_entity_bool(payment, STARKBANK_BRCODE_PREVIEW_ALLOW_CHANGE, &delivered)
              == STARKBANK_OK && delivered == 1
          && starkbank_entity_amount(payment, STARKBANK_BRCODE_PREVIEW_NOMINAL_AMOUNT, &amount)
              == STARKBANK_OK && amount == 900.0, NULL);
    check("a zero amount is a value and not an absence",
          starkbank_entity_amount(payment, STARKBANK_BRCODE_PREVIEW_DISCOUNT_AMOUNT, &amount)
              == STARKBANK_OK && amount == 0.0, NULL);

    preview = starkbank_list_at(created, 1);
    starkbank_entity_entity(preview, STARKBANK_PAYMENT_PREVIEW_PAYMENT, &payment);
    check("a BoletoPreview's due is the string the API sent, as in python",
          starkbank_entity_string(payment, STARKBANK_BOLETO_PREVIEW_DUE, &text) == STARKBANK_OK
          && equalStrings(text, "2026-10-28"), NULL);
    check("and a table that declared DATE here would be a type the reader cannot use",
          starkbank_entity_datetime(payment, STARKBANK_BOLETO_PREVIEW_DUE, NULL, NULL, NULL,
                                    NULL, NULL, NULL, NULL) == STARKBANK_ERROR_TYPE, NULL);

    preview = starkbank_list_at(created, 3);
    starkbank_entity_entity(preview, STARKBANK_PAYMENT_PREVIEW_PAYMENT, &payment);
    check("a UtilityPreview and a TaxPreview are separate tags, not one shape",
          starkbank_entity_string(payment, STARKBANK_UTILITY_PREVIEW_NAME, &text)
              == STARKBANK_OK && equalStrings(text, "Light Company"), NULL);
    starkbank_list_free(created);
    created = NULL;

    /* The case the design cares about: a type this build predates. Built
       without scheduled, because python's case is, and an optional key left
       unset must not appear on the wire. */
    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake, responseBody("previewUnknown"), NULL);
    starkbank_list_new(&batch);
    starkbank_payment_preview_new(&bare);
    starkbank_entity_set_string(bare, STARKBANK_PAYMENT_PREVIEW_ID, "5656565656565656");
    starkbank_list_append(batch, bare);
    starkbank_payment_preview_create(client, batch, &created, NULL);
    checkRequests("paymentpreview.unknown", &fake);
    preview = starkbank_list_at(created, 0);
    checkHydration("paymentpreview.unknown", 0, preview);
    check("an unknown type leaves payment untagged and permissive",
          starkbank_entity_entity(preview, STARKBANK_PAYMENT_PREVIEW_PAYMENT, &payment)
              == STARKBANK_OK && starkbank_entity_resource(payment) == NULL
          && starkbank_entity_string(payment, "id", &text) == STARKBANK_OK
          && equalStrings(text, "1"), NULL);
    check("and the gap is one unknown, not one per key inside it",
          starkbank_entity_unknown_count(preview) == 1, NULL);
    starkbank_list_free(batch);
    starkbank_list_free(created);
    starkbank_client_free(client);
}

/* ============================================================ negatives */

static void testNegatives(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_entity *invoice = NULL;
    starkbank_entity *transfer = NULL;
    starkbank_entity *params = NULL;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    const char *text = NULL;
    double number = -1.0;
    int year = 0;

    startGroup("negatives on real tables");
    starkbank_client_free(client);
    client = newClient(&fake);
    replies(&fake,
            "{\"invoice\":{\"id\":\"1\",\"taxId\":\"***.345.678-**\","
            "\"created\":\"2026-**-16T12:00:00+00:00\",\"nickname\":\"a key we predate\"}}",
            NULL);
    starkbank_invoice_get(client, "1", &invoice, NULL);

    check("a key on neither the table nor the document is FIELD, not ABSENT",
          starkbank_entity_string(invoice, "nosuchfield", &text) == STARKBANK_ERROR_FIELD
          && text == NULL, NULL);
    check("a key the document has and the table lacks still reads, and is counted",
          starkbank_entity_string(invoice, "nickname", &text) == STARKBANK_OK
          && starkbank_entity_unknown_count(invoice) == 1, NULL);
    check("the wrong accessor for a declared type is TYPE",
          starkbank_entity_amount(invoice, STARKBANK_INVOICE_TAX_ID, &number)
              == STARKBANK_ERROR_TYPE
          && starkbank_entity_string(invoice, STARKBANK_INVOICE_AMOUNT, &text)
              == STARKBANK_ERROR_TYPE, NULL);
    check("a declared key the object does not carry is ABSENT",
          starkbank_entity_string(invoice, STARKBANK_INVOICE_BRCODE, &text)
              == STARKBANK_ERROR_ABSENT, NULL);
    check("a server-redacted datetime is MASKED, not an encoding failure",
          starkbank_entity_datetime(invoice, STARKBANK_INVOICE_CREATED, &year, NULL, NULL,
                                    NULL, NULL, NULL, NULL) == STARKBANK_ERROR_MASKED, NULL);
    check("a masked tax id is still the string the API sent",
          starkbank_entity_string(invoice, STARKBANK_INVOICE_TAX_ID, &text) == STARKBANK_OK
          && equalStrings(text, "***.345.678-**"), NULL);
    check("a write to a key the table lacks is refused, unlike a read",
          starkbank_entity_set_string(invoice, "nickname", "x") == STARKBANK_ERROR_FIELD, NULL);
    starkbank_entity_free(invoice);
    invoice = NULL;

    check("a return-only key is not writable",
          starkbank_invoice_new(&invoice) == STARKBANK_OK
          && starkbank_entity_set_string(invoice, STARKBANK_INVOICE_BRCODE, "x")
              == STARKBANK_ERROR_FIELD, NULL);
    check("Invoice.due takes a date or a datetime; Transfer has no patch at all",
          starkbank_entity_set_date(invoice, STARKBANK_INVOICE_DUE, 2026, 1, 1) == STARKBANK_OK
          && starkbank_entity_set_datetime(invoice, STARKBANK_INVOICE_CREATED,
                                           2026, 1, 1, 0, 0, 0) == STARKBANK_ERROR_FIELD, NULL);
    check("expiration is SECONDS: set_number is the wrong writer",
          starkbank_entity_set_number(invoice, STARKBANK_INVOICE_EXPIRATION, 60)
              == STARKBANK_ERROR_TYPE, NULL);
    check("a fractional amount is refused rather than rounded",
          starkbank_entity_set_amount(invoice, STARKBANK_INVOICE_AMOUNT, 12.34)
              == STARKCORE_ERROR_ARGUMENT, NULL);

    /* REQUIRED is checked before anything is sent: a 400 avoided costs one
       local error, and this Invoice has no taxId and no name. */
    starkbank_list_new(&batch);
    starkbank_list_append(batch, invoice);
    starkbank_client_free(client);
    client = newClient(&fake);
    check("a missing REQUIRED key fails locally, with no request made",
          starkbank_invoice_create(client, batch, &created, NULL) == STARKBANK_ERROR_FIELD
          && fake.calls == 0 && created == NULL, NULL);
    starkbank_list_free(batch);
    invoice = NULL;

    /* A Transfer in an Invoice batch: caught by table pointer, so a resource
       whose registry line someone forgot still creates. */
    starkbank_transfer_new(&transfer);
    starkbank_list_new(&batch);
    starkbank_list_append(batch, transfer);
    check("an entity of another resource is refused before the wire",
          starkbank_invoice_create(client, batch, &created, NULL) == STARKBANK_ERROR_RESOURCE
          && fake.calls == 0, NULL);
    starkbank_list_free(batch);

    check("a Transfer.Rule cannot be appended to an Invoice's rules",
          starkbank_invoice_new(&invoice) == STARKBANK_OK
          && starkbank_transfer_rule_new(&transfer) == STARKBANK_OK
          && starkbank_entity_append_entity(invoice, STARKBANK_INVOICE_RULES, transfer)
              == STARKBANK_ERROR_RESOURCE, NULL);
    starkbank_entity_free(invoice);
    invoice = NULL;

    /* A typed row keeps its type in a params bag: tags is a LIST_STRING and a
       filter on it is a list, not a string. That holds in both builds - the
       loosening is about names the table has not learned, never about types. */
    check("a filter on a typed field is still type-checked",
          starkbank_invoice_params_new(&params) == STARKBANK_OK
          && starkbank_entity_set_string(params, "tags", "war") == STARKBANK_ERROR_TYPE
          && starkbank_entity_append_string(params, "ids", "1") == STARKBANK_OK, NULL);
#ifndef STARKBANK_LOOSE_QUERY
    check("and a return-only field is not a filter",
          starkbank_entity_set_string(params, STARKBANK_INVOICE_PDF, "x")
              == STARKBANK_ERROR_FIELD, NULL);
#endif
    starkbank_entity_free(params);

#ifdef STARKBANK_LOOSE_QUERY
    /* The escape hatch, on a real table: a filter this build's table has not
       learned is accepted. A wrong filter costs a retry where a wrong write
       costs money, which is why this loosening exists and the one on writes
       does not. */
    check("under -DSTARKBANK_LOOSE_QUERY an unknown filter is accepted",
          starkbank_transfer_params_new(&params) == STARKBANK_OK
          && starkbank_entity_set_string(params, "invoiceIds", "1") == STARKBANK_OK, NULL);
#else
    check("a Transfer params bag rejects a filter Transfer does not have",
          starkbank_transfer_params_new(&params) == STARKBANK_OK
          && starkbank_entity_set_string(params, "invoiceIds", "1") == STARKBANK_ERROR_FIELD,
          NULL);
#endif
    starkbank_entity_free(params);
    starkbank_client_free(client);
}

static void testErrorsAndAbi(void)
{
    starkbank_client *client = NULL;
    Fake fake;
    starkbank_entity *invoice = NULL;
    starkbank_errors *errors = NULL;

    startGroup("error mapping and the core-ABI guard");
    starkbank_client_free(client);
    client = newClient(&fake);
    fake.status = 400;
    replies(&fake, "{\"errors\":[{\"code\":\"invalidTaxId\",\"message\":\"bad tax id\"}]}", NULL);
    check("a 400 with errors surfaces them",
          starkbank_invoice_get(client, "1", &invoice, &errors) == STARKCORE_ERROR_INPUT
          && starkbank_errors_count(errors) == 1
          && equalStrings(starkbank_errors_code_at(errors, 0), "invalidTaxId")
          && invoice == NULL, NULL);
    starkbank_errors_free(errors);
    errors = NULL;
    starkbank_client_free(client);

    /* starkcore_abi_version is macro-renamed to a stub in the test-abi build,
       so this suite asserts the agreeing half and tests/abi.c the other. */
    check("this build agrees with the starkcore it linked",
          starkcore_abi_version() == STARKCORE_ABI_VERSION, NULL);
}

/* ============================================================== driver */

int main(void)
{
    char *goldenText;
    size_t length;

    privatePem = readFile("privateKey.pem");
    goldenText = readFile("../reference/slice.json");
    if (privatePem == NULL || goldenText == NULL) {
        printf("missing fixtures; run tests/tools/record_fixtures.py\n");
        return 1;
    }
    length = strlen(goldenText);
    if (starkcore_json_parse(goldenText, length, &goldenDocument) != STARKCORE_OK) {
        printf("tests/reference/slice.json does not parse\n");
        return 1;
    }
    goldenCases = starkcore_json_get(goldenDocument, "cases");
    goldenResponses = starkcore_json_get(goldenDocument, "responses");

    testCanonical();
    testRegistry();
    testInvoice();
    testInvoiceLog();
    testTransfer();
    testTransferLog();
    testBoleto();
    testBoletoLog();
    testBoletoPayment();
    testBoletoPaymentLog();
    testBrcodePayment();
    testBrcodePaymentLog();
    testEvent();
    testEventAttempt();
    testEventParse();
    testWebhook();
    testPaymentPreview();
    testBalance();
    testNegatives();
    testErrorsAndAbi();

    starkcore_json_free(goldenDocument);
    free(goldenText);
    free(privatePem);
    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
