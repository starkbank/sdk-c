/*
 * One client, eight threads, the fake transport, under ThreadSanitizer.
 *
 * The header promises that a configured starkbank_client is immutable and may
 * be shared across threads, including concurrent starkbank_parse_and_verify,
 * which writes the public key cache under the client's lock. That promise is
 * the one an FFI host is most likely to take at face value and the one nothing
 * else in the suite exercises: every other binary here is single-threaded, and
 * a data race in the cache would show up as a corrupted key in a bank's
 * process rather than as a failing assertion.
 *
 * Entities, lists and iterators stay single-owner, as the header says: each
 * thread builds and frees its own. What is shared is exactly what the header
 * says may be shared.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "starkbank.h"

#define THREADS 8
#define ROUNDS 25

static int failed;

static void check(const char *name, int ok)
{
    if (ok) {
        printf("  ok   %s\n", name);
        return;
    }
    failed++;
    printf("  FAIL %s\n", name);
}

/*
 * Reentrant by construction: it reads its context and writes nothing. The
 * header requires a host transport to be reentrant and this is what that
 * means - a transport with a call counter in it would be reporting its own
 * race, not the library's.
 */
typedef struct {
    const char *one;
    const char *page;
    const char *keys;
} Replies;

static int STARKBANK_CALL fakeTransport(void *context, int method, const char *url,
                                        const starkbank_headers *headers, const char *body,
                                        size_t bodyLength, int timeoutSeconds,
                                        starkbank_response **out)
{
    const Replies *replies = (const Replies *)context;
    const char *reply = replies->page;

    if (strstr(url, "public-key") != NULL) {
        reply = replies->keys;
    } else if (strstr(url, "/invoice/5") != NULL) {
        reply = replies->one;           /* get_id unwraps the singular envelope key */
    }

    (void)method;
    (void)headers;
    (void)body;
    (void)bodyLength;
    (void)timeoutSeconds;
    return starkbank_response_new(200, (const unsigned char *)reply, strlen(reply), NULL, out);
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
    if (buffer == NULL || fread(buffer, 1, (size_t)size, handle) != (size_t)size) {
        free(buffer);
        fclose(handle);
        return NULL;
    }
    fclose(handle);
    buffer[size] = '\0';
    return buffer;
}

/* The key listing the client fetches on first verify, built through starkcore's
   own printer: a PEM has newlines in it and hand-rolling the escaping here
   would be testing snprintf, not the library. */
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

typedef struct {
    starkbank_client *client;
    const char *body;
    const char *signature;
    int errors;
} Work;

static void *worker(void *argument)
{
    Work *work = (Work *)argument;
    int round;

    for (round = 0; round < ROUNDS; round++) {
        starkbank_entity *invoice = NULL;
        starkbank_entity *event = NULL;
        starkbank_entity *params = NULL;
        starkbank_list *page = NULL;
        char *cursor = NULL;
        const char *brcode = NULL;

        if (starkbank_invoice_get(work->client, "5", &invoice, NULL) != STARKBANK_OK
            || starkbank_entity_string(invoice, "brcode", &brcode) != STARKBANK_OK
            || strcmp(brcode, "00020101") != 0) {
            work->errors++;
        }
        starkbank_entity_free(invoice);

        starkbank_invoice_params_new(&params);
        starkbank_entity_set_string(params, "status", "paid");
        if (starkbank_invoice_page(work->client, params, &page, &cursor, NULL) != STARKBANK_OK
            || starkbank_list_count(page) != 1) {
            work->errors++;
        }
        starkbank_free(cursor);
        starkbank_list_free(page);
        starkbank_entity_free(params);

        /* The one genuinely shared mutable thing in the library: the client's
           public key cache, written the first time any thread verifies. */
        if (starkbank_parse_and_verify(work->client, work->body, strlen(work->body),
                                       work->signature, &event, NULL) != STARKBANK_OK) {
            work->errors++;
        }
        starkbank_entity_free(event);
    }
    return NULL;
}

int main(void)
{
    starkbank_user *project = NULL;
    starkcore_user *signer = NULL;
    starkbank_client *client = NULL;
    pthread_t threads[THREADS];
    Work work[THREADS];
    Replies replies;
    char keyReply[4096];
    char *privatePem = readFixture("privateKey.pem");
    char *publicPem = readFixture("publicKey.pem");
    char *signature = NULL;
    int index;
    int errors = 0;
    const char *body = "{\"event\":{\"id\":\"7\",\"subscription\":\"invoice\","
                       "\"log\":{\"id\":\"9\",\"type\":\"created\"}}}";
    const char *one = "{\"invoice\":{\"id\":\"5\",\"brcode\":\"00020101\"}}";
    const char *page = "{\"invoices\":[{\"id\":\"5\",\"brcode\":\"00020101\"}],\"cursor\":\"\"}";

    printf("\nthreads: one client, %d threads, %d rounds each\n", THREADS, ROUNDS);
    if (privatePem == NULL || publicPem == NULL) {
        check("key fixtures", 0);
        return 1;
    }
    buildKeyReply(keyReply, sizeof(keyReply), publicPem);

    starkbank_project_new("5656565656565656", STARKBANK_ENVIRONMENT_SANDBOX, privatePem, &project);
    starkbank_client_new(project, &client);
    replies.one = one;
    replies.page = page;
    replies.keys = keyReply;
    starkbank_client_set_transport(client, fakeTransport, &replies);

    starkcore_project_new("1", STARKCORE_ENVIRONMENT_SANDBOX, privatePem, &signer);
    starkcore_auth_sign(signer, body, strlen(body), &signature);
    starkcore_user_free(signer);

    for (index = 0; index < THREADS; index++) {
        work[index].client = client;
        work[index].body = body;
        work[index].signature = signature;
        work[index].errors = 0;
        if (pthread_create(&threads[index], NULL, worker, &work[index]) != 0) {
            check("pthread_create", 0);
            return 1;
        }
    }
    for (index = 0; index < THREADS; index++) {
        pthread_join(threads[index], NULL);
        errors += work[index].errors;
    }

    check("every thread read, paged and verified with no error", errors == 0);
    starkcore_free(signature);
    free(privatePem);
    free(publicPem);
    starkbank_client_free(client);
    printf("\nthreads: %s\n", failed == 0 ? "ok" : "FAILED");
    return failed == 0 ? 0 : 1;
}
