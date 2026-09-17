/*
 * The X-macro machinery a resource is authored with. One field table plus one
 * STARKBANK_RESOURCE line per resource; the preprocessor expands it into a
 * static array and two starkbankResource structs and nothing else.
 *
 * There is no generator and no generated file. What a reviewer diffs is the
 * table itself, which is why the table has to read like data.
 */

#ifndef STARKBANK_TABLE_H
#define STARKBANK_TABLE_H

#include "internal.h"

/*
 * The flag column is spelled REQUIRED | CREATE | PATCH, unprefixed, because a
 * three-column table 700 lines long has to fit on one screen to be reviewable
 * and STARKBANK_FLAG_REQUIRED | STARKBANK_FLAG_CREATE | STARKBANK_FLAG_PATCH is
 * 66 characters of noise per row. The names are short enough to collide with a
 * platform header, so colliding is made a compile error here rather than a
 * silently wrong flag word later. Nothing in this file reaches starkbank.h.
 */
#if defined(REQUIRED) || defined(CREATE) || defined(PATCH) || defined(RO)
#  error "starkc/table.h: REQUIRED, CREATE, PATCH or RO is already defined; \
include this header before the one that took the name, or rename the column."
#endif

#define REQUIRED  STARKBANK_FLAG_REQUIRED
#define CREATE    STARKBANK_FLAG_CREATE
#define PATCH     STARKBANK_FLAG_PATCH
#define RO        0                      /* return-only: no writer accepts it */

/* One row. The type column is pasted, so a typo is an unknown identifier at
   compile time rather than a field that reads as a STRING for ever. */
#define STARKBANK_TABLE_ROW(wireKey, fieldType, fieldRef, fieldFlags) \
    { wireKey, STARKBANK_FIELD_##fieldType, fieldRef, (fieldFlags) },

/*
 * A polymorphic field's variant map, authored as a table for the same reason
 * the fields are: Event.log and PaymentPreview.payment differ only in which
 * sibling field names the variant and in what the names are, and a resolver
 * written in C per resource is per-resource code the house rules do not allow.
 *
 * STARKBANK_POLYMORPH(event, "log", "subscription", STARKBANK_EVENT_LOG_VARIANTS)
 * expands the map and the one-entry list the resource points at; a resource
 * needing two polymorphic fields gets a second entry, and nothing in the
 * engine changes.
 */
#define STARKBANK_TABLE_VARIANT(discriminatorValue, resourceName) \
    { discriminatorValue, resourceName },

#define STARKBANK_POLYMORPH(ident, fieldName, discriminatorKey, VARIANTS)  \
    static const starkbankVariant starkbankVariants_##ident[] = {          \
        VARIANTS(STARKBANK_TABLE_VARIANT)                                  \
        { NULL, NULL }                                                     \
    };                                                                     \
    static const starkbankPolymorph starkbankPolymorph_##ident[] = {       \
        { fieldName, discriminatorKey, starkbankVariants_##ident },        \
        { NULL, NULL, NULL }                                               \
    }

/*
 * Two resources per table. The second is the query and patch view of the same
 * fields under the name "<Resource>.Params": it shares the rows, so a filter
 * on a typed field is type-checked, and it additionally accepts the resource's
 * query keys, which are names with no wire type of their own.
 *
 * The params view is deliberately absent from the registry. It is reached
 * through resource->params, it hydrates nothing, and reflecting it would
 * double every binding generator's output for no caller's benefit.
 */
#define STARKBANK_RESOURCE_FULL(ident, resourceName, FIELDS, queryKeyList, polymorph) \
    static const starkbankField starkbankRows_##ident[] = {                          \
        FIELDS(STARKBANK_TABLE_ROW)                                                  \
        { NULL, 0, NULL, 0 }                                                         \
    };                                                                               \
    extern const starkbankResource starkbankParams_##ident;                          \
    extern const starkbankResource starkbankTable_##ident;                           \
    const starkbankResource starkbankTable_##ident = {                               \
        resourceName,                                                                \
        starkbankRows_##ident,                                                       \
        (int)(sizeof(starkbankRows_##ident) / sizeof(starkbankRows_##ident[0])) - 1,  \
        queryKeyList,                                                                \
        &starkbankParams_##ident,                                                    \
        0,                                                                           \
        polymorph                                                                    \
    };                                                                               \
    const starkbankResource starkbankParams_##ident = {                              \
        resourceName ".Params",                                                      \
        starkbankRows_##ident,                                                       \
        (int)(sizeof(starkbankRows_##ident) / sizeof(starkbankRows_##ident[0])) - 1,  \
        queryKeyList,                                                                \
        NULL,                                                                        \
        1,                                                                           \
        NULL                                                                         \
    }

#define STARKBANK_RESOURCE(ident, resourceName, FIELDS, queryKeyList) \
    STARKBANK_RESOURCE_FULL(ident, resourceName, FIELDS, queryKeyList, NULL)

#endif /* STARKBANK_TABLE_H */
