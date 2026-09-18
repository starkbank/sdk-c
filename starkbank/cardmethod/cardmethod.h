/*
 * CardMethod: a query-only lookup used to build CorporateRule.methods
 * filters, e.g. [{"code": "chip"}]. sdk-python's CardMethod is a SubResource
 * (no id) with a single module function, query(search=None, user=None) -
 * no page(), and no "limit" keyword at all, unlike every other query() in
 * this SDK. The docs' GET /v2/card-method parameter list agrees: it lists
 * only "search" (and "fields", which core-c/table.h's drift check already
 * excludes everywhere), so the query key list below carries no "limit" and
 * needs no known-drift entry for it - neither side has one to disagree
 * about. This build has no PAGE verb for the same reason it has no QUERY-vs-
 * PAGE mismatch: python's module defines query() and nothing else.
 *
 * CorporateRule.methods is not promoted from LIST_OBJECT to
 * LIST_RESOURCE("CardMethod") here: that table is out of scope for this
 * family and the promotion is a mechanical follow-up corporaterule.h's own
 * header already flags.
 */

#ifndef STARKBANK_CARDMETHOD_H
#define STARKBANK_CARDMETHOD_H

#define STARKBANK_CARD_METHOD_FIELDS(F)                                             \
/*    wire key           type               ref                 flags           */ \
    F("code",            STRING,            NULL,  REQUIRED | CREATE)               \
    F("name",            STRING,            NULL,  RO)                              \
    F("number",          STRING,            NULL,  RO)

#endif /* STARKBANK_CARDMETHOD_H */
