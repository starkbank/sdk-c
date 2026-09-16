/*
 * The C89 gate. The public header is the ABI and every FFI host's header
 * translator reads C89 at best, so the header must compile as C89 with
 * -pedantic-errors: no // comments, no long, no bool, no stdint, no
 * declarations after statements, no trailing commas in enumerators.
 *
 * This TU also proves the header is self-contained: it includes nothing
 * first, so a missing stddef.h inside starkbank.h fails here.
 */

#include "starkbank.h"

/* Force the handles to be usable as incomplete types in the caller, and force
   one declaration of every kind to be referenced so a stray definition (rather
   than a declaration) in the header would be caught by the linker gate too. */
static starkbank_client *gateClient;
static starkbank_entity *gateEntity;
static starkbank_list *gateList;
static starkbank_iter *gateIter;
static starkbank_errors *gateErrors;
static starkbank_user *gateUser;

int main(void)
{
    gateClient = 0;
    gateEntity = 0;
    gateList = 0;
    gateIter = 0;
    gateErrors = 0;
    gateUser = 0;
    return (gateClient || gateEntity || gateList || gateIter || gateErrors || gateUser) ? 1 : 0;
}
