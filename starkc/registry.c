/*
 * Resource name to table. Two callers: hydration, which needs the table for a
 * RESOURCE or LIST_RESOURCE field, and the reflected registry, which is how a
 * Delphi or .NET host regenerates its own wrappers on its own schedule instead
 * of waiting for a release of ours.
 *
 * The list itself is starkbank/resources.h, expanded twice. The header is
 * overridable so the suite can register its own resources without a second
 * copy of this file; nothing else overrides it.
 */

#include <string.h>

#include "internal.h"

#ifndef STARKBANK_RESOURCES_HEADER
#  define STARKBANK_RESOURCES_HEADER "resources.h"
#endif

#include STARKBANK_RESOURCES_HEADER

#define STARKBANK_REGISTRY_DECLARE(ident) extern const starkbankResource starkbankTable_##ident;
#define STARKBANK_REGISTRY_ENTRY(ident)   &starkbankTable_##ident,

STARKBANK_RESOURCES(STARKBANK_REGISTRY_DECLARE)

/* The trailing NULL keeps the array non-empty while the registry is, which C
   requires and which costs one pointer. */
static const starkbankResource *const registry[] = {
    STARKBANK_RESOURCES(STARKBANK_REGISTRY_ENTRY)
    NULL
};

static const int registryCount = (int)(sizeof(registry) / sizeof(registry[0])) - 1;

const starkbankResource * starkbankRegistryFind(const char *name)
{
    int index;

    if (name == NULL) {
        return NULL;
    }
    for (index = 0; index < registryCount; index++) {
        if (strcmp(registry[index]->name, name) == 0) {
            return registry[index];
        }
    }
    return NULL;
}

static const starkbankField * fieldAt(const char *resourceName, int index)
{
    const starkbankResource *resource = starkbankRegistryFind(resourceName);

    if (resource == NULL || index < 0 || index >= resource->fieldCount) {
        return NULL;
    }
    return &resource->fields[index];
}

STARKBANK_API int STARKBANK_CALL starkbank_resource_count(void)
{
    return registryCount;
}

STARKBANK_API const char * STARKBANK_CALL starkbank_resource_name_at(int index)
{
    if (index < 0 || index >= registryCount) {
        return NULL;
    }
    return registry[index]->name;
}

STARKBANK_API int STARKBANK_CALL starkbank_resource_field_count(const char *resource)
{
    const starkbankResource *found = starkbankRegistryFind(resource);

    if (found == NULL) {
        return -1;
    }
    return found->fieldCount;
}

STARKBANK_API const char * STARKBANK_CALL starkbank_resource_field_name_at(const char *resource,
                                                                          int index)
{
    const starkbankField *field = fieldAt(resource, index);

    if (field == NULL) {
        return NULL;
    }
    return field->key;
}

STARKBANK_API int STARKBANK_CALL starkbank_resource_field_type_at(const char *resource, int index)
{
    const starkbankField *field = fieldAt(resource, index);

    if (field == NULL) {
        return -1;
    }
    return field->type;
}

STARKBANK_API int STARKBANK_CALL starkbank_resource_field_flags_at(const char *resource, int index)
{
    const starkbankField *field = fieldAt(resource, index);

    if (field == NULL) {
        return -1;
    }
    return field->flags;
}
