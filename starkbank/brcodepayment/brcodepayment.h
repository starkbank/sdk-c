/*
 * BrcodePayment, brcodepayment.Log and BrcodePayment.Rule. Tables only.
 *
 * status carries PATCH and neither CREATE nor REQUIRED: sdk-python's update()
 * takes only status (payload={"status": status}), the same shape Invoice's
 * status field has, and for the same reason - a create-time value and an
 * update-time value are different facts even when they share one wire key.
 *
 * scheduled is DATE, not DATE_OR_DATETIME: sdk-python calls check_date(
 * scheduled) despite its docstring's prose claiming "date, datetime.datetime
 * or string". The docstring is stale and the coercion is normative
 * (design.md ranks sdk-python's input coercion above its own prose); a table
 * that said DATE_OR_DATETIME here would invent a semantic python's code does
 * not honour.
 *
 * A BrcodePayment.Rule's value is an integer, like Transfer.Rule's and unlike
 * Invoice.Rule's list of strings - each Rule keeps its own table and tag.
 */

#ifndef STARKBANK_BRCODEPAYMENT_H
#define STARKBANK_BRCODEPAYMENT_H

#define STARKBANK_BRCODEPAYMENT_FIELDS(F)                                            \
/*    wire key           type               ref                       flags     */ \
    F("brcode",          STRING,            NULL,  REQUIRED | CREATE)                \
    F("taxId",           STRING,            NULL,  REQUIRED | CREATE)                \
    F("description",     STRING,            NULL,  REQUIRED | CREATE)                \
    F("amount",          AMOUNT,            NULL,  CREATE)                           \
    F("scheduled",       DATE,              NULL,  CREATE)                           \
    F("tags",            LIST_STRING,       NULL,  CREATE)                           \
    F("rules",           LIST_RESOURCE,     "BrcodePayment.Rule", CREATE)            \
    F("id",              STRING,            NULL,  RO)                               \
    F("name",            STRING,            NULL,  RO)                               \
    F("status",          STRING,            NULL,  PATCH)                            \
    F("type",            STRING,            NULL,  RO)                               \
    F("transactionIds",  LIST_STRING,       NULL,  RO)                               \
    F("fee",             AMOUNT,            NULL,  RO)                               \
    F("updated",         DATETIME,          NULL,  RO)                               \
    F("created",         DATETIME,          NULL,  RO)

#define STARKBANK_BRCODEPAYMENT_RULE_FIELDS(F)                                       \
    F("key",              STRING,            NULL,  REQUIRED | CREATE)                \
    F("value",            NUMBER,            NULL,  REQUIRED | CREATE)

#define STARKBANK_BRCODEPAYMENT_LOG_FIELDS(F)                                        \
    F("id",               STRING,            NULL,  RO)                              \
    F("created",          DATETIME,          NULL,  RO)                              \
    F("type",             STRING,            NULL,  RO)                              \
    F("errors",           LIST_STRING,       NULL,  RO)                              \
    F("payment",          RESOURCE,          "BrcodePayment", RO)

#endif /* STARKBANK_BRCODEPAYMENT_H */
