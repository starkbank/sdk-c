/*
 * VerifiedTransfer: create only. sdk-python's verifiedtransfer module exports
 * no get, query or page at all - just create(verified_transfers) over
 * rest.post_multi - so this table carries no query key list and the .c file
 * declares no PARAMS/GET_ID/QUERY/PAGE verb to match; python is normative for
 * the verb surface.
 *
 * rules is LIST_RESOURCE("Transfer.Rule"), not a VerifiedTransfer-local Rule:
 * sdk-python's __verifiedtransfer.py imports transfer.rule.Rule directly
 * ("from ..transfer.rule.__rule import Rule") and its own _parse_rules
 * hydrates unrecognised entries through transfer.rule's _sub_resource, so
 * there is exactly one Rule table on the wire here, the same one
 * Transfer.rules already uses.
 *
 * accountId is the only field the docs' response object list does not carry
 * at all - verified-transfer.json's object instead lists the VerifiedAccount
 * fields it denormalises into the response (bankCode, branchCode, name,
 * taxId, accountNumber, accountType) with no accountId key anywhere. Those
 * fields are not sdk-python attributes of VerifiedTransfer (__init__ knows
 * only account_id), and sdk-python is normative for the field set: the docs
 * are lossy here, not the table. accountId stays the one CREATE field this
 * table has for it; the denormalised fields are unmodelled the same way
 * app-docs' own doc-only overlay would leave an unmapped return-only value.
 */

#ifndef STARKBANK_VERIFIEDTRANSFER_H
#define STARKBANK_VERIFIEDTRANSFER_H

#define STARKBANK_VERIFIED_TRANSFER_FIELDS(F)                                        \
/*    wire key              type               ref                 flags        */  \
    F("amount",             AMOUNT,            NULL,  REQUIRED | CREATE)             \
    F("accountId",          STRING,            NULL,  REQUIRED | CREATE)             \
    F("externalId",         STRING,            NULL,  CREATE)                        \
    F("scheduled",          DATE_OR_DATETIME,  NULL,  CREATE)                        \
    F("description",        STRING,            NULL,  CREATE)                        \
    F("displayDescription", STRING,            NULL,  CREATE)                        \
    F("tags",               LIST_STRING,       NULL,  CREATE)                        \
    F("rules",              LIST_RESOURCE,     "Transfer.Rule", CREATE)              \
    F("id",                 STRING,            NULL,  RO)                            \
    F("fee",                AMOUNT,            NULL,  RO)                            \
    F("status",             STRING,            NULL,  RO)                            \
    F("transactionIds",     LIST_STRING,       NULL,  RO)                            \
    F("metadata",           OBJECT,            NULL,  RO)                            \
    F("created",            DATETIME,          NULL,  RO)                            \
    F("updated",            DATETIME,          NULL,  RO)

#endif /* STARKBANK_VERIFIEDTRANSFER_H */
