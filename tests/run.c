/*
 * The engine suite. Nothing here touches the network: a fake starkcore
 * transport records what the library would have sent and returns canned
 * bodies, which is how the verb shapes, the envelopes and the error mapping
 * are exercised. core-c itself is never mocked - a request that reaches the
 * fake has been built, cast and signed by the real starkcore.
 *
 * The resources under test are tests/fixtures/testresources.c, authored with
 * the same macros a real resource uses.
 */

#define STARKBANK_TEST_RESOURCE_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "starkbank.h"
#include "internal.h"
#include "testresources.h"

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

/* Computed through volatiles so no compiler folds the division into a
   diagnostic. <math.h> is deliberately not pulled in: the guards under test
   detect NaN by self-comparison, and the suite proves them the same way. */
static double nonFinite(double numerator, double denominator)
{
    volatile double left = numerator;
    volatile double right = denominator;

    return left / right;
}

static char *readFixture(const char *name)
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

/* ------------------------------------------------------- fake transport */

#define FAKE_MAX_CALLS 8

typedef struct {
    int calls;
    int callCap;
    int method;
    char url[FAKE_MAX_CALLS][512];
    char body[FAKE_MAX_CALLS][2048];
    char userAgent[256];
    int status;
    const char *reply;
    const char *replies[FAKE_MAX_CALLS];
    int replyCount;
} FakeTransport;

static int STARKBANK_CALL fakeTransport(void *context, int method, const char *url,
                                        const starkbank_headers *headers, const char *body,
                                        size_t bodyLength, int timeoutSeconds,
                                        starkbank_response **out)
{
    FakeTransport *fake = (FakeTransport *)context;
    const char *reply;
    size_t copyLength;
    int index;

    (void)timeoutSeconds;
    if (fake->calls >= FAKE_MAX_CALLS || (fake->callCap > 0 && fake->calls >= fake->callCap)) {
        return STARKCORE_ERROR_TRANSPORT;
    }
    fake->method = method;
    snprintf(fake->url[fake->calls], sizeof(fake->url[0]), "%s", url);
    copyLength = bodyLength < sizeof(fake->body[0]) - 1 ? bodyLength : sizeof(fake->body[0]) - 1;
    memcpy(fake->body[fake->calls], body, copyLength);
    fake->body[fake->calls][copyLength] = '\0';
    for (index = 0; index < starkbank_headers_count(headers); index++) {
        if (equalStrings(starkbank_headers_name_at(headers, index), "User-Agent")) {
            snprintf(fake->userAgent, sizeof(fake->userAgent), "%s",
                     starkbank_headers_value_at(headers, index));
        }
    }
    reply = fake->reply;
    if (fake->replyCount > 0) {
        reply = fake->calls < fake->replyCount ? fake->replies[fake->calls]
                                               : fake->replies[fake->replyCount - 1];
    }
    fake->calls++;
    return starkbank_response_new(fake->status, (const unsigned char *)reply, strlen(reply),
                                  NULL, out);
}

static char *privatePem;

