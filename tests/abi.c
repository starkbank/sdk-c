/*
 * The core-ABI guard, exercised rather than asserted.
 *
 * starkbank_client_new refuses to build a client on a starkcore whose ABI
 * version is not the one this library compiled against - the case an FFI host
 * hits when it ships libstarkbank and libstarkcore as separate files and
 * upgrades one of them. There is no way to reach that from inside a build
 * that agrees with its starkcore, so this binary is compiled with
 *
 *     -Dstarkcore_abi_version=sliceFakeAbiVersion
 *
 * which redirects the call in starkc/facade.c to the stub below. Nothing in
 * the library is changed to make itself testable; the one call is renamed at
 * the preprocessor, which is why this has to be its own program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "starkbank.h"

int sliceFakeAbiVersion(void);

int sliceFakeAbiVersion(void)
{
    return STARKCORE_ABI_VERSION + 1;
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
    if (buffer == NULL || fread(buffer, 1, (size_t)size, handle) != (size_t)size) {
        free(buffer);
        fclose(handle);
        return NULL;
    }
    fclose(handle);
    buffer[size] = '\0';
    return buffer;
}

int main(void)
{
    starkbank_user *project = NULL;
    starkbank_client *client = NULL;
    char *pem = readFile("privateKey.pem");
    int status;

    if (pem == NULL) {
        printf("cannot read the private key fixture\n");
        return 1;
    }
    if (starkbank_project_new("5656565656565656", STARKBANK_ENVIRONMENT_SANDBOX,
                              pem, &project) != STARKBANK_OK) {
        printf("  FAIL the project should still build: the guard is on the client\n");
        free(pem);
        return 1;
    }
    /* The client takes the user even when it refuses to be built, so an
       ignored return code cannot leak a private key. */
    status = starkbank_client_new(project, &client);
    free(pem);

    printf("\ncore ABI mismatch\n");
    if (status != STARKBANK_ERROR_ABI) {
        printf("  FAIL a mismatched starkcore must be STARKBANK_ERROR_ABI, got %d (%s)\n",
               status, starkbank_strerror(status));
        return 1;
    }
    if (client != NULL) {
        printf("  FAIL the out parameter must be NULL on failure\n");
        return 1;
    }
    printf("  ok   a mismatched starkcore is refused at starkbank_client_new\n");
    printf("  ok   and the out parameter is nulled\n");
    printf("\n2 passed, 0 failed\n");
    return 0;
}
