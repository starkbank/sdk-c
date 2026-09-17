/*
 * GET /v2/darf-payment/:id/pdf
 *
 * Emitted from the DarfPayment table by tools/emit.py. Edit the table, never this
 * file: make check-emit regenerates it and fails on any difference.
 */
#include <stdio.h>
#include <stdlib.h>

#include "starkbank.h"

/*<*/
static starkbank_client *connect(void)
{
    starkbank_user *project = NULL;
    starkbank_client *client = NULL;

    if (starkbank_project_new("5656565656565656", STARKBANK_ENVIRONMENT_SANDBOX,
                              getenv("STARK_PRIVATE_KEY"), &project) != STARKBANK_OK) {
        return NULL;
    }
    if (starkbank_client_new(project, &client) != STARKBANK_OK) {
        return NULL;
    }
    starkbank_client_set_curl_transport(client);
    return client;
}

static int report(int status, starkbank_errors *errors)
{
    int index;

    fprintf(stderr, "%s\n", starkbank_strerror(status));
    for (index = 0; index < starkbank_errors_count(errors); index++) {
        fprintf(stderr, "  %s: %s\n", starkbank_errors_code_at(errors, index),
                starkbank_errors_message_at(errors, index));
    }
    starkbank_errors_free(errors);
    return 1;
}
/*>*/

int main(void)
{
    starkbank_client *client = NULL;
    unsigned char *content = NULL;
    size_t length = 0;
    starkbank_errors *errors = NULL;
    FILE *file;
    int status;

    client = connect();
    if (client == NULL) {
        return 1;
    }

    status = starkbank_darf_payment_pdf(client, "5656565656565656", &content, &length, &errors);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    file = fopen("darf_payment-pdf.bin", "wb");
    if (file != NULL) {
        fwrite(content, 1, length, file);
        fclose(file);
    }
    printf("%lu bytes\n", (unsigned long)length);
    starkbank_free(content);

    starkbank_client_free(client);
    return 0;
}
