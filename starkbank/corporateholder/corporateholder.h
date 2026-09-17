/*
 * CorporateHolder, its Log and its Permission sub-resource.
 *
 * expand is not modeled as a create/get parameter: sdk-python's create, get,
 * query and page all take an expand=["rules"] keyword, but core-c's CREATE
 * and GET_ID verb shapes carry no query of their own to put it in - the same
 * gap Invoice's [EXPANDABLE] rules already leaves unmodeled in this build,
 * since Invoice's docs do not even list "expand" as a filter. CorporateHolder's
 * docs do list it on the GET list endpoint, and QUERY/PAGE already forward any
 * bare query key through the params entity for free, so "expand" rides in
 * STARKBANK_CORPORATE_HOLDER_QUERY: a caller who lists or pages holders gets
 * real expand support today, while create() and get() do not until those verb
 * shapes grow a query parameter of their own.
 *
 * status carries PATCH and not CREATE: sdk-python's update() sends it, and the
 * class docstring files status under "Attributes (return-only)" rather than
 * under "## Parameters", so a caller blocks or unblocks a holder through
 * update() and never sets it at create.
 *
 * permissions and rules are both CREATE and PATCH: sdk-python's create() posts
 * the whole CorporateHolder object it is given, and both fields are accepted
 * by the documented POST /v2/corporate-holder body as well as by update()'s
 * explicit payload dict, so both halves agree here (unlike CorporateCard,
 * where only holderId is a documented create field - see corporatecard.h).
 *
 * Permission has no id: it is a SubResource in sdk-python, not a Resource, and
 * carries none of the id/created-by-the-API shape the way CorporateRule does.
 */

#ifndef STARKBANK_CORPORATEHOLDER_H
#define STARKBANK_CORPORATEHOLDER_H

#define STARKBANK_CORPORATE_HOLDER_FIELDS(F)                                          \
/*    wire key           type               ref                          flags     */ \
    F("name",            STRING,            NULL,  REQUIRED | CREATE | PATCH)          \
    F("centerId",        STRING,            NULL,  CREATE | PATCH)                     \
    F("permissions",     LIST_RESOURCE,     "Permission",    CREATE | PATCH)           \
    F("rules",           LIST_RESOURCE,     "CorporateRule", CREATE | PATCH)           \
    F("tags",            LIST_STRING,       NULL,  CREATE | PATCH)                     \
    F("status",          STRING,            NULL,  PATCH)                              \
    F("id",              STRING,            NULL,  RO)                                 \
    F("updated",         DATETIME,          NULL,  RO)                                 \
    F("created",         DATETIME,          NULL,  RO)

#define STARKBANK_PERMISSION_FIELDS(F)                                                 \
    F("ownerId",         STRING,            NULL,  CREATE)                             \
    F("ownerType",       STRING,            NULL,  CREATE)                             \
    F("ownerEmail",      STRING,            NULL,  RO)                                 \
    F("ownerName",       STRING,            NULL,  RO)                                 \
    F("ownerPictureUrl", STRING,            NULL,  RO)                                 \
    F("ownerStatus",     STRING,            NULL,  RO)                                 \
    F("created",         DATETIME,          NULL,  RO)

#define STARKBANK_CORPORATE_HOLDER_LOG_FIELDS(F)                                      \
    F("id",              STRING,            NULL,  RO)                                 \
    F("created",         DATETIME,          NULL,  RO)                                 \
    F("type",            STRING,            NULL,  RO)                                 \
    F("holder",          RESOURCE,          "CorporateHolder", RO)

#endif /* STARKBANK_CORPORATEHOLDER_H */
