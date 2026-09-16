/*
 * The engine's private contract. Nothing here crosses the ABI: the public
 * header names six opaque handles and this file is where they have a layout.
 *
 * Internals are lowerCamelCase with a bare starkbank prefix (starkbankFieldFind),
 * so the archive's external symbols all live in the starkbank namespace and
 * check-exports can tell a legitimate cross-file helper from a leaked
 * dependency. Only the public entry points carry the starkbank_ underscore
 * spelling, and only those are in the shared library's export list.
 */

#ifndef STARKBANK_INTERNAL_H
#define STARKBANK_INTERNAL_H

#include <stddef.h>

#include <starkcore.h>

#include "starkbank.h"

/* --------------------------------------------------------------- tables */

typedef struct starkbankField {
    const char *key;    /* the wire key, camelCase, exactly as the API spells it */
    int type;           /* STARKBANK_FIELD_* */
    const char *ref;    /* resource name for RESOURCE and LIST_RESOURCE, else NULL */
    int flags;          /* OR of STARKBANK_FLAG_*; 0 is return-only */
} starkbankField;

struct starkbankResource;

/*
 * Event.log is an InvoiceLog, a TransferLog or one of eight others, and which
 * one is written in the sibling "subscription" field rather than in any table.
 * A resource with a polymorphic field carries one of these and the engine calls
 * it instead of resolving field->ref; returning NULL leaves the nested entity
 * untagged and permissive, which is how a subscription this build predates
 * still hydrates.
 */
typedef const struct starkbankResource *(*starkbankRefFn)(const starkcore_json *object,
                                                          const char *field);

typedef struct starkbankResource {
    const char *name;                       /* "Invoice", "InvoiceLog", "Invoice.Rule" */
    const starkbankField *fields;
    int fieldCount;
    const char *const *queryKeys;           /* NULL-terminated; NULL for none */
    const struct starkbankResource *params; /* the query/patch view of this table */
    int isParams;                           /* nonzero on that view itself */
    starkbankRefFn refResolve;
} starkbankResource;

/* --------------------------------------------------------------- handles */

/*
 * Every handle starts with a magic word, checked on entry and cleared on free.
 * An FFI host has no type system to lean on and hands back whatever integer it
 * kept, so the alternative to this word is a wild pointer dereference inside a
 * bank's process. It is a guard against a WRONG or STALE handle, not a licence
 * to free twice: reading a word out of a block already returned to the
 * allocator is undefined, and ASan reports it.
 */
#define STARKBANK_MAGIC_ENTITY 0x5342454eu   /* "SBEN" */
#define STARKBANK_MAGIC_LIST   0x53424c49u   /* "SBLI" */
#define STARKBANK_MAGIC_ITER   0x53424954u   /* "SBIT" */
#define STARKBANK_MAGIC_USER   0x53425553u   /* "SBUS" */
#define STARKBANK_MAGIC_CLIENT 0x5342434cu   /* "SBCL" */

/*
 * Wrappers handed out for nested reads. They hang off a separate block rather
 * than sitting in the entity, so a read through a const starkbank_entity * can
 * cache one without casting the qualifier away: the pointer is const, the
 * thing it points at is not. The block is carved out of the entity's own
 * allocation, so it costs no second malloc.
 */
typedef struct starkbankChildren {
    starkbank_entity **items;
    int count;
    int capacity;
} starkbankChildren;

struct starkbank_entity {
    unsigned int magic;
    const starkbankResource *resource;  /* NULL: untagged, permissive, unvalidated */
    starkcore_json *owned;              /* non-NULL when this entity may be mutated */
    const starkcore_json *json;         /* the document; equals owned when owned */
    starkbankChildren *children;
};

struct starkbank_list {
    unsigned int magic;
    starkbank_entity **items;
    int count;
    int capacity;
    starkcore_json *document;           /* a page the items borrow from; freed last */
};

struct starkbank_iter {
    unsigned int magic;
    starkcore_stream *stream;
    const starkbankResource *resource;
    starkbank_entity *current;          /* rewrapped per step; borrowed by the caller */
};

struct starkbank_user {
    unsigned int magic;
    starkcore_user *core;
};

struct starkbank_client {
    unsigned int magic;
    starkcore_client *core;
    starkbank_user *user;               /* owned, unlike starkcore's borrowed one */
};