static starkbank_client *newClient(FakeTransport *fake)
{
    starkbank_user *project = NULL;
    starkbank_client *client = NULL;

    memset(fake, 0, sizeof(*fake));
    fake->status = 200;
    fake->reply = "{}";
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

/* A Widget as the API returns one, with one unknown key the table predates. */
static const char *widgetBody =
    "{\"widget\":{"
    "\"id\":\"5155165527080960\","
    "\"amount\":0,"
    "\"name\":\"Arya Stark\","
    "\"rate\":2.5,"
    "\"expiration\":123456789,"
    "\"count\":7,"
    "\"active\":true,"
    "\"day\":\"2026-10-28\","
    "\"moment\":\"2026-10-28T17:59:26+00:00\","
    "\"due\":\"2026-10-28\","
    "\"tags\":[\"war\",\"supply\"],"
    "\"discounts\":[{\"percentage\":10.0,\"due\":\"2026-10-01\"}],"
    "\"rules\":[{\"key\":\"allowedTaxIds\",\"value\":[\"012.345.678-90\"]}],"
    "\"metadata\":{\"tracker\":\"abc\"},"
    "\"fee\":null,"
    "\"created\":\"2026-09-16T12:00:00+00:00\","
    "\"displayDescription\":\"a key this build predates\""
    "}}";

/* ============================================================ library */

static void testLibrary(void)
{
    startGroup("library");
    check("abi version is the header macro",
          starkbank_abi_version() == STARKBANK_ABI_VERSION, NULL);
    check("version string is the header macro, so a host can #if on it",
          equalStrings(starkbank_version(), STARKBANK_VERSION), NULL);
    check("the linked starkcore reports its own version",
          equalStrings(starkbank_core_version(), starkcore_version()), NULL);
    check("freeing null is safe", (starkbank_free(NULL), 1), NULL);
    check("this tier's codes have their own text",
          !equalStrings(starkbank_strerror(STARKBANK_ERROR_FIELD),
                        starkbank_strerror(STARKBANK_ERROR_TYPE))
          && !equalStrings(starkbank_strerror(STARKBANK_ERROR_ABSENT),
                           starkbank_strerror(STARKBANK_ERROR_MASKED))
          && !equalStrings(starkbank_strerror(STARKBANK_ERROR_RESOURCE),
                           starkbank_strerror(STARKBANK_ERROR_ABI)), NULL);
    check("a starkcore code is forwarded, not shadowed",
          equalStrings(starkbank_strerror(STARKCORE_ERROR_INPUT),
                       starkcore_strerror(STARKCORE_ERROR_INPUT)), NULL);
    check("an unknown code still returns text, never NULL",
          starkbank_strerror(-9999) != NULL, NULL);
    check("every code this tier owns sits below -100, so starkcore cannot collide",
          STARKBANK_ERROR_FIELD < -100 && STARKBANK_ERROR_ABI < -100
          && STARKCORE_ERROR_INTERNAL > -100, NULL);
}

/* =========================================================== registry */

static void testRegistry(void)
{
    const starkbankResource *widget;
    int index;
    int found = 0;

    startGroup("registry and reflection");
    widget = starkbankRegistryFind("Widget");
    check("a resource is found by its wire name", widget != NULL, NULL);
    check("an unknown resource is NULL, never a wild pointer",
          starkbankRegistryFind("Nonesuch") == NULL, NULL);
    check("the params view is not in the registry",
          starkbankRegistryFind("Widget.Params") == NULL, NULL);

    check("resource count is the registry length", starkbank_resource_count() == 6, NULL);
    for (index = 0; index < starkbank_resource_count(); index++) {
        if (equalStrings(starkbank_resource_name_at(index), "Widget")) {
            found = 1;
        }
    }
    check("every resource is reachable by index", found, NULL);
    check("an out-of-range index is NULL", starkbank_resource_name_at(99) == NULL
          && starkbank_resource_name_at(-1) == NULL, NULL);

    check("field count is reflected", starkbank_resource_field_count("Widget") == 17, NULL);
    check("an unknown resource reflects -1, not 0",
          starkbank_resource_field_count("Nonesuch") == -1, NULL);
    check("field names come back in table order",
          equalStrings(starkbank_resource_field_name_at("Widget", 0), "amount")
          && equalStrings(starkbank_resource_field_name_at("Widget", 1), "name"), NULL);
    check("field types are reflected",
          starkbank_resource_field_type_at("Widget", 0) == STARKBANK_FIELD_AMOUNT
          && starkbank_resource_field_type_at("Widget", 8) == STARKBANK_FIELD_DATE_OR_DATETIME,
          NULL);
    check("field flags are reflected",
          starkbank_resource_field_flags_at("Widget", 0)
              == (STARKBANK_FLAG_REQUIRED | STARKBANK_FLAG_CREATE | STARKBANK_FLAG_PATCH)
          && starkbank_resource_field_flags_at("Widget", 15) == 0, NULL);
    check("out of range reflects -1 rather than reading past the table",
          starkbank_resource_field_type_at("Widget", 17) == -1
          && starkbank_resource_field_flags_at("Widget", -1) == -1
          && starkbank_resource_field_name_at("Widget", 17) == NULL, NULL);
}

/* ============================================================= reads */

static void testReads(void)
{
    starkbank_client *client = NULL;
    FakeTransport fake;
    starkbank_entity *widget = NULL;
    const starkbank_entity *nested = NULL;
    const char *text = NULL;
    double number = -1.0;
    int flag = -1;
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, hasTime = -1;
    int size = -1;

    startGroup("reads: every field type, absent, masked, unknown");
    client = newClient(&fake);
    fake.reply = widgetBody;
    check("get hydrates", starkbank_widget_get(client, "1", &widget, NULL) == STARKBANK_OK
          && widget != NULL, NULL);

    check("the entity carries its resource tag",
          equalStrings(starkbank_entity_resource(widget), "Widget"), NULL);
    check("id has a shorthand", equalStrings(starkbank_entity_id(widget), "5155165527080960"), NULL);

    check("STRING", starkbank_entity_string(widget, "name", &text) == STARKBANK_OK
          && equalStrings(text, "Arya Stark"), text);
    check("AMOUNT of zero is a value, not an absence",
          starkbank_entity_amount(widget, "amount", &number) == STARKBANK_OK && number == 0.0, NULL);
    check("RATE", starkbank_entity_number(widget, "rate", &number) == STARKBANK_OK
          && number == 2.5, NULL);
    check("SECONDS", starkbank_entity_number(widget, "expiration", &number) == STARKBANK_OK
          && number == 123456789.0, NULL);
    check("NUMBER", starkbank_entity_number(widget, "count", &number) == STARKBANK_OK
          && number == 7.0, NULL);
    check("BOOL", starkbank_entity_bool(widget, "active", &flag) == STARKBANK_OK && flag == 1, NULL);

    check("DATE has no time",
          starkbank_entity_datetime(widget, "day", &year, &month, &day,
                                    &hour, &minute, &second, &hasTime) == STARKBANK_OK
          && year == 2026 && month == 10 && day == 28 && hasTime == 0, NULL);
    check("DATETIME has one",
          starkbank_entity_datetime(widget, "moment", &year, &month, &day,
                                    &hour, &minute, &second, &hasTime) == STARKBANK_OK
          && hour == 17 && minute == 59 && second == 26 && hasTime == 1, NULL);
    check("DATE_OR_DATETIME reports which it got",
          starkbank_entity_datetime(widget, "due", &year, &month, &day,
                                    NULL, NULL, NULL, &hasTime) == STARKBANK_OK
          && hasTime == 0, NULL);
    check("every datetime out parameter is optional",
          starkbank_entity_datetime(widget, "moment", NULL, NULL, NULL,
                                    NULL, NULL, NULL, NULL) == STARKBANK_OK, NULL);

    check("LIST_STRING size and elements",
          starkbank_entity_list_size(widget, "tags", &size) == STARKBANK_OK && size == 2
          && starkbank_entity_list_string_at(widget, "tags", 1, &text) == STARKBANK_OK
          && equalStrings(text, "supply"), NULL);
    check("an out-of-range list index is ABSENT, never a read past the end",
          starkbank_entity_list_string_at(widget, "tags", 2, &text) == STARKBANK_ERROR_ABSENT
          && starkbank_entity_list_string_at(widget, "tags", -1, &text) == STARKBANK_ERROR_ABSENT,
          NULL);

    check("LIST_OBJECT elements are untagged and permissive",
          starkbank_entity_list_entity_at(widget, "discounts", 0, &nested) == STARKBANK_OK
          && starkbank_entity_resource(nested) == NULL
          && starkbank_entity_number(nested, "percentage", &number) == STARKBANK_OK
          && number == 10.0, NULL);

    check("LIST_RESOURCE elements carry the sub-resource's tag",
          starkbank_entity_list_entity_at(widget, "rules", 0, &nested) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(nested), "Widget.Rule")
          && starkbank_entity_string(nested, "key", &text) == STARKBANK_OK
          && equalStrings(text, "allowedTaxIds"), NULL);
    check("a nested entity is validated against its own table, not its parent's",
          starkbank_entity_string(nested, "name", &text) == STARKBANK_ERROR_FIELD, NULL);

    check("OBJECT is reachable as an untagged entity",
          starkbank_entity_entity(widget, "metadata", &nested) == STARKBANK_OK
          && starkbank_entity_string(nested, "tracker", &text) == STARKBANK_OK
          && equalStrings(text, "abc"), NULL);

    number = 42.0;
    check("a null value is ABSENT and leaves the out parameter untouched",
          starkbank_entity_amount(widget, "fee", &number) == STARKBANK_ERROR_ABSENT
          && number == 42.0, NULL);
    check("a key the object does not carry is ABSENT",
          starkbank_entity_string(widget, "status", &text) == STARKBANK_ERROR_ABSENT, NULL);
    check("entity_has separates carried from absent",
          starkbank_entity_has(widget, "name") != 0
          && starkbank_entity_has(widget, "fee") == 0
          && starkbank_entity_has(widget, "status") == 0, NULL);

    check("a key in neither the table nor the document is FIELD, which is how a typo reads",
          starkbank_entity_string(widget, "nmae", &text) == STARKBANK_ERROR_FIELD, NULL);
    check("a key the document carries and the table does not is readable anyway",
          starkbank_entity_string(widget, "displayDescription", &text) == STARKBANK_OK
          && equalStrings(text, "a key this build predates"), NULL);
    check("and it is counted, so the gap fails CI here instead of a caller's build",
          starkbank_entity_unknown_count(widget) == 1, NULL);
    check("unknown_count is -1 for NULL, never 0",
          starkbank_entity_unknown_count(NULL) == -1, NULL);

    check("the wrong accessor for a field's type is TYPE, not a coerced value",
          starkbank_entity_amount(widget, "name", &number) == STARKBANK_ERROR_TYPE
          && starkbank_entity_string(widget, "amount", &text) == STARKBANK_ERROR_TYPE
          && starkbank_entity_bool(widget, "count", &flag) == STARKBANK_ERROR_TYPE
          && starkbank_entity_datetime(widget, "name", &year, &month, &day,
                                       NULL, NULL, NULL, NULL) == STARKBANK_ERROR_TYPE
          && starkbank_entity_list_size(widget, "name", &size) == STARKBANK_ERROR_TYPE, NULL);
    check("an AMOUNT is not a NUMBER and a RATE is not an AMOUNT: the columns mean something",
          starkbank_entity_number(widget, "amount", &number) == STARKBANK_ERROR_TYPE
          && starkbank_entity_amount(widget, "rate", &number) == STARKBANK_ERROR_TYPE, NULL);

    {
        char *dumped = NULL;
        /* python's api_json drops a None, and every reader here reports a
           null as ABSENT, so the dump has to drop it too or a hydration
           golden reads as a diff for a field neither side carries. */
        check("dump drops a null, because a null is an absence everywhere else",
              starkbank_entity_dump(widget, &dumped, NULL) == STARKBANK_OK
              && strstr(dumped, "\"fee\"") == NULL
              && strstr(dumped, "\"amount\":0") != NULL, dumped);
        starkbank_free(dumped);
    }

    check("the underlying document is reachable for anything the tables miss",
          starkbank_entity_json(widget) != NULL
          && starkcore_json_get(starkbank_entity_json(widget), "name") != NULL, NULL);
    check("NULL arguments are rejected, never dereferenced",
          starkbank_entity_string(NULL, "name", &text) == STARKCORE_ERROR_ARGUMENT
          && starkbank_entity_string(widget, NULL, &text) == STARKCORE_ERROR_ARGUMENT
          && starkbank_entity_string(widget, "name", NULL) == STARKCORE_ERROR_ARGUMENT
          && starkbank_entity_resource(NULL) == NULL
          && starkbank_entity_id(NULL) == NULL
          && starkbank_entity_json(NULL) == NULL
          && starkbank_entity_has(NULL, "name") == 0, NULL);

    starkbank_entity_free(widget);
    starkbank_client_free(client);
}

