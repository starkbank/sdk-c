/* Bare sub-resource: sdk-python's corporateholder.__permission.py assigns its
   dict to _resource (not _sub_resource) and names it plain "Permission", so
   this table registers as "Permission" - a top-level bare name, ident
   "permission" - the same way Split's own _resource makes it "Split" rather
   than "Invoice.Split", even though the only place a caller meets one today
   is CorporateHolder.permissions. It lives in this directory because that is
   the only family that embeds it, not because the registry says so. */

#include "../../starkc/verbs.h"

#include "corporateholder.h"

STARKBANK_RESOURCE(permission, "Permission", STARKBANK_PERMISSION_FIELDS, NULL);

STARKBANK_VERB_NEW(permission)
