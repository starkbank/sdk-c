/*
 * The tables, printed as JSON, for the Python tools that must not re-implement
 * any of this.
 *
 * tools/drift.py needs three things this is the only honest source for: the
 * field tables as the preprocessor actually expanded them (not as a regex read
 * the X-macros), the endpoint starkcore derives from a resource name at run
 * time, and core-c's snake-to-camel rule applied by core-c itself. A Python
 * copy of any of the three would be a fourth source of truth that can rot, and
 * the whole point of the drift checker is that it compares real artifacts.
 *
 * Build-only. It is not installed, not exported and not part of the ABI; it
 * links the library's own objects and reaches into starkc/internal.h, which no
 * consumer may do.
 */

#include <stdio.h>
#include <string.h>

#include "internal.h"

static void printJsonString(const char *text)
{
    const unsigned char *cursor;

    if (text == NULL) {
        fputs("null", stdout);
        return;
    }
    putchar('"');
    for (cursor = (const unsigned char *)text; *cursor != '\0'; cursor++) {
        if (*cursor == '"' || *cursor == '\\') {
            printf("\\%c", (char)*cursor);
            continue;
        }
        if (*cursor < 0x20) {
            printf("\\u%04x", (unsigned int)*cursor);
            continue;
        }
        putchar((char)*cursor);
    }
    putchar('"');
}

/* Printed as null rather than "" when core-c refuses the name, so a resource
   whose endpoint cannot be derived is a visible drift finding and not an empty
   string that compares equal to an absent docs path. */
static void printDerived(const char *label, int (*derive)(const char *, char **),
                         const char *resourceName)
{
    char *value = NULL;

    printf("      \"%s\": ", label);
    if (derive(resourceName, &value) != STARKCORE_OK) {
        fputs("null", stdout);
        return;
    }
    printJsonString(value);
    starkcore_free(value);
}

static void printResource(const starkbankResource *resource)
{
    int index;

    printf("    {\n      \"name\": ");
    printJsonString(resource->name);
    printf(",\n");
    printDerived("endpoint", starkcore_api_endpoint, resource->name);
    printf(",\n");
    printDerived("lastName", starkcore_api_last_name, resource->name);
    printf(",\n");
    printDerived("lastNamePlural", starkcore_api_last_name_plural, resource->name);
    printf(",\n      \"polymorphic\": %s,\n", resource->refResolve != NULL ? "true" : "false");

    printf("      \"queryKeys\": [");
    if (resource->queryKeys != NULL) {
        for (index = 0; resource->queryKeys[index] != NULL; index++) {
            if (index > 0) {
                fputs(", ", stdout);
            }
            printJsonString(resource->queryKeys[index]);
        }
    }
    printf("],\n      \"fields\": [\n");
    for (index = 0; index < resource->fieldCount; index++) {
        const starkbankField *field = &resource->fields[index];

        printf("        {\"key\": ");
        printJsonString(field->key);
        printf(", \"type\": %d, \"flags\": %d, \"ref\": ", field->type, field->flags);
        printJsonString(field->ref);
        printf("}%s\n", index + 1 < resource->fieldCount ? "," : "");
    }
    printf("      ]\n    }");
}

static int printTables(void)
{
    int count = starkbank_resource_count();
    int index;

    printf("{\n  \"version\": ");
    printJsonString(starkbank_version());
    printf(",\n  \"resources\": [\n");
    for (index = 0; index < count; index++) {
        const starkbankResource *resource = starkbankRegistryFind(starkbank_resource_name_at(index));

        if (resource == NULL) {
            fprintf(stderr, "reflect: registry index %d does not resolve\n", index);
            return 1;
        }
        printResource(resource);
        printf("%s\n", index + 1 < count ? "," : "");
    }
    printf("  ]\n}\n");
    return 0;
}

/* One name per line in, one camelCased name per line out, through core-c's own
   converter: drift.py compares python's snake_case attribute names with the
   tables' wire keys and must use the rule the library uses, not a lookalike. */
static int printCamel(void)
{
    char line[256];

    while (fgets(line, (int)sizeof(line), stdin) != NULL) {
        char *converted = NULL;
        size_t length = strlen(line);

        while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
            line[--length] = '\0';
        }
        if (length == 0) {
            continue;
        }
        if (starkcore_case_snake_to_camel(line, &converted) != STARKCORE_OK) {
            fprintf(stderr, "reflect: cannot convert %s\n", line);
            return 1;
        }
        printf("%s\n", converted);
        starkcore_free(converted);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--camel") == 0) {
        return printCamel();
    }
    if (argc > 1) {
        fprintf(stderr, "usage: reflect [--camel]\n");
        return 2;
    }
    return printTables();
}