static void testMasked(void)
{
    starkbank_client *client = NULL;
    FakeTransport fake;
    starkbank_entity *widget = NULL;
    const char *text = NULL;
    int year = 0;

    startGroup("masked values");
    client = newClient(&fake);
    fake.reply = "{\"widget\":{\"id\":\"1\",\"name\":\"***.345.678-**\",\"created\":\"2026-**-16T12:00:00+00:00\"}}";
    starkbank_widget_get(client, "1", &widget, NULL);
    check("a redacted datetime is MASKED, not an encoding failure",
          starkbank_entity_datetime(widget, "created", &year, NULL, NULL,
                                    NULL, NULL, NULL, NULL) == STARKBANK_ERROR_MASKED, NULL);
    check("a redacted string comes back verbatim: the mask is the value the API sent",
          starkbank_entity_string(widget, "name", &text) == STARKBANK_OK
          && equalStrings(text, "***.345.678-**"), text);
    starkbank_entity_free(widget);
    starkbank_client_free(client);
}

/* ============================================================ writes */

static void testWrites(void)
{
    starkbank_entity *widget = NULL;
    starkbank_entity *rule = NULL;
    starkbank_entity *object = NULL;
    starkbank_entity *params = NULL;
    const starkbank_entity *nested = NULL;
    const char *text = NULL;
    double number = 0.0;
    char *dump = NULL;
    size_t dumpLength = 0;

    startGroup("writes: strict against the table");
    check("new gives an empty tagged entity",
          starkbank_widget_new(&widget) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(widget), "Widget")
          && starkbank_entity_id(widget) == NULL, NULL);

    check("a CREATE field accepts its own writer",
          starkbank_entity_set_amount(widget, "amount", 400000) == STARKBANK_OK
          && starkbank_entity_set_string(widget, "name", "Arya Stark") == STARKBANK_OK
          && starkbank_entity_set_number(widget, "rate", 2.5) == STARKBANK_OK
          && starkbank_entity_set_seconds(widget, "expiration", 3600) == STARKBANK_OK
          && starkbank_entity_set_bool(widget, "active", 1) == STARKBANK_OK
          && starkbank_entity_set_date(widget, "day", 2026, 10, 28) == STARKBANK_OK
          && starkbank_entity_set_datetime(widget, "moment", 2026, 10, 28, 17, 59, 26) == STARKBANK_OK,
          NULL);
    check("a write reads back through the accessor",
          starkbank_entity_amount(widget, "amount", &number) == STARKBANK_OK && number == 400000.0,
          NULL);

    check("a key the table does not have is FIELD, never a silently dropped payload key",
          starkbank_entity_set_string(widget, "nmae", "x") == STARKBANK_ERROR_FIELD, NULL);
    check("a return-only field is FIELD too",
          starkbank_entity_set_string(widget, "id", "1") == STARKBANK_ERROR_FIELD
          && starkbank_entity_set_amount(widget, "fee", 1) == STARKBANK_ERROR_FIELD, NULL);
    check("a patch-only field is not writable on a create entity",
          starkbank_entity_set_string(widget, "status", "paid") == STARKBANK_ERROR_FIELD, NULL);
    check("the wrong writer for a type is TYPE",
          starkbank_entity_set_string(widget, "amount", "400000") == STARKBANK_ERROR_TYPE
          && starkbank_entity_set_amount(widget, "name", 1) == STARKBANK_ERROR_TYPE
          && starkbank_entity_set_number(widget, "amount", 1.0) == STARKBANK_ERROR_TYPE, NULL);

    check("set_date on a DATETIME field is TYPE: the choice of writer is the wire semantic",
          starkbank_entity_set_date(widget, "moment", 2026, 10, 28) == STARKBANK_ERROR_TYPE, NULL);
    check("set_datetime on a DATE field likewise",
          starkbank_entity_set_datetime(widget, "day", 2026, 10, 28, 1, 2, 3) == STARKBANK_ERROR_TYPE,
          NULL);
    check("DATE_OR_DATETIME takes either, and that is the whole distinction",
          starkbank_entity_set_date(widget, "due", 2026, 10, 28) == STARKBANK_OK
          && starkbank_entity_string(widget, "due", &text) == STARKBANK_ERROR_TYPE, NULL);
    check("a date is written in the wire form, never through strftime",
          starkcore_json_string(starkcore_json_get(starkbank_entity_json(widget), "due")) != NULL
          && equalStrings(starkcore_json_string(
                 starkcore_json_get(starkbank_entity_json(widget), "due")), "2026-10-28"), NULL);
    check("set_datetime writes the literal +00:00 form",
          equalStrings(starkcore_json_string(
              starkcore_json_get(starkbank_entity_json(widget), "moment")),
              "2026-10-28T17:59:26+00:00"), NULL);
    check("a datetime replaces a date on the same DATE_OR_DATETIME field",
          starkbank_entity_set_datetime(widget, "due", 2026, 10, 28, 1, 2, 3) == STARKBANK_OK
          && equalStrings(starkcore_json_string(
                 starkcore_json_get(starkbank_entity_json(widget), "due")),
                 "2026-10-28T01:02:03+00:00"), NULL);

    check("a fractional amount is refused rather than rounded",
          starkbank_entity_set_amount(widget, "amount", 400000.5) == STARKCORE_ERROR_ARGUMENT, NULL);
    check("and the field keeps its old value",
          starkbank_entity_amount(widget, "amount", &number) == STARKBANK_OK
          && number == 400000.0, NULL);
    /* A non-finite double is undefined behaviour to cast to long long, and
       cJSON prints one as JSON null - so an unguarded money write is either a
       UBSan abort or a silently dropped amount. Both are refusals here. */
    check("a NaN amount is refused",
          starkbank_entity_set_amount(widget, "amount", nonFinite(0.0, 0.0))
          == STARKCORE_ERROR_ARGUMENT, NULL);
    check("an infinite amount is refused, in both signs",
          starkbank_entity_set_amount(widget, "amount", nonFinite(1.0, 0.0))
          == STARKCORE_ERROR_ARGUMENT
          && starkbank_entity_set_amount(widget, "amount", nonFinite(-1.0, 0.0))
          == STARKCORE_ERROR_ARGUMENT, NULL);
    check("and a refused non-finite amount leaves the previous value intact",
          starkbank_entity_amount(widget, "amount", &number) == STARKBANK_OK
          && number == 400000.0, NULL);

    check("set_number takes a finite rate",
          starkbank_entity_set_number(widget, "rate", 2.5) == STARKBANK_OK, NULL);
    check("a NaN rate is refused rather than written as a JSON null",
          starkbank_entity_set_number(widget, "rate", nonFinite(0.0, 0.0))
          == STARKCORE_ERROR_ARGUMENT, NULL);
    check("an infinite rate is refused, in both signs",
          starkbank_entity_set_number(widget, "rate", nonFinite(1.0, 0.0))
          == STARKCORE_ERROR_ARGUMENT
          && starkbank_entity_set_number(widget, "rate", nonFinite(-1.0, 0.0))
          == STARKCORE_ERROR_ARGUMENT, NULL);
    check("a rate beyond 2^53 is refused: it is not a rate",
          starkbank_entity_set_number(widget, "rate", 1.0e300) == STARKCORE_ERROR_ARGUMENT
          && starkbank_entity_set_number(widget, "rate", -1.0e300)
          == STARKCORE_ERROR_ARGUMENT, NULL);
    check("and a refused rate leaves the previous value intact",
          starkbank_entity_number(widget, "rate", &number) == STARKBANK_OK
          && number == 2.5, NULL);

    check("a bad date is refused at the writer",
          starkbank_entity_set_date(widget, "day", 2026, 13, 1) == STARKCORE_ERROR_ARGUMENT, NULL);

    check("append_string builds a LIST_STRING",
          starkbank_entity_append_string(widget, "tags", "war") == STARKBANK_OK
          && starkbank_entity_append_string(widget, "tags", "supply") == STARKBANK_OK, NULL);
    check("append_string refuses a field that is not a LIST_STRING",
          starkbank_entity_append_string(widget, "name", "x") == STARKBANK_ERROR_TYPE, NULL);

    starkbank_widget_rule_new(&rule);
    starkbank_entity_set_string(rule, "key", "allowedTaxIds");
    starkbank_entity_append_string(rule, "value", "012.345.678-90");
    check("append_entity takes a tagged sub-resource",
          starkbank_entity_append_entity(widget, "rules", rule) == STARKBANK_OK, NULL);
    check("and the appended entity is readable in place",
          starkbank_entity_list_entity_at(widget, "rules", 0, &nested) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(nested), "Widget.Rule"), NULL);

    starkbank_object_new(&object);
    starkbank_entity_set_number(object, "percentage", 10.0);
    check("a free-form map goes into a LIST_OBJECT",
          starkbank_entity_append_entity(widget, "discounts", object) == STARKBANK_OK, NULL);

    starkbank_widget_rule_new(&rule);
    check("the wrong sub-resource is RESOURCE, and is still consumed so nothing leaks",
          starkbank_entity_append_entity(widget, "discounts", rule) == STARKBANK_OK, NULL);
    starkbank_object_new(&object);
    check("an untagged object into a LIST_RESOURCE is refused",
          starkbank_entity_append_entity(widget, "rules", object) == STARKBANK_ERROR_RESOURCE, NULL);

    check("set_json_raw is the documented way past the table",
          starkbank_entity_set_json_raw(widget, "somethingNew", "{\"a\":1}") == STARKBANK_OK
          && starkcore_json_get(starkbank_entity_json(widget), "somethingNew") != NULL, NULL);
    check("and malformed json is still refused",
          starkbank_entity_set_json_raw(widget, "broken", "{oops") == STARKCORE_ERROR_ENCODING, NULL);

    check("dump is sorted and table-filtered, so a golden compares byte for byte",
          starkbank_entity_dump(widget, &dump, &dumpLength) == STARKBANK_OK
          && strstr(dump, "\"active\":true,\"amount\":400000") != NULL
          && strstr(dump, "somethingNew") == NULL, dump);
    starkbank_free(dump);

    startGroup("writes: the params bag");
    starkbank_widget_params_new(&params);
    check("the bag is tagged as a view of its resource",
          equalStrings(starkbank_entity_resource(params), "Widget.Params"), NULL);
    check("a typed field keeps its type in a filter",
          starkbank_entity_set_string(params, "status", "paid") == STARKBANK_OK
          && starkbank_entity_set_amount(params, "amount", 100) == STARKBANK_OK
          && starkbank_entity_set_string(params, "amount", "100") == STARKBANK_ERROR_TYPE, NULL);
    check("a query key with no wire type of its own takes any writer",
          starkbank_entity_set_date(params, "after", 2026, 9, 1) == STARKBANK_OK
          && starkbank_entity_append_string(params, "ids", "1") == STARKBANK_OK, NULL);
    check("pagination keys are accepted on every bag, so no table can forget them",
          starkbank_entity_set_number(params, "limit", 10) == STARKBANK_OK
          && starkbank_entity_set_string(params, "cursor", "abc") == STARKBANK_OK, NULL);
