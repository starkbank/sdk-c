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
    R(allowed_installment)     \
    R(balance)                 \
    R(boleto)                  \
    R(boleto_holmes)           \
    R(boleto_holmes_log)       \
    R(boleto_log)              \
    R(boleto_payment)          \
    R(boleto_payment_log)      \
    R(boleto_preview)          \
    R(brcode_payment)          \
    R(brcode_payment_log)      \
    R(brcode_payment_rule)     \
    R(brcode_preview)          \
    R(card_method)             \
    R(corporate_balance)       \
    R(corporate_card)          \
    R(corporate_card_log)      \
    R(corporate_holder)        \
    R(corporate_holder_log)    \
    R(corporate_invoice)       \
    R(corporate_purchase)      \
    R(corporate_purchase_log)  \
    R(corporate_rule)          \
    R(corporate_transaction)   \
    R(corporate_withdrawal)    \
    R(darf_payment)            \
    R(darf_payment_log)        \
    R(deposit)                 \
    R(deposit_log)             \
    R(dict_key)                \
    R(dynamic_brcode)          \
    R(dynamic_brcode_rule)     \
    R(event)                   \
    R(event_attempt)           \
    R(institution)             \
    R(invoice)                 \
    R(invoice_log)             \
    R(invoice_payment)         \
    R(invoice_pull_request)    \
    R(invoice_pull_request_log) \
    R(invoice_pull_subscription) \
    R(invoice_pull_subscription_log) \
    R(invoice_rule)            \
    R(merchant_card)           \
    R(merchant_card_log)       \
    R(merchant_category)       \
    R(merchant_country)        \
    R(merchant_installment)    \
    R(merchant_installment_log) \
    R(merchant_purchase)       \
    R(merchant_purchase_log)   \
    R(merchant_session)        \
    R(merchant_session_log)    \
    R(payment_preview)         \
    R(payment_request)         \
    R(permission)              \
    R(purchase)                \
    R(split)                   \
    R(tax_payment)             \
    R(tax_payment_log)         \
    R(tax_preview)             \
    R(transaction)             \
    R(transfer)                \
    R(transfer_log)            \
    R(transfer_rule)           \
    R(utility_payment)         \
    R(utility_payment_log)     \
    R(utility_preview)         \
    R(webhook)                 \
    R(workspace)

#endif /* STARKBANK_RESOURCES_H */
