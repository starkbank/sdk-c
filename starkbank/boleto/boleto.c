#include "../../starkc/verbs.h"

#include "boleto.h"

static const char *const boletoQuery[] = {
    "limit", "after", "before", "status", "tags", "ids", NULL
};

STARKBANK_RESOURCE(boleto, "Boleto", STARKBANK_BOLETO_FIELDS, boletoQuery);

STARKBANK_VERB_NEW(boleto)
STARKBANK_VERB_PARAMS(boleto)
STARKBANK_VERB_POST_MULTI(boleto)
STARKBANK_VERB_GET_ID(boleto)
STARKBANK_VERB_DELETE_ID(boleto)
STARKBANK_VERB_QUERY(boleto)
STARKBANK_VERB_PAGE(boleto)
STARKBANK_VERB_CONTENT_QUERY(boleto, pdf, "layout", "hiddenFields")
