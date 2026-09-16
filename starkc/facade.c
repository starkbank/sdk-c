/*
 * The facade: everything a host needs that is not a resource. Its job is to
 * keep starkcore out of the signatures, so a Delphi or .NET consumer binds one
 * library, one header and one allocator instead of two of each.
 *
 * Two things are absorbed here rather than forwarded. The client OWNS the user,
 * where starkcore borrows it and requires it to outlive the client - that
 * ordering is the lifetime bug an FFI host reliably writes. And the core ABI
 * check the frozen headers ask every host to make is made once, here, so no
 * host has to remember it.
 */

#include <stdlib.h>

#include "internal.h"

static int isUser(const starkbank_user *user)
{
    return user != NULL && user->magic == STARKBANK_MAGIC_USER;
}

static int isClient(const starkbank_client *client)
{
    return client != NULL && client->magic == STARKBANK_MAGIC_CLIENT;
}

starkcore_client * starkbankClientCore(const starkbank_client *client)
{
    if (!isClient(client)) {
        return NULL;
    }
    return client->core;
}

/* ---------------------------------------------------------------- library */

STARKBANK_API int STARKBANK_CALL starkbank_abi_version(void)
{
    return STARKBANK_ABI_VERSION;
}

STARKBANK_API const char * STARKBANK_CALL starkbank_version(void)
{
    return STARKBANK_VERSION;
}

STARKBANK_API const char * STARKBANK_CALL starkbank_core_version(void)
{
    return starkcore_version();
}

STARKBANK_API void STARKBANK_CALL starkbank_free(void *pointer)
{
    starkcore_free(pointer);
}

/* ------------------------------------------------------------------- user */

static int wrapUser(starkcore_user *core, starkbank_user **out)
{
    starkbank_user *user = (starkbank_user *)calloc(1, sizeof(*user));

    if (user == NULL) {
        starkcore_user_free(core);
        return STARKCORE_ERROR_MEMORY;
    }
    user->magic = STARKBANK_MAGIC_USER;
    user->core = core;
    *out = user;
    return STARKCORE_OK;
}

STARKBANK_API int STARKBANK_CALL starkbank_project_new(const char *id, int environment,
    const char *private_key_pem, starkbank_user **out)
{
    starkcore_user *core = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = starkcore_project_new(id, environment, private_key_pem, &core);
    if (status != STARKCORE_OK) {
        return status;
    }
    return wrapUser(core, out);
}

STARKBANK_API int STARKBANK_CALL starkbank_organization_new(const char *id, int environment,
    const char *private_key_pem, const char *workspace_id, starkbank_user **out)
{
    starkcore_user *core = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = starkcore_organization_new(id, environment, private_key_pem, workspace_id, &core);
    if (status != STARKCORE_OK) {
        return status;
    }
    return wrapUser(core, out);
}

