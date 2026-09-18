/*
 * Workspace.
 *
 * create is post_single, like Webhook: sdk-python builds one Workspace
 * locally from username/name/allowed_tax_ids and posts it as the body
 * itself, not wrapped in a list.
 *
 * picture is the one field with judgement in it. sdk-python's update() takes
 * picture as raw bytes and picture_type as a separate parameter, then builds
 * exactly one wire value itself:
 *     payload["picture"] = "data:{picture_type};base64,{base64(picture)}"
 * picture_type never reaches the wire as its own key - it is only ever
 * encoded into the "picture" string - so there is exactly one wire field to
 * table, and its wire representation is already a plain string. No new
 * engine verb shape is needed, and none is added: the existing generic
 * starkbank_entity_set_string covers it completely. A caller wanting to send
 * a picture builds the same "data:<mime>;base64,<...>" string sdk-python
 * builds, base64-encoding their own bytes, and calls
 * starkbank_entity_set_string(patch, STARKBANK_WORKSPACE_PICTURE, dataUri) -
 * a plain const char *, the smallest possible shape and the same preference
 * STARKBANK_VERB_CONTENT_QUERY's own header comment argues for boleto.pdf's
 * two string keys.
 *
 * status carries PATCH and neither CREATE nor REQUIRED, the same shape
 * Invoice.status and BrcodePayment.status have: sdk-python's update() takes
 * it as one key among several, never at create.
 */

#ifndef STARKBANK_WORKSPACE_H
#define STARKBANK_WORKSPACE_H

#define STARKBANK_WORKSPACE_FIELDS(F)                                               \
/*    wire key           type               ref                 flags            */ \
    F("username",        STRING,            NULL,  REQUIRED | CREATE | PATCH)        \
    F("name",            STRING,            NULL,  REQUIRED | CREATE | PATCH)        \
    F("allowedTaxIds",   LIST_STRING,       NULL,  CREATE | PATCH)                    \
    F("id",              STRING,            NULL,  RO)                               \
    F("status",          STRING,            NULL,  PATCH)                            \
    F("organizationId",  STRING,            NULL,  RO)                               \
    F("pictureUrl",      STRING,            NULL,  RO)                               \
    F("created",         DATETIME,          NULL,  RO)                               \
    F("picture",         STRING,            NULL,  PATCH)

#endif /* STARKBANK_WORKSPACE_H */