/* ----------------------------------------------------------------- table */

const starkbankField * starkbankFieldFind(const starkbankResource *resource, const char *key);
int starkbankQueryKeyKnown(const starkbankResource *resource, const char *key);
const starkbankResource * starkbankFieldRef(const starkbankResource *resource,
                                            const starkbankField *field,
                                            const starkcore_json *object);
int starkbankTypeReadableAs(int fieldType, int accessor);
int starkbankTypeWritableAs(int fieldType, int accessor);

/* The accessor a caller reached for, so one validator serves all of them. */
#define STARKBANK_ACCESS_STRING    0
#define STARKBANK_ACCESS_AMOUNT    1
#define STARKBANK_ACCESS_NUMBER    2
#define STARKBANK_ACCESS_BOOL      3
#define STARKBANK_ACCESS_DATETIME  4
#define STARKBANK_ACCESS_DATE      5
#define STARKBANK_ACCESS_SECONDS   6
#define STARKBANK_ACCESS_LIST      7
#define STARKBANK_ACCESS_STRINGS   8   /* an element of a LIST_STRING */
#define STARKBANK_ACCESS_ENTITIES  9   /* an element of a LIST_RESOURCE or LIST_OBJECT */
#define STARKBANK_ACCESS_ENTITY   10   /* a RESOURCE or OBJECT */

/* ---------------------------------------------------------------- entity */

int starkbankEntityWrap(const starkbankResource *resource, const starkcore_json *json,
                        starkcore_json *owned, starkbank_entity **out);
int starkbankEntityNew(const starkbankResource *resource, starkbank_entity **out);
int starkbankEntityDehydrate(const starkbank_entity *entity, int flag, starkcore_json **out);
/* Which REQUIRED key a create payload is missing. Internal for now: the ABI
   has no channel to carry the name back, and adding one is step 4's call. */
int starkbankEntityRequiredMissing(const starkbank_entity *entity, const char **outKey);

/* ------------------------------------------------------------------ list */

int starkbankListFromJson(const starkbankResource *resource, starkcore_json *array,
                          starkbank_list **out);

/* ------------------------------------------------------------------ iter */

int starkbankIterNew(starkcore_stream *stream, const starkbankResource *resource,
                     starkbank_iter **out);

/* -------------------------------------------------------------- registry */

const starkbankResource * starkbankRegistryFind(const char *name);

/* ----------------------------------------------------------------- verbs */

starkcore_client * starkbankClientCore(const starkbank_client *client);

int starkbankVerbCreate(const starkbank_client *client, const starkbankResource *resource,
                        const starkbank_list *entities, starkbank_list **out,
                        starkbank_errors **errors);
int starkbankVerbGetId(const starkbank_client *client, const starkbankResource *resource,
                       const char *id, starkbank_entity **out, starkbank_errors **errors);
int starkbankVerbGetFirst(const starkbank_client *client, const starkbankResource *resource,
                          starkbank_entity **out, starkbank_errors **errors);
int starkbankVerbQuery(const starkbank_client *client, const starkbankResource *resource,
                       const starkbank_entity *params, int limit, starkbank_iter **out);
int starkbankVerbPage(const starkbank_client *client, const starkbankResource *resource,
                      const starkbank_entity *params, starkbank_list **out,
                      char **outCursor, starkbank_errors **errors);
int starkbankVerbPatchId(const starkbank_client *client, const starkbankResource *resource,
                         const char *id, const starkbank_entity *patch,
                         starkbank_entity **out, starkbank_errors **errors);
int starkbankVerbDeleteId(const starkbank_client *client, const starkbankResource *resource,
                          const char *id, starkbank_entity **out, starkbank_errors **errors);
int starkbankVerbContent(const starkbank_client *client, const starkbankResource *resource,
                         const char *id, const char *subResourceName,
                         const char *queryKey, int queryValue, int queryMin, int queryMax,
                         unsigned char **out, size_t *outLen, starkbank_errors **errors);
int starkbankVerbSubResource(const starkbank_client *client, const starkbankResource *resource,
                             const char *id, const char *subResourceName, const char *tagName,
                             starkbank_entity **out, starkbank_errors **errors);

#endif /* STARKBANK_INTERNAL_H */
