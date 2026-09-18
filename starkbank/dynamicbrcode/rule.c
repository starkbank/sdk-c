/* Registered under the qualified tag "DynamicBrcode.Rule": sdk-python's
   dynamicbrcode/rule/__rule.py assigns its dict to _sub_resource (not
   _resource) under the "Rule" bare name, and tools/drift.py's readPython
   qualifies every _sub_resource by the resource name of the package that
   owns it - the same mechanism that already makes Invoice's own Rule
   "Invoice.Rule" and Transfer's "Transfer.Rule". */

#include "../../starkc/verbs.h"

#include "dynamicbrcode.h"

STARKBANK_RESOURCE(dynamic_brcode_rule, "DynamicBrcode.Rule",
                   STARKBANK_DYNAMIC_BRCODE_RULE_FIELDS, NULL);

STARKBANK_VERB_NEW(dynamic_brcode_rule)