#ifdef STARKBANK_LOOSE_QUERY
    check("a filter this build's table has not learned is accepted in a loose build",
          starkbank_entity_set_string(params, "nmae", "x") == STARKBANK_OK, NULL);
    {
        starkbank_entity *strict = NULL;
        starkbank_widget_new(&strict);
        check("and writes stay strict even so: the loosening is queries only",
              starkbank_entity_set_string(strict, "nmae", "x") == STARKBANK_ERROR_FIELD, NULL);
        starkbank_entity_free(strict);
    }
#else
    check("a filter in neither the table nor the query list is FIELD",
          starkbank_entity_set_string(params, "nmae", "x") == STARKBANK_ERROR_FIELD, NULL);
#endif
#ifndef STARKBANK_LOOSE_QUERY
    check("a return-only field is not a filter unless the query list names it",
          starkbank_entity_set_string(params, "id", "1") == STARKBANK_ERROR_FIELD, NULL);
#endif
    starkbank_entity_free(params);

    starkbank_entity_free(widget);
}

static void testBorrowedIsReadOnly(void)
{
    starkbank_client *client = NULL;
    FakeTransport fake;
    starkbank_entity *widget = NULL;
    const starkbank_entity *nested = NULL;

    startGroup("a borrowed entity is read-only");
    client = newClient(&fake);
    fake.reply = widgetBody;
    starkbank_widget_get(client, "1", &widget, NULL);
    starkbank_entity_list_entity_at(widget, "rules", 0, &nested);
    /* const is the real guard for C; an FFI host has no const, so the engine
       refuses rather than mutating a document the parent owns. */
    check("writing through a nested handle is refused",
          starkbank_entity_set_string((starkbank_entity *)(size_t)nested, "key", "x")
              == STARKCORE_ERROR_ARGUMENT, NULL);
    starkbank_entity_free(widget);
    starkbank_client_free(client);
}