STARKBANK_API int STARKBANK_CALL starkbank_organization_replace(
    const starkbank_user *organization, const char *workspace_id, starkbank_user **out)
{
    starkcore_user *core = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (!isUser(organization)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_organization_replace(organization->core, workspace_id, &core);
    if (status != STARKCORE_OK) {
        return status;
    }
    return wrapUser(core, out);
}

STARKBANK_API const char * STARKBANK_CALL starkbank_user_access_id(const starkbank_user *user)
{
    if (!isUser(user)) {
        return NULL;
    }
    return starkcore_user_access_id(user->core);
}

STARKBANK_API int STARKBANK_CALL starkbank_user_environment(const starkbank_user *user)
{
    if (!isUser(user)) {
        return -1;
    }
    return starkcore_user_environment(user->core);
}

STARKBANK_API void STARKBANK_CALL starkbank_user_free(starkbank_user *user)
{
    if (!isUser(user)) {
        return;
    }
    starkcore_user_free(user->core);
    user->magic = 0;
    free(user);
}

/* ----------------------------------------------------------------- client */

STARKBANK_API int STARKBANK_CALL starkbank_client_new(starkbank_user *user,
                                                      starkbank_client **out)
{
    starkbank_client *client;
    starkcore_client *core = NULL;
    int status;

    /* Ownership passes on failure too, so one error path serves the caller and
       a host that ignores the return code cannot leak a private key. */
    if (out == NULL) {
        starkbank_user_free(user);
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (!isUser(user)) {
        starkbank_user_free(user);
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (starkcore_abi_version() != STARKCORE_ABI_VERSION) {
        starkbank_user_free(user);
        return STARKBANK_ERROR_ABI;
    }
    status = starkcore_client_new(STARKCORE_HOST_BANK, STARKBANK_VERSION, user->core, &core);
    if (status != STARKCORE_OK) {
        starkbank_user_free(user);
        return status;
    }
    client = (starkbank_client *)calloc(1, sizeof(*client));
    if (client == NULL) {
        starkcore_client_free(core);
        starkbank_user_free(user);
        return STARKCORE_ERROR_MEMORY;
    }
    client->magic = STARKBANK_MAGIC_CLIENT;
    client->core = core;
    client->user = user;
    *out = client;
    return STARKCORE_OK;
}

STARKBANK_API int STARKBANK_CALL starkbank_client_set_transport(starkbank_client *client,
    starkbank_transport_fn transport, void *context)
{
    if (!isClient(client)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    return starkcore_client_set_transport(client->core, transport, context);
}

STARKBANK_API int STARKBANK_CALL starkbank_client_set_curl_transport(starkbank_client *client)
{
    if (!isClient(client)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
#ifdef STARKBANK_WITH_CURL
    return starkcore_client_set_transport(client->core, starkcore_transport_curl, NULL);
#else
    /* The symbol exists in every build so a host can probe and fall back,
       rather than failing to link against a library that simply was not built
       with curl. A .NET host cannot cheaply test for a missing export. */
    return STARKCORE_ERROR_NO_TRANSPORT;
#endif
}

STARKBANK_API int STARKBANK_CALL starkbank_client_set_language(starkbank_client *client,
                                                               int language)
{
    if (!isClient(client)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    return starkcore_client_set_language(client->core, language);
}

STARKBANK_API int STARKBANK_CALL starkbank_client_set_timeout(starkbank_client *client,
                                                              int seconds)
{
    if (!isClient(client)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    return starkcore_client_set_timeout(client->core, seconds);
}

STARKBANK_API int STARKBANK_CALL starkbank_client_set_user_agent_prefix(
    starkbank_client *client, const char *prefix)
{
    if (!isClient(client)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    return starkcore_client_set_user_agent_prefix(client->core, prefix);
}

STARKBANK_API int STARKBANK_CALL starkbank_client_set_max_response_size(
    starkbank_client *client, size_t bytes)
{
    if (!isClient(client)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    return starkcore_client_set_max_response_size(client->core, bytes);
}

STARKBANK_API int STARKBANK_CALL starkbank_client_set_user(starkbank_client *client,
                                                           starkbank_user *user)
{
    starkbank_user *previous;
    int status;

    if (!isClient(client) || !isUser(user)) {
        starkbank_user_free(user);
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_client_set_user(client->core, user->core);
    if (status != STARKCORE_OK) {
        starkbank_user_free(user);
        return status;
    }
    /* The old user is freed only after core stopped pointing at it. */
    previous = client->user;
    client->user = user;
    starkbank_user_free(previous);
    return STARKCORE_OK;
}

STARKBANK_API void STARKBANK_CALL starkbank_client_cache_clear(starkbank_client *client)
{
    if (!isClient(client)) {
        return;
    }
    starkcore_client_cache_clear(client->core);
}

STARKBANK_API void STARKBANK_CALL starkbank_client_free(starkbank_client *client)
{
    if (!isClient(client)) {
        return;
    }
    starkcore_client_free(client->core);
    starkbank_user_free(client->user);
    client->magic = 0;
    free(client);
}

/* -------------------------------------------------------- transport seam */

STARKBANK_API int STARKBANK_CALL starkbank_response_new(int status, const unsigned char *content,
    size_t content_len, const starkbank_headers *headers, starkbank_response **out)
{
    return starkcore_response_new(status, content, content_len, headers, out);
}

STARKBANK_API int STARKBANK_CALL starkbank_headers_count(const starkbank_headers *headers)
{
    return starkcore_headers_count(headers);
}

STARKBANK_API const char * STARKBANK_CALL starkbank_headers_name_at(
    const starkbank_headers *headers, int index)
{
    return starkcore_headers_name_at(headers, index);
}

STARKBANK_API const char * STARKBANK_CALL starkbank_headers_value_at(
    const starkbank_headers *headers, int index)
{
    return starkcore_headers_value_at(headers, index);
}

/* ----------------------------------------------------------------- errors */

STARKBANK_API int STARKBANK_CALL starkbank_errors_count(const starkbank_errors *errors)
{
    return starkcore_errors_count(errors);
}

STARKBANK_API const char * STARKBANK_CALL starkbank_errors_code_at(
    const starkbank_errors *errors, int index)
{
    return starkcore_errors_code_at(errors, index);
}

STARKBANK_API const char * STARKBANK_CALL starkbank_errors_message_at(
    const starkbank_errors *errors, int index)
{
    return starkcore_errors_message_at(errors, index);
}

STARKBANK_API void STARKBANK_CALL starkbank_errors_free(starkbank_errors *errors)
{
    starkcore_errors_free(errors);
}

/* ------------------------------------------------------------------ parse */

STARKBANK_API int STARKBANK_CALL starkbank_parse_and_verify(const starkbank_client *client,
    const char *content, size_t content_len, const char *signature_base64,
    starkbank_entity **out, starkbank_errors **errors)
{
    starkcore_json *document = NULL;
    int status;

    if (errors != NULL) {
        *errors = NULL;
    }
    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (starkbankClientCore(client) == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    /* The "event" envelope key is supplied here so a host never has to know
       the webhook body is wrapped. */
    status = starkcore_parse_and_verify(starkbankClientCore(client), content, content_len,
                                        signature_base64, "event", &document, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    return starkbankEntityWrap(starkbankRegistryFind("Event"), document, document, out);
}
