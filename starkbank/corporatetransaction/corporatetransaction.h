/*
 * CorporateTransaction: a read-only ledger entry. sdk-python's module has no
 * create(), no update() and no delete() - a caller never posts a transaction,
 * the API writes one for every balance shift (a purchase, an invoice, a
 * withdrawal) - so this table has no CREATE bit anywhere and no NEW/POST verb.
 *
 * query()'s actual signature takes source, tags, external_ids, after, before,
 * ids and limit; its docstring also prose-lists a "status" filter that the
 * signature does not accept. Code is normative over docstring prose here (the
 * same rule CorporateTransaction's query key list below follows), so "status"
 * is not in STARKBANK_CORPORATE_TRANSACTION_QUERY.
 *
 * Field order mirrors sdk-python's __init__(id, amount, balance, description,
 * source, tags, created) exactly, id included: every field is RO, so there is
 * no CREATE-then-RO split to sort id out of the way for, the same shape
 * CorporateCard.Log and CorporateHolder.Log already use.
 */

#ifndef STARKBANK_CORPORATETRANSACTION_H
#define STARKBANK_CORPORATETRANSACTION_H

#define STARKBANK_CORPORATE_TRANSACTION_FIELDS(F)                                    \
/*    wire key           type               ref               flags            */    \
    F("id",              STRING,            NULL,  RO)                               \
    F("amount",          AMOUNT,            NULL,  RO)                               \
    F("balance",         AMOUNT,            NULL,  RO)                               \
    F("description",     STRING,            NULL,  RO)                               \
    F("source",          STRING,            NULL,  RO)                               \
    F("tags",            LIST_STRING,       NULL,  RO)                               \
    F("created",         DATETIME,          NULL,  RO)

#endif /* STARKBANK_CORPORATETRANSACTION_H */