/* ============================================================== list */

static void testList(void)
{
    starkbank_list *list = NULL;
    starkbank_entity *widget = NULL;
    starkbank_entity *clone = NULL;
    const char *text = NULL;

    startGroup("list ownership");
    check("a new list is empty", starkbank_list_new(&list) == STARKBANK_OK
          && starkbank_list_count(list) == 0, NULL);
    check("count is -1 for NULL, so an error cannot read as an empty page",
          starkbank_list_count(NULL) == -1, NULL);

    starkbank_widget_new(&widget);
    starkbank_entity_set_string(widget, "name", "Arya Stark");
    check("append takes ownership", starkbank_list_append(list, widget) == STARKBANK_OK
          && starkbank_list_count(list) == 1, NULL);
    check("the entity is reachable and borrowed",
          starkbank_list_at(list, 0) == widget
          && starkbank_entity_string(starkbank_list_at(list, 0), "name", &text) == STARKBANK_OK,
          NULL);
    check("an out-of-range index is NULL", starkbank_list_at(list, 1) == NULL
          && starkbank_list_at(list, -1) == NULL && starkbank_list_at(NULL, 0) == NULL, NULL);

    check("clone survives the list it came from",
          starkbank_entity_clone(starkbank_list_at(list, 0), &clone) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(clone), "Widget"), NULL);
    starkbank_list_free(list);
    check("and the clone is still readable afterwards",
          starkbank_entity_string(clone, "name", &text) == STARKBANK_OK
          && equalStrings(text, "Arya Stark"), NULL);
    check("a clone is independent: writing it does not touch the original",
          starkbank_entity_set_string(clone, "name", "Sansa") == STARKBANK_OK, NULL);
    starkbank_entity_free(clone);

    check("freeing NULL is safe for every handle",
          (starkbank_list_free(NULL), starkbank_entity_free(NULL), starkbank_iter_free(NULL),
           starkbank_client_free(NULL), starkbank_user_free(NULL), 1), NULL);
    {
        /* An FFI host has no type system and hands back whatever integer it
           kept. Every handle carries a tag word, so a pointer that is not one
           of ours is rejected rather than dereferenced. This is NOT the
           double-free case: reading a block already returned to the allocator
           is undefined however it is tagged, and the header says so. */
        char notAHandle[64];
        memset(notAHandle, 0, sizeof(notAHandle));
        check("a pointer that is not one of our handles is rejected, not dereferenced",
              (starkbank_entity_free((starkbank_entity *)notAHandle),
               starkbank_list_free((starkbank_list *)notAHandle),
               starkbank_iter_free((starkbank_iter *)notAHandle),
               starkbank_client_free((starkbank_client *)notAHandle),
               starkbank_user_free((starkbank_user *)notAHandle),
               starkbank_entity_resource((starkbank_entity *)notAHandle) == NULL
               && starkbank_list_count((starkbank_list *)notAHandle) == -1), NULL);
    }
    check("appending to NULL, or NULL to a list, is rejected",
          starkbank_list_append(NULL, NULL) == STARKCORE_ERROR_ARGUMENT, NULL);
    {
        /* Ownership rule 3 is unconditional: append consumes the entity on
           every failure path, so a caller who follows the header never has a
           handle left to free and never leaks one. `make leaks` is the other
           half of this assertion. */
        starkbank_entity *orphan = NULL;
        char notAList[64];

        memset(notAList, 0, sizeof(notAList));
        starkbank_widget_new(&orphan);
        check("append to a NULL list still consumes the entity",
              starkbank_list_append(NULL, orphan) == STARKCORE_ERROR_ARGUMENT, NULL);
        starkbank_widget_new(&orphan);
        check("append to a pointer that is not a list still consumes the entity",
              starkbank_list_append((starkbank_list *)notAList, orphan)
              == STARKCORE_ERROR_ARGUMENT, NULL);
    }
}

/* ============================================================= verbs */

