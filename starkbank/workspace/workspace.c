#include "../../starkc/verbs.h"

#include "workspace.h"

static const char *const workspaceQuery[] = {
    "limit", "username", "ids", NULL
};

/* sdk-python's workspace.update is the one patch_id caller in the whole SDK
   that also echoes patched keys into the URL's query string (username and
   name; core-python's rest.patch_id forwards **query, and update() is the
   only call site that passes any). See STARKBANK_VERB_PATCH_ID_ECHO. */
static const char *const workspaceEchoQuery[] = {
    STARKBANK_WORKSPACE_USERNAME, STARKBANK_WORKSPACE_NAME, NULL
};

STARKBANK_RESOURCE(workspace, "Workspace", STARKBANK_WORKSPACE_FIELDS, workspaceQuery);

STARKBANK_VERB_NEW(workspace)
STARKBANK_VERB_PARAMS(workspace)
STARKBANK_VERB_POST_SINGLE(workspace)
STARKBANK_VERB_GET_ID(workspace)
STARKBANK_VERB_QUERY(workspace)
STARKBANK_VERB_PAGE(workspace)
STARKBANK_VERB_PATCH_ID_ECHO(workspace, workspaceEchoQuery)
