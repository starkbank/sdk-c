/* Return-only, reached through starkbank_invoice_payment: no verbs of its own. */

#include "../../starkc/verbs.h"

#include "invoice.h"

STARKBANK_RESOURCE(invoice_payment, "Invoice.Payment", STARKBANK_INVOICE_PAYMENT_FIELDS, NULL);