static void testVerbs(void)
{
    starkbank_client *client = NULL;
    FakeTransport fake;
    starkbank_entity *widget = NULL;
    starkbank_entity *params = NULL;
    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_list *page = NULL;
    char *cursor = NULL;
    unsigned char *blob = NULL;
    size_t blobLength = 0;
    starkbank_errors *errors = NULL;
    int status;

    startGroup("verbs");
    client = newClient(&fake);

    fake.reply = widgetBody;
    check("get_id asks the endpoint starkcore derives, never one spelled here",
          starkbank_widget_get(client, "5155165527080960", &widget, &errors) == STARKBANK_OK
          && strstr(fake.url[0], "/v2/widget/5155165527080960") != NULL, fake.url[0]);
    check("the errors out parameter is nulled on success", errors == NULL, NULL);
    check("the User-Agent carries this library's version",
          strstr(fake.userAgent, STARKBANK_VERSION) != NULL, fake.userAgent);
    starkbank_entity_free(widget);

    /* create */
    memset(&fake, 0, sizeof(fake));
    fake.status = 200;
    fake.reply = "{\"widgets\":[{\"id\":\"1\",\"name\":\"Arya Stark\",\"amount\":100}]}";
    starkbank_list_new(&batch);
    starkbank_widget_new(&widget);
    starkbank_entity_set_amount(widget, "amount", 100);
    starkbank_entity_set_string(widget, "name", "Arya Stark");
    starkbank_list_append(batch, widget);
    check("create posts the plural envelope and returns a list",
          starkbank_widget_create(client, batch, &created, &errors) == STARKBANK_OK
          && starkbank_list_count(created) == 1
          && strstr(fake.body[0], "\"widgets\"") != NULL
          && strstr(fake.body[0], "\"name\":\"Arya Stark\"") != NULL, fake.body[0]);
    check("the created entities carry the resource tag",
          equalStrings(starkbank_entity_resource(starkbank_list_at(created, 0)), "Widget"), NULL);
    starkbank_list_free(created);
    created = NULL;
    starkbank_list_free(batch);

    /* a required field missing must cost no round trip */
    memset(&fake, 0, sizeof(fake));
    fake.status = 200;
    fake.reply = "{\"widgets\":[]}";
    starkbank_list_new(&batch);
    starkbank_widget_new(&widget);
    starkbank_entity_set_amount(widget, "amount", 100);
    starkbank_list_append(batch, widget);
    status = starkbank_widget_create(client, batch, &created, &errors);
    check("a missing REQUIRED field fails locally and sends nothing",
          status == STARKBANK_ERROR_FIELD && fake.calls == 0 && created == NULL, NULL);
    starkbank_list_free(batch);

    /* the wrong resource in a batch */
    starkbank_list_new(&batch);
    starkbank_widget_rule_new(&widget);
    starkbank_entity_set_string(widget, "key", "k");
    starkbank_entity_append_string(widget, "value", "v");
    starkbank_list_append(batch, widget);
    check("an entity of another resource in a create batch is RESOURCE",
          starkbank_widget_create(client, batch, &created, NULL) == STARKBANK_ERROR_RESOURCE,
          NULL);
    starkbank_list_free(batch);

    /* page */
    memset(&fake, 0, sizeof(fake));
    fake.status = 200;
    fake.reply = "{\"widgets\":[{\"id\":\"1\"},{\"id\":\"2\"}],\"cursor\":\"next\"}";
    starkbank_widget_params_new(&params);
    starkbank_entity_set_string(params, "status", "paid");
    check("page returns the list and the cursor python returns",
          starkbank_widget_page(client, params, &page, &cursor, &errors) == STARKBANK_OK
          && starkbank_list_count(page) == 2 && equalStrings(cursor, "next")
          && strstr(fake.url[0], "status=paid") != NULL, fake.url[0]);
    starkbank_free(cursor);
    cursor = NULL;
    starkbank_list_free(page);
    page = NULL;

    fake.calls = 0;
    fake.reply = "{\"widgets\":[]}";
    check("the last page reports no cursor",
          starkbank_widget_page(client, NULL, &page, &cursor, NULL) == STARKBANK_OK
          && cursor == NULL && starkbank_list_count(page) == 0, NULL);
    starkbank_list_free(page);
    page = NULL;
    starkbank_entity_free(params);

    /* update */
    memset(&fake, 0, sizeof(fake));
    fake.status = 200;
    fake.reply = "{\"widget\":{\"id\":\"1\",\"status\":\"canceled\"}}";
    starkbank_widget_params_new(&params);
    starkbank_entity_set_string(params, "status", "canceled");
    check("update patches only what the table marks patchable",
          starkbank_widget_update(client, "1", params, &widget, &errors) == STARKBANK_OK
          && strstr(fake.body[0], "\"status\":\"canceled\"") != NULL, fake.body[0]);
    starkbank_entity_free(widget);
    starkbank_entity_free(params);

    memset(&fake, 0, sizeof(fake));
    fake.status = 200;
    fake.reply = "{}";
    starkbank_widget_params_new(&params);
    starkbank_entity_set_number(params, "limit", 10);
    check("a query-only key in a patch is FIELD, and nothing is sent",
          starkbank_widget_update(client, "1", params, &widget, NULL) == STARKBANK_ERROR_FIELD
          && fake.calls == 0, NULL);
    starkbank_entity_free(params);

    /* delete */
    memset(&fake, 0, sizeof(fake));
    fake.status = 200;
    fake.reply = "{\"widget\":{\"id\":\"1\",\"status\":\"canceled\"}}";
    check("delete returns the cancelled entity",
          starkbank_widget_delete(client, "1", &widget, NULL) == STARKBANK_OK
          && equalStrings(starkbank_entity_id(widget), "1"), NULL);
    starkbank_entity_free(widget);

    /* content */
    memset(&fake, 0, sizeof(fake));
    fake.status = 200;
    fake.reply = "%PDF-1.4 fake";
    check("pdf returns raw bytes from the content endpoint",
          starkbank_widget_pdf(client, "1", &blob, &blobLength, NULL) == STARKBANK_OK
          && blobLength == strlen("%PDF-1.4 fake")
          && memcmp(blob, "%PDF-", 5) == 0
          && strstr(fake.url[0], "/v2/widget/1/pdf") != NULL, fake.url[0]);
    starkbank_free(blob);
    blob = NULL;

    memset(&fake, 0, sizeof(fake));
    fake.status = 200;
    fake.reply = "PNG";
    check("an int content parameter is sent when given",
          starkbank_widget_qrcode(client, "1", 12, &blob, &blobLength, NULL) == STARKBANK_OK
          && strstr(fake.url[0], "size=12") != NULL, fake.url[0]);
    starkbank_free(blob);
    blob = NULL;

    fake.calls = 0;
    check("zero sends no parameter at all, so the API applies its own default",
          starkbank_widget_qrcode(client, "1", 0, &blob, &blobLength, NULL) == STARKBANK_OK
          && strstr(fake.url[0], "size=") == NULL, fake.url[0]);
    starkbank_free(blob);
    blob = NULL;

    fake.calls = 0;
    check("out of range is refused here rather than by the API",
          starkbank_widget_qrcode(client, "1", 51, &blob, &blobLength, NULL)
              == STARKCORE_ERROR_ARGUMENT
          && starkbank_widget_qrcode(client, "1", -1, &blob, &blobLength, NULL)
              == STARKCORE_ERROR_ARGUMENT
          && fake.calls == 0 && blob == NULL, NULL);

    /* sub-resource */
    memset(&fake, 0, sizeof(fake));
    fake.status = 200;
    fake.reply = "{\"payment\":{\"amount\":100,\"name\":\"Arya Stark\"}}";
    check("a sub-resource is unwrapped by its own key and carries its own tag",
          starkbank_widget_payment(client, "1", &widget, NULL) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(widget), "Widget.Payment")
          && strstr(fake.url[0], "/v2/widget/1/payment") != NULL, fake.url[0]);
    starkbank_entity_free(widget);

    /* the degenerate get with no id */
    memset(&fake, 0, sizeof(fake));
    fake.status = 200;
    fake.reply = "{\"ledgers\":[{\"id\":\"1\",\"amount\":500}]}";
    check("a resource with no id reads the listing endpoint and takes its head",
          starkbank_ledger_get(client, &widget, NULL) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(widget), "Ledger")
          && equalStrings(starkbank_entity_id(widget), "1"), NULL);
    starkbank_entity_free(widget);

    check("every verb rejects a NULL client rather than dereferencing it",
          starkbank_widget_get(NULL, "1", &widget, NULL) == STARKCORE_ERROR_ARGUMENT
          && starkbank_widget_get(client, NULL, &widget, NULL) == STARKCORE_ERROR_ARGUMENT
          && starkbank_widget_get(client, "1", NULL, NULL) == STARKCORE_ERROR_ARGUMENT, NULL);

    starkbank_client_free(client);
}

/* ============================================================== iter */

static void testIter(void)
{
    starkbank_client *client = NULL;
    FakeTransport fake;
    starkbank_iter *iter = NULL;
    const starkbank_entity *found = NULL;
    starkbank_entity *kept = NULL;
    int count = 0;
    int status;

    startGroup("iterator");
    client = newClient(&fake);
    fake.replies[0] = "{\"widgets\":[{\"id\":\"1\"},{\"id\":\"2\"}],\"cursor\":\"c1\"}";
    fake.replies[1] = "{\"widgets\":[{\"id\":\"3\"}],\"cursor\":\"\"}";
    fake.replyCount = 2;
    check("query builds an iterator without sending anything",
          starkbank_widget_query(client, NULL, 0, &iter) == STARKBANK_OK && fake.calls == 0, NULL);

    while ((status = starkbank_iter_next(iter, &found, NULL)) == STARKBANK_OK && found != NULL) {
        if (count == 0) {
            starkbank_entity_clone(found, &kept);
        }
        count++;
    }
    check("it walks every page and stops on an empty cursor", status == STARKBANK_OK && count == 3,
          NULL);
    check("two pages were fetched", fake.calls == 2, NULL);
    check("the yielded entity carried the resource tag", kept != NULL
          && equalStrings(starkbank_entity_resource(kept), "Widget"), NULL);
    check("a clone taken mid-walk is still valid after it",
          equalStrings(starkbank_entity_id(kept), "1"), NULL);
    starkbank_entity_free(kept);
    starkbank_iter_free(iter);
    iter = NULL;

    memset(&fake, 0, sizeof(fake));
    fake.status = 400;
    fake.reply = "{\"errors\":[{\"code\":\"invalidWidget\",\"message\":\"no\"}]}";
    {
        starkbank_errors *errors = NULL;
        starkbank_widget_query(client, NULL, 0, &iter);
        status = starkbank_iter_next(iter, &found, &errors);
        check("an API failure surfaces from next, not from query",
              status == STARKCORE_ERROR_INPUT && found == NULL
              && starkbank_errors_count(errors) == 1
              && equalStrings(starkbank_errors_code_at(errors, 0), "invalidWidget"), NULL);
        starkbank_errors_free(errors);
    }
    starkbank_iter_free(iter);

    check("next on NULL is rejected",
          starkbank_iter_next(NULL, &found, NULL) == STARKCORE_ERROR_ARGUMENT
          && starkbank_iter_cursor(NULL) == NULL, NULL);
    starkbank_client_free(client);
}

