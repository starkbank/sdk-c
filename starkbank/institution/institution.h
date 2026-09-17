/*
 * Institution. sdk-python models it as a SubResource, not a Resource: there
 * is no id field at all, and the module's only function is query(), which is
 * itself `rest.get_page(...)[0]` - one page call, the cursor thrown away, no
 * looping across pages. That is the PAGE shape wearing a different name, not
 * the QUERY/stream shape, so sdk-c exposes starkbank_institution_page
 * directly rather than inventing an iterator python's own code never builds.
 * Pass NULL for out_cursor to match python's query() exactly, or capture it
 * if a caller wants more than python offers.
 */

#ifndef STARKBANK_INSTITUTION_H
#define STARKBANK_INSTITUTION_H

#define STARKBANK_INSTITUTION_FIELDS(F)                                             \
/*    wire key           type               ref                 flags           */ \
    F("displayName",     STRING,            NULL,  RO)                              \
    F("name",            STRING,            NULL,  RO)                              \
    F("spiCode",         STRING,            NULL,  RO)                              \
    F("strCode",         STRING,            NULL,  RO)

#endif /* STARKBANK_INSTITUTION_H */
