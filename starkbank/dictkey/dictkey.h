/*
 * DictKey. Query/get only: sdk-python's DictKey has no create() - an EVP key
 * is created automatically for every new Workspace, since an active DICT key
 * is required for Invoice to work - and no update(); every attribute is
 * return-only. id is the PIX key itself (an email, a tax id, a phone number
 * or a DICT-issued EVP uuid), which is why get() takes it as a plain string
 * identifier exactly like every other resource's id, with nothing special in
 * the table for it.
 */

#ifndef STARKBANK_DICTKEY_H
#define STARKBANK_DICTKEY_H

#define STARKBANK_DICT_KEY_FIELDS(F)                                                \
/*    wire key           type               ref                 flags           */ \
    F("id",              STRING,            NULL,  RO)                             \
    F("type",            STRING,            NULL,  RO)                             \
    F("name",            STRING,            NULL,  RO)                             \
    F("taxId",           STRING,            NULL,  RO)                             \
    F("ownerType",       STRING,            NULL,  RO)                             \
    F("bankName",        STRING,            NULL,  RO)                             \
    F("ispb",            STRING,            NULL,  RO)                             \
    F("branchCode",      STRING,            NULL,  RO)                             \
    F("accountNumber",   STRING,            NULL,  RO)                             \
    F("accountType",     STRING,            NULL,  RO)                             \
    F("status",          STRING,            NULL,  RO)

#endif /* STARKBANK_DICTKEY_H */