/* ==================================================== nested and errors */

static void testNested(void)
{
    starkbank_client *client = NULL;
    FakeTransport fake;
    starkbank_entity *log = NULL;
    starkbank_entity *gadget = NULL;
    const starkbank_entity *nested = NULL;
    const starkbank_entity *deeper = NULL;
    const char *text = NULL;

    startGroup("nested and polymorphic resources");
    client = newClient(&fake);
    fake.reply = "{\"log\":{\"id\":\"9\",\"type\":\"created\",\"errors\":[],"
                 "\"widget\":{\"id\":\"1\",\"name\":\"Arya Stark\"}}}";
    check("a log endpoint is derived by starkcore, not spelled here",
          starkbank_widget_log_get(client, "9", &log, NULL) == STARKBANK_OK
          && strstr(fake.url[0], "/v2/widget/log/9") != NULL, fake.url[0]);
    check("a RESOURCE field hydrates as a fully tagged entity",
          starkbank_entity_entity(log, "widget", &nested) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(nested), "Widget")
          && starkbank_entity_string(nested, "name", &text) == STARKBANK_OK, NULL);
    check("the nested entity is counted against its own table",
          starkbank_entity_unknown_count(log) == 0, NULL);
    starkbank_entity_free(log);

    memset(&fake, 0, sizeof(fake));
    fake.status = 200;
    fake.reply = "{\"gadget\":{\"id\":\"7\",\"subscription\":\"widget\","
                 "\"log\":{\"id\":\"9\",\"type\":\"created\","
                 "\"widget\":{\"id\":\"1\",\"name\":\"Arya Stark\"}}}}";
    starkbank_gadget_get(client, "7", &gadget, NULL);
    check("a polymorphic field is tagged by the document, not by the table",
          starkbank_entity_entity(gadget, "log", &nested) == STARKBANK_OK
          && equalStrings(starkbank_entity_resource(nested), "WidgetLog"), NULL);
    check("and reading through two levels needs no special case",
          starkbank_entity_entity(nested, "widget", &deeper) == STARKBANK_OK
          && starkbank_entity_string(deeper, "name", &text) == STARKBANK_OK
          && equalStrings(text, "Arya Stark"), NULL);
    check("nothing unknown in a fully modelled tree",
          starkbank_entity_unknown_count(gadget) == 0, NULL);
    starkbank_entity_free(gadget);

    memset(&fake, 0, sizeof(fake));
    fake.status = 200;
    fake.reply = "{\"gadget\":{\"id\":\"7\",\"subscription\":\"sphinx\","
                 "\"log\":{\"id\":\"9\",\"riddle\":\"unmapped\"}}}";
    starkbank_gadget_get(client, "7", &gadget, NULL);
    check("a subscription this build predates still hydrates, untagged and permissive",
          starkbank_entity_entity(gadget, "log", &nested) == STARKBANK_OK
          && starkbank_entity_resource(nested) == NULL
          && starkbank_entity_string(nested, "riddle", &text) == STARKBANK_OK, NULL);
    check("and the gap is counted, so it fails CI here rather than in a caller",
          starkbank_entity_unknown_count(gadget) == 1, NULL);
    starkbank_entity_free(gadget);
    starkbank_client_free(client);
}

static void testErrors(void)
{
    starkbank_client *client = NULL;
    FakeTransport fake;
    starkbank_entity *widget = NULL;
    starkbank_errors *errors = NULL;

    startGroup("error mapping");
    client = newClient(&fake);

    fake.status = 400;
    fake.reply = "{\"errors\":[{\"code\":\"invalidAmount\",\"message\":\"too much\"}]}";
    check("a 400 surfaces the API's coded errors",
          starkbank_widget_get(client, "1", &widget, &errors) == STARKCORE_ERROR_INPUT
          && widget == NULL && starkbank_errors_count(errors) == 1
          && equalStrings(starkbank_errors_code_at(errors, 0), "invalidAmount")
          && equalStrings(starkbank_errors_message_at(errors, 0), "too much"), NULL);
    starkbank_errors_free(errors);
    errors = NULL;

    fake.calls = 0;
    fake.status = 400;
    fake.reply = "not json at all";
    check("a 400 with a non-JSON body is ENCODING, exactly as python",
          starkbank_widget_get(client, "1", &widget, &errors) == STARKCORE_ERROR_ENCODING
          && widget == NULL, NULL);

    fake.calls = 0;
    fake.status = 400;
    fake.reply = "{\"message\":\"no errors key\"}";
    check("a 400 with no errors key is MISSING_KEY, as python's KeyError",
          starkbank_widget_get(client, "1", &widget, &errors) == STARKCORE_ERROR_MISSING_KEY, NULL);

    fake.calls = 0;
    fake.status = 500;
    fake.reply = "{}";
    check("a 500 is INTERNAL_SERVER",
          starkbank_widget_get(client, "1", &widget, &errors) == STARKCORE_ERROR_INTERNAL_SERVER,
          NULL);

    fake.calls = 0;
    fake.status = 418;
    check("any other status is UNKNOWN",
          starkbank_widget_get(client, "1", &widget, &errors) == STARKCORE_ERROR_UNKNOWN, NULL);

    fake.calls = 0;
    fake.callCap = 1;
    fake.status = 200;
    fake.reply = widgetBody;
    starkbank_widget_get(client, "1", &widget, &errors);
    starkbank_entity_free(widget);
    widget = NULL;
    /* A transport failure arrives at the status mapper as python's synthetic
       status 0, which is neither 200 nor 400 nor 500, so python raises
       UnknownError and starkcore returns UNKNOWN. This tier forwards it
       unchanged rather than "improving" it into TRANSPORT: a code a caller
       cannot match against core-python's behaviour is a worse code. */
    check("a transport failure surfaces as core maps it, and nulls the out parameter",
          starkbank_widget_get(client, "1", &widget, &errors) == STARKCORE_ERROR_UNKNOWN
          && widget == NULL, NULL);
    check("errors_count is 0 for NULL and out-of-range access is NULL",
          starkbank_errors_count(NULL) == 0 && starkbank_errors_code_at(NULL, 0) == NULL
          && starkbank_errors_message_at(NULL, 0) == NULL, NULL);
    starkbank_client_free(client);
}

/* ============================================================ facade */

/* ============================================================== parse */

/* GET /public-key?limit=1 as the API answers it, with our fixture key
   presented as Stark's, so the verify path is exercised end to end offline. */
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

