/*
 * One strerror for both tiers. A caller compares against STARKCORE_OK and
 * switches on STARKCORE_ERROR_* exactly as it would one tier down, so the
 * codes this tier adds sit below -100 and everything else is forwarded
 * unchanged rather than remapped - a remapped code is a code a caller cannot
 * match against the starkcore documentation.
 */

#include "internal.h"

STARKBANK_API const char * STARKBANK_CALL starkbank_strerror(int code)
{
    switch (code) {
    case STARKBANK_ERROR_FIELD:
        return "no such field on this resource, or not writable here";
    case STARKBANK_ERROR_TYPE:
        return "the field exists but this accessor is the wrong one for its type";
    case STARKBANK_ERROR_ABSENT:
        return "the field is valid and this object does not carry a value for it";
    case STARKBANK_ERROR_MASKED:
        return "the value was redacted by the server";
    case STARKBANK_ERROR_RESOURCE:
        return "the entity belongs to another resource";
    case STARKBANK_ERROR_ABI:
        return "the linked starkcore ABI is not the one this library was built against";
    default:
        return starkcore_strerror(code);
    }
}
