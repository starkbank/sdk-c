/*
 * Resources that exist only for the suite. They are authored with exactly the
 * macros a real resource is authored with - that is the point of them: the
 * engine is tested through the same expansion step-3's Invoice will use, not
 * through a bespoke harness that could diverge from it.
 *
 * Widget is deliberately denser than any real resource: one field of every one
 * of the fourteen types, a LIST_RESOURCE, a nested RESOURCE, a sub-resource,
 * and a legal zero amount. Gadget exists for the polymorphic case Event.log
 * will need. Ledger is the degenerate get-with-no-id shape Balance has.
 */

#ifndef STARKBANK_TEST_RESOURCES_H
#define STARKBANK_TEST_RESOURCES_H

/* The registry, in the shape starkc/registry.c expands. Params views are
   absent by design: they are reached through resource->params. */
#define STARKBANK_RESOURCES(R) \
    R(widget)                  \
    R(widget_rule)             \
    R(widget_payment)          \
    R(widget_log)              \
    R(gadget)                  \
    R(ledger)

#ifdef STARKBANK_TEST_RESOURCE_PROTOTYPES

#include <stddef.h>

#include "starkbank.h"

STARKBANK_API int STARKBANK_CALL starkbank_widget_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_widget_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_widget_create(const starkbank_client *client,
    const starkbank_list *entities, starkbank_list **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_widget_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_widget_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_widget_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_widget_update(const starkbank_client *client,
    const char *id, const starkbank_entity *patch, starkbank_entity **out,
    starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_widget_delete(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_widget_pdf(const starkbank_client *client,
    const char *id, unsigned char **out, size_t *out_len, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_widget_qrcode(const starkbank_client *client,
    const char *id, int value, unsigned char **out, size_t *out_len, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_widget_payment(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_widget_rule_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_widget_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_widget_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_widget_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_gadget_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_ledger_get(const starkbank_client *client,
    starkbank_entity **out, starkbank_errors **errors);

#endif /* STARKBANK_TEST_RESOURCE_PROTOTYPES */

#endif /* STARKBANK_TEST_RESOURCES_H */