static void testParse(void)
{
    starkbank_client *client = NULL;
    FakeTransport fake;
    starkbank_entity *event = NULL;
    const starkbank_entity *log = NULL;
    starkcore_user *signer = NULL;
    char *publicPem;
    char *signature = NULL;
    char keyReply[4096];
    const char *body =
        "{\"event\":{\"id\":\"7\",\"subscription\":\"widget\","
        "\"log\":{\"id\":\"9\",\"type\":\"created\"}}}";

    startGroup("parse and verify");
    publicPem = readFixture("publicKey.pem");
    if (publicPem == NULL) {
        check("publicKey.pem fixture", 0, NULL);
        return;
    }
    starkcore_project_new("1", STARKCORE_ENVIRONMENT_SANDBOX, privatePem, &signer);
    starkcore_auth_sign(signer, body, strlen(body), &signature);
    starkcore_user_free(signer);

    client = newClient(&fake);
    buildKeyReply(keyReply, sizeof(keyReply), publicPem);
    fake.reply = keyReply;
    check("a genuine signature verifies and the event envelope is unwrapped",
          starkbank_parse_and_verify(client, body, strlen(body), signature, &event, NULL)
              == STARKBANK_OK
          && event != NULL
          && equalStrings(starkbank_entity_id(event), "7"), NULL);
    check("the nested log is reachable with the same accessors",
          starkbank_entity_entity(event, "log", &log) == STARKBANK_OK && log != NULL, NULL);
    starkbank_entity_free(event);
    event = NULL;

    fake.calls = 0;
    check("a tampered body does not verify and hands back nothing",
          starkbank_parse_and_verify(client, "{\"event\":{\"id\":\"8\"}}",
                                     strlen("{\"event\":{\"id\":\"8\"}}"),
                                     signature, &event, NULL) == STARKCORE_ERROR_SIGNATURE
          && event == NULL, NULL);
    check("NULL arguments are rejected rather than dereferenced",
          starkbank_parse_and_verify(NULL, body, strlen(body), signature, &event, NULL)
              == STARKCORE_ERROR_ARGUMENT
          && starkbank_parse_and_verify(client, body, strlen(body), signature, NULL, NULL)
              == STARKCORE_ERROR_ARGUMENT, NULL);

    starkcore_free(signature);
    free(publicPem);
    starkbank_client_free(client);
}

static void testFacade(void)
{
    starkbank_user *project = NULL;
    starkbank_user *organization = NULL;
    starkbank_user *replaced = NULL;
    starkbank_client *client = NULL;

    startGroup("facade: the client owns the user");
    check("a project is built and carries its access id",
          starkbank_project_new("5656565656565656", STARKBANK_ENVIRONMENT_SANDBOX,
                                privatePem, &project) == STARKBANK_OK
          && equalStrings(starkbank_user_access_id(project), "project/5656565656565656")
          && starkbank_user_environment(project) == STARKBANK_ENVIRONMENT_SANDBOX, NULL);
    check("the client takes the user, so the host cannot outlive-order it wrong",
          starkbank_client_new(project, &client) == STARKBANK_OK, NULL);
    /* No starkbank_user_free(project) here: the client owns it now, and the
       ASan job is what proves that is neither a leak nor a double free. */
    starkbank_client_free(client);
    client = NULL;

    check("a bad key is refused at construction, not at the first request",
          starkbank_project_new("1", STARKBANK_ENVIRONMENT_SANDBOX, "not a pem", &project)
              == STARKCORE_ERROR_PRIVATE_KEY && project == NULL, NULL);
    check("a bad environment likewise",
          starkbank_project_new("1", 7, privatePem, &project) == STARKCORE_ERROR_ENVIRONMENT
          && project == NULL, NULL);

    check("an organization omits the workspace segment when it has none",
          starkbank_organization_new("1234", STARKBANK_ENVIRONMENT_SANDBOX, privatePem,
                                     NULL, &organization) == STARKBANK_OK
          && equalStrings(starkbank_user_access_id(organization), "organization/1234"), NULL);
    check("replace leaves the original untouched",
          starkbank_organization_replace(organization, "9999", &replaced) == STARKBANK_OK
          && equalStrings(starkbank_user_access_id(replaced),
                          "organization/1234/workspace/9999")
          && equalStrings(starkbank_user_access_id(organization), "organization/1234"), NULL);
    starkbank_user_free(replaced);

    check("the client takes the swapped user too",
          starkbank_client_new(organization, &client) == STARKBANK_OK, NULL);
    starkbank_organization_new("4321", STARKBANK_ENVIRONMENT_SANDBOX, privatePem, NULL, &replaced);
    check("set_user hops workspace and frees the user it replaced",
          starkbank_client_set_user(client, replaced) == STARKBANK_OK, NULL);
    starkbank_client_free(client);
    client = NULL;

    check("a NULL user is refused and the out parameter is nulled",
          starkbank_client_new(NULL, &client) == STARKCORE_ERROR_ARGUMENT && client == NULL, NULL);
    check("user accessors tolerate NULL",
          starkbank_user_access_id(NULL) == NULL
          && starkbank_user_environment(NULL) == -1, NULL);
    check("client setters reject NULL rather than dereferencing",
          starkbank_client_set_timeout(NULL, 5) == STARKCORE_ERROR_ARGUMENT
          && starkbank_client_set_language(NULL, STARKBANK_LANGUAGE_PT_BR)
                 == STARKCORE_ERROR_ARGUMENT
          && starkbank_client_set_transport(NULL, NULL, NULL) == STARKCORE_ERROR_ARGUMENT, NULL);
}

static void testClientSetters(void)
{
    starkbank_client *client = NULL;
    FakeTransport fake;
    starkbank_entity *widget = NULL;

    startGroup("client configuration reaches the wire");
    client = newClient(&fake);
    fake.reply = widgetBody;
    check("a user-agent prefix is prepended",
          starkbank_client_set_user_agent_prefix(client, "Joker") == STARKBANK_OK
          && starkbank_widget_get(client, "1", &widget, NULL) == STARKBANK_OK
          && strncmp(fake.userAgent, "Joker", 5) == 0, fake.userAgent);
    starkbank_entity_free(widget);
    widget = NULL;
    check("the remaining setters are accepted",
          starkbank_client_set_language(client, STARKBANK_LANGUAGE_PT_BR) == STARKBANK_OK
          && starkbank_client_set_timeout(client, 30) == STARKBANK_OK
          && starkbank_client_set_max_response_size(client, 1024 * 1024) == STARKBANK_OK, NULL);
    check("cache_clear is safe before anything was cached",
          (starkbank_client_cache_clear(client), starkbank_client_cache_clear(NULL), 1), NULL);
    starkbank_client_free(client);
}

int main(void)
{
    privatePem = readFixture("privateKey.pem");
    if (privatePem == NULL) {
        printf("cannot read tests/fixtures/privateKey.pem\n");
        return 1;
    }

    testLibrary();
    testRegistry();
    testReads();
    testMasked();
    testWrites();
    testBorrowedIsReadOnly();
    testList();
    testVerbs();
    testIter();
    testNested();
    testErrors();
    testParse();
    testFacade();
    testClientSetters();

    free(privatePem);
    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
