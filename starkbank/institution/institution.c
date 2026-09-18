#include "../../starkc/verbs.h"

#include "institution.h"

static const char *const institutionQuery[] = {
    "limit", "search", "spiCodes", "strCodes", NULL
};

STARKBANK_RESOURCE(institution, "Institution", STARKBANK_INSTITUTION_FIELDS, institutionQuery);

STARKBANK_VERB_PARAMS(institution)
STARKBANK_VERB_PAGE(institution)
