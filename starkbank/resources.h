/*
 * The registry: every resource this library knows, one line each. starkc/registry.c
 * expands it twice, once for the extern declarations and once for the table of
 * pointers, so adding a resource is this line and nothing else.
 *
 * Params views are deliberately absent. They are reached through
 * resource->params, they hydrate nothing, and reflecting them would double
 * every binding generator's output for no caller's benefit.
 */

#ifndef STARKBANK_RESOURCES_H
#define STARKBANK_RESOURCES_H

#define STARKBANK_RESOURCES(R) \
    R(balance)                 \
    R(boleto)                  \
    R(boleto_log)              \
    R(boleto_payment)          \
    R(boleto_payment_log)      \
    R(boleto_preview)          \
    R(brcode_preview)          \
    R(event)                   \
    R(event_attempt)           \
    R(invoice)                 \
    R(invoice_log)             \
    R(invoice_payment)         \
    R(invoice_rule)            \
    R(payment_preview)         \
    R(split)                   \
    R(tax_preview)             \
    R(transfer)                \
    R(transfer_log)            \
    R(transfer_rule)           \
    R(utility_preview)         \
    R(webhook)

#endif /* STARKBANK_RESOURCES_H */
