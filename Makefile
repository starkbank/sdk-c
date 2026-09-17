# starkbank sdk-c
#
#   make                static library
#   make shared         shared library exporting only starkbank_*
#   make bundle         one artifact with core-c, ecdsa-c and secp256k1 inside
#   make bundle-curl    the same, with core-c's libcurl transport linked in
#   make install        headers, libraries and starkbank.pc under PREFIX
#   make test           both suites, offline, with a fake transport
#   make test-engine    the engine suite over tests/fixtures' own resources
#   make test-slice     Invoice/Transfer/Event/Balance against the python goldens
#   make test-loose     both suites built with -DSTARKBANK_LOOSE_QUERY
#   make leaks          both suites under leaks (Darwin) or valgrind (Linux)
#   make asan           both suites under AddressSanitizer and UBSan
#   make expand R=<r>   preprocessed output for one resource, for macro debugging
#   make test-abi       the core-ABI guard, with starkcore_abi_version stubbed
#   make tsan           one client, eight threads, under ThreadSanitizer
#   make check-drift    the tables against sdk-python and the docs JSON
#   make check-goldens  re-records tests/reference/slice.json from sdk-python
#   make check-tools    the unit cases for the Python tools
#   make samples        compile every docs sample with the library's own flags
#   make samples-emit   regenerate the mechanical samples from the tables
#   make check-exports  every exported symbol of every artifact
#   make check-handwritten  the linker-enforced opt-out, both directions
#   make check-header   the header gate: C89, C++98, cffi, bindings
#   make bindings       regenerate bindings/ from include/starkbank.h
#   make clean
#
# core-c's Makefile discipline is copied deliberately, including keeping the
# mandatory flags apart from a user-facing CFLAGS: a command-line CFLAGS
# overrides every makefile assignment, and losing -fvisibility=hidden would
# widen the exported surface without anyone noticing.
#
# There is no HTTP and no TLS here either. The host supplies a transport, or
# links libstarkcore_curl.

CC ?= cc
CXX ?= c++
AR ?= ar

# One source for the version: the public header. cut and awk rather than sed,
# because make counts parentheses inside a $(shell) call and a sed capture
# group would close it early; the '#' is escaped because make starts a comment
# at an unescaped one even inside quotes.
VERSION := $(shell grep '^\#define STARKBANK_VERSION ' include/starkbank.h | cut -d'"' -f2)
ABI_VERSION := $(shell grep '^\#define STARKBANK_ABI_VERSION ' include/starkbank.h | awk '{print $$3}')

PREFIX ?= /usr/local
INCLUDEDIR ?= $(PREFIX)/include
LIBDIR ?= $(PREFIX)/lib
PKGCONFIGDIR ?= $(LIBDIR)/pkgconfig

GOLDEN = tests/reference/slice.json

# The two inputs the drift checker reads and never writes. Overridable so CI
# can point at the pinned checkouts rather than somebody's working tree.
SDK_PYTHON ?= ../sdk-python/starkbank
APP_DOCS ?= ../app-docs/src/uipages/api/bank/resources

CORE_PREFIX ?= $(shell \
	for p in ../../starkinfra/core-c /usr/local /usr; do \
		if [ -f "$$p/include/starkcore.h" ]; then echo $$p; break; fi; \
	done)
ECDSA_PREFIX ?= $(shell \
	for p in ../ecdsa-c /usr/local /usr; do \
		if [ -f "$$p/include/starkecdsa.h" ]; then echo $$p; break; fi; \
	done)
SECP256K1_PREFIX ?= $(shell \
	for p in /opt/homebrew/opt/secp256k1 /usr/local/opt/secp256k1 /usr/local /usr; do \
		if [ -f "$$p/include/secp256k1.h" ]; then echo $$p; break; fi; \
	done)

CORE_LIB ?= $(firstword $(wildcard $(CORE_PREFIX)/libstarkcore.a $(CORE_PREFIX)/lib/libstarkcore.a))
ECDSA_LIB ?= $(firstword $(wildcard $(ECDSA_PREFIX)/libstarkecdsa.a $(ECDSA_PREFIX)/lib/libstarkecdsa.a))
# Optional: core-c builds it only under `make curl`, and a bundle without it is
# a library with no HTTP, which is the supported POS shape rather than a fault.
CORE_CURL_LIB ?= $(firstword $(wildcard $(CORE_PREFIX)/libstarkcore_curl.a \
                                        $(CORE_PREFIX)/lib/libstarkcore_curl.a))
SECP256K1_LIB ?= $(firstword $(wildcard $(SECP256K1_PREFIX)/lib/libsecp256k1.a \
                                        $(SECP256K1_PREFIX)/libsecp256k1.a))

CFLAGS ?= -O2
TEST_CFLAGS ?= -O1 -g
REQUIRED_CFLAGS = -std=c99 -pedantic -Wall -Wextra -Wshadow -Wconversion -Wstrict-prototypes \
          -Wmissing-prototypes -Wpointer-arith -Wwrite-strings -Wcast-qual \
          -fvisibility=hidden -fPIC -MMD -MP
ALL_CFLAGS = $(REQUIRED_CFLAGS) $(CFLAGS)
CPPFLAGS += -Iinclude -Istarkc -Istarkbank -I$(CORE_PREFIX)/include \
            -I$(ECDSA_PREFIX)/include -I$(SECP256K1_PREFIX)/include
LDFLAGS += -L$(SECP256K1_PREFIX)/lib
LDLIBS += $(CORE_LIB) $(ECDSA_LIB) -lsecp256k1

ENGINE_SOURCES = \
	starkc/table.c \
	starkc/entity.c \
	starkc/list.c \
	starkc/iter.c \
	starkc/error.c \
	starkc/registry.c \
	starkc/verb.c \
	starkc/facade.c

# One line per resource, and nothing else changes when one is added: the
# registry is starkbank/resources.h and the verbs are in the file itself.
RESOURCE_SOURCES = \
	starkbank/balance/balance.c \
	starkbank/boleto/boleto.c \
	starkbank/boleto/log.c \
	starkbank/boletopayment/boletopayment.c \
	starkbank/boletopayment/log.c \
	starkbank/brcodepayment/brcodepayment.c \
	starkbank/brcodepayment/log.c \
	starkbank/brcodepayment/rule.c \
	starkbank/event/attempt.c \
	starkbank/event/event.c \
	starkbank/invoice/invoice.c \
	starkbank/invoice/log.c \
	starkbank/invoice/payment.c \
	starkbank/invoice/rule.c \
	starkbank/paymentpreview/boletopreview.c \
	starkbank/paymentpreview/brcodepreview.c \
	starkbank/paymentpreview/paymentpreview.c \
	starkbank/paymentpreview/taxpreview.c \
	starkbank/paymentpreview/utilitypreview.c \
	starkbank/split/split.c \
	starkbank/taxpayment/log.c \
	starkbank/taxpayment/taxpayment.c \
	starkbank/transfer/log.c \
	starkbank/transfer/rule.c \
	starkbank/transfer/transfer.c \
	starkbank/utilitypayment/log.c \
	starkbank/utilitypayment/utilitypayment.c \
	starkbank/webhook/webhook.c

# The judgement methods. A symbol declared in the public header that no macro
# expands must have exactly one definition here, and check-handwritten proves
# both halves of that.
# A wildcard on purpose. If this line enumerated the files, deleting one would
# be a "no rule to make target" and the design's claim - that a judgement
# method nobody wrote is an undefined symbol at link time - would be false.
# With the wildcard, the file simply is not built and the linker says which
# entry point is missing. check-handwritten guards the other direction: a
# stray definition in here that the header never declared.
HANDWRITTEN_SOURCES = $(wildcard handwritten/*.c)

SOURCES = $(ENGINE_SOURCES) $(RESOURCE_SOURCES) $(HANDWRITTEN_SOURCES)

TEST_SOURCES = tests/run.c tests/fixtures/testresources.c
# The suite links the engine from source rather than the archive: it reaches
# starkc/internal.h helpers directly, and a sanitizer build must instrument
# every line of the engine, not only the test.
# The engine suite keeps its own registry, so it links the engine only.
TEST_ALL_SOURCES = $(TEST_SOURCES) $(ENGINE_SOURCES)

# The slice suite is the real library plus its goldens, which is why it is a
# second binary: the registry is chosen at compile time and the two suites
# cannot share one.
SLICE_ALL_SOURCES = tests/slice.c $(ENGINE_SOURCES) $(RESOURCE_SOURCES) $(HANDWRITTEN_SOURCES)
SLICE_CPPFLAGS = $(CPPFLAGS) -DTEST_FIXTURE_DIR='"tests/fixtures"' 
TEST_CPPFLAGS = $(CPPFLAGS) -Itests/fixtures -DTEST_FIXTURE_DIR='"tests/fixtures"' \
                -DSTARKBANK_RESOURCES_HEADER='"testresources.h"'

OBJECTS = $(SOURCES:.c=.o)
DEPENDS = $(OBJECTS:.o=.d) libstarkbank.d tests/run.d tests/asan.d tests/slice.d

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
  SHARED_NAME = libstarkbank.dylib
  SHARED_FLAGS = -dynamiclib -install_name @rpath/$(@F) \
                 -current_version $(VERSION) -compatibility_version $(ABI_VERSION) \
                 -Wl,-exported_symbols_list,exports.txt
else
  SHARED_NAME = libstarkbank.so.$(ABI_VERSION)
  SHARED_FLAGS = -shared -Wl,-soname,$(SHARED_NAME) -Wl,--version-script,exports.map \
                 -Wl,--exclude-libs,ALL
  LDLIBS += -pthread
endif

# Ownership rule 1 hands out borrowed pointers everywhere and three entry
# points transfer ownership, so this is where the real bugs in this tier are.
ifeq ($(UNAME),Darwin)
  ASAN_ENV = MallocNanoZone=0
  LEAK_COMMAND = leaks --atExit -- ./tests/run && leaks --atExit -- ./tests/slice
else
  ASAN_ENV = ASAN_OPTIONS=detect_leaks=1
  LEAK_COMMAND = valgrind --leak-check=full --error-exitcode=1 \
                 --errors-for-leak-kinds=definite ./tests/run \
                 && valgrind --leak-check=full --error-exitcode=1 \
                 --errors-for-leak-kinds=definite ./tests/slice
endif

.PHONY: all shared bundle bundle-curl install uninstall test test-engine test-slice \
        test-abi test-loose test-threads asan tsan leaks expand reflect \
        check-exports check-handwritten check-header check-drift check-goldens \
        check-drift-pinned check-tools \
        samples samples-emit bindings print-ldflags clean version

all: libstarkbank.a

version:
	@echo "starkbank sdk-c $(VERSION), ABI $(ABI_VERSION), core-c at $(CORE_PREFIX)"

libstarkbank.a: $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

%.o: %.c
	$(CC) $(ALL_CFLAGS) $(CPPFLAGS) -c $< -o $@

# The release shared library. It links only when every entry point the header
# declares has a definition, which is the linker-enforced opt-out doing its
# job: delete handwritten/event_parse.c and this target is the thing that
# fails, with the symbol named.
shared: ALL_CFLAGS += -DSTARKBANK_BUILD_SHARED
shared: $(SOURCES) exports.txt exports.map
	$(CC) $(ALL_CFLAGS) $(CPPFLAGS) $(SHARED_FLAGS) -o $(SHARED_NAME) $(SOURCES) \
		$(LDFLAGS) $(LDLIBS)

# One artifact for a host that must name a single library: Delphi, .NET, and
# anything loading a DLL by name. core-c, ecdsa-c and secp256k1 go inside, and
# the export list keeps them there - a host that loads this must not find our
# starkcore on top of its own.
#
# The static half is a merged archive rather than four: `make print-ldflags`
# exists for the C/C++ shape, and this exists so nobody has to use it.
BUNDLE_ARCHIVES = $(CORE_LIB) $(ECDSA_LIB) $(SECP256K1_LIB)
ifeq ($(UNAME),Darwin)
  BUNDLE_SHARED_NAME = libstarkbank_full.dylib
  BUNDLE_CURL_SHARED_NAME = libstarkbank_full_curl.dylib
  MERGE = libtool -static -o
else
  BUNDLE_SHARED_NAME = libstarkbank_full.so.$(ABI_VERSION)
  BUNDLE_CURL_SHARED_NAME = libstarkbank_full_curl.so.$(ABI_VERSION)
endif

bundle: libstarkbank_full.a $(BUNDLE_SHARED_NAME)

# The same bundle with core-c's libcurl transport inside, so
# starkbank_client_set_curl_transport resolves instead of returning
# STARKCORE_ERROR_NO_TRANSPORT. Separate artifacts and not a flag on the same
# ones: a target-specific variable cannot change what an up-to-date file
# contains, and a bundle that silently has no transport in it is worse than
# one that does not exist.
bundle-curl: libstarkbank_full_curl.a $(BUNDLE_CURL_SHARED_NAME)

libstarkbank_full.a: $(OBJECTS)
	@rm -f $@
	$(call mergeArchive,$@,)

libstarkbank_full_curl.a: $(OBJECTS)
	@test -n "$(CORE_CURL_LIB)" || { echo "bundle-curl: build core-c with 'make curl' first" >&2; exit 1; }
	@rm -f $@
	$(call mergeArchive,$@,$(CORE_CURL_LIB))

# One recipe, two artifacts. $(1) is the archive, $(2) the extra library.
ifeq ($(UNAME),Darwin)
define mergeArchive
	$(MERGE) $(1) $(OBJECTS) $(2) $(BUNDLE_ARCHIVES)
endef
else
define mergeArchive
	@{ echo "create $(1)"; \
	   for object in $(OBJECTS); do echo "addmod $$object"; done; \
	   for archive in $(2) $(BUNDLE_ARCHIVES); do echo "addlib $$archive"; done; \
	   echo save; echo end; } | $(AR) -M
endef
endif

$(BUNDLE_SHARED_NAME): ALL_CFLAGS += -DSTARKBANK_BUILD_SHARED
$(BUNDLE_SHARED_NAME): $(SOURCES) exports.txt exports.map
	$(CC) $(ALL_CFLAGS) $(CPPFLAGS) $(SHARED_FLAGS) -o $@ $(SOURCES) $(BUNDLE_ARCHIVES)

$(BUNDLE_CURL_SHARED_NAME): ALL_CFLAGS += -DSTARKBANK_BUILD_SHARED
$(BUNDLE_CURL_SHARED_NAME): $(SOURCES) exports.txt exports.map
	@test -n "$(CORE_CURL_LIB)" || { echo "bundle-curl: build core-c with 'make curl' first" >&2; exit 1; }
	$(CC) $(ALL_CFLAGS) $(CPPFLAGS) $(SHARED_FLAGS) -o $@ $(SOURCES) \
		$(CORE_CURL_LIB) $(BUNDLE_ARCHIVES) -lcurl

# Nobody should have to guess the static link order. The archives are listed
# dependency-last because that is the only order a one-pass linker accepts.
print-ldflags:
	@echo "$(LDFLAGS) libstarkbank.a $(CORE_LIB) $(ECDSA_LIB) -lsecp256k1"

starkbank.pc: include/starkbank.h
	@printf 'prefix=%s\nexec_prefix=$${prefix}\nlibdir=%s\nincludedir=%s\n\n' \
		"$(PREFIX)" "$(LIBDIR)" "$(INCLUDEDIR)" > $@
	@printf 'Name: starkbank\nDescription: Stark Bank SDK, resource tier over starkcore\n' >> $@
	@printf 'Version: %s\nRequires: starkcore starkecdsa\n' "$(VERSION)" >> $@
	@printf 'Libs: -L$${libdir} -lstarkbank\nCflags: -I$${includedir}\n' >> $@

install: libstarkbank.a shared starkbank.pc
	install -d $(DESTDIR)$(INCLUDEDIR) $(DESTDIR)$(LIBDIR) $(DESTDIR)$(PKGCONFIGDIR)
	install -m 644 include/starkbank.h $(DESTDIR)$(INCLUDEDIR)/
	install -m 644 libstarkbank.a $(DESTDIR)$(LIBDIR)/
	install -m 755 $(SHARED_NAME) $(DESTDIR)$(LIBDIR)/
	install -m 644 starkbank.pc $(DESTDIR)$(PKGCONFIGDIR)/

uninstall:
	rm -f $(DESTDIR)$(INCLUDEDIR)/starkbank.h $(DESTDIR)$(LIBDIR)/libstarkbank.a \
	      $(DESTDIR)$(LIBDIR)/$(SHARED_NAME) $(DESTDIR)$(PKGCONFIGDIR)/starkbank.pc

# The suite compiles under the library's own warning set, not a softer one.
# tests/fixtures/testresources.c is a real resource file in every respect, so
# a macro that expands to warning-triggering code fails here rather than in
# step 3 across forty-three resources at once.
test: test-engine test-slice

test-engine: $(TEST_ALL_SOURCES)
	$(CC) $(REQUIRED_CFLAGS) $(TEST_CFLAGS) $(TEST_CPPFLAGS) \
		-o tests/run $(TEST_ALL_SOURCES) $(LDFLAGS) $(LDLIBS)
	./tests/run

# Invoice, Transfer, Event and Balance against tests/reference/slice.json,
# recorded from sdk-python. Regenerate it with
#   python3 tests/tools/record_fixtures.py
# which needs core-python and ecdsa-python importable and touches no network.
test-slice: $(SLICE_ALL_SOURCES) tests/reference/slice.json
	$(CC) $(REQUIRED_CFLAGS) $(TEST_CFLAGS) $(SLICE_CPPFLAGS) \
		-o tests/slice $(SLICE_ALL_SOURCES) $(LDFLAGS) $(LDLIBS)
	./tests/slice

# The core-ABI guard, which cannot be reached from inside a build that agrees
# with its starkcore: the call is macro-renamed to a stub that disagrees, so
# the guard is exercised rather than asserted. Its own binary, because the
# renamed facade.c would otherwise collide with the real one.
test-abi: tests/abi.c $(ENGINE_SOURCES) $(RESOURCE_SOURCES) $(HANDWRITTEN_SOURCES)
	$(CC) $(REQUIRED_CFLAGS) $(TEST_CFLAGS) $(SLICE_CPPFLAGS) \
		-Dstarkcore_abi_version=sliceFakeAbiVersion \
		-o tests/abi tests/abi.c $(ENGINE_SOURCES) $(RESOURCE_SOURCES) \
		$(HANDWRITTEN_SOURCES) $(LDFLAGS) $(LDLIBS)
	./tests/abi

# Borrowed pointers everywhere and three ownership transfers: the real bugs in
# this tier are here, not in the tables. A required CI job, not an option.
asan: $(TEST_ALL_SOURCES) $(SLICE_ALL_SOURCES)
	$(CC) $(REQUIRED_CFLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
		-fno-sanitize-recover=all $(TEST_CPPFLAGS) \
		-o tests/asan $(TEST_ALL_SOURCES) $(LDFLAGS) $(LDLIBS)
	$(ASAN_ENV) ./tests/asan
	$(CC) $(REQUIRED_CFLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
		-fno-sanitize-recover=all $(SLICE_CPPFLAGS) \
		-o tests/asanslice $(SLICE_ALL_SOURCES) $(LDFLAGS) $(LDLIBS)
	$(ASAN_ENV) ./tests/asanslice

# ASan on Darwin has no LeakSanitizer, so leaks are a second job with the
# platform's own tool rather than an unrun option nobody notices is off.
leaks: test
	$(LEAK_COMMAND)

# The tables as the preprocessor expanded them, the endpoints core-c derives
# from a resource name, and core-c's own casing rule, printed for the Python
# tools. It exists so drift.py compares real artifacts instead of a Python
# lookalike of three things core-c already owns.
reflect: tools/reflect

# The headers are prerequisites too, and they are the ones that matter: a field
# table lives in a .h, so a reflect built only against the .c files would let
# check-drift pass against a table that had already changed.
RESOURCE_HEADERS = $(wildcard starkbank/*/*.h) $(wildcard starkc/*.h) \
                   starkbank/resources.h include/starkbank.h

tools/reflect: tools/reflect.c $(ENGINE_SOURCES) $(RESOURCE_SOURCES) $(HANDWRITTEN_SOURCES) \
               $(RESOURCE_HEADERS)
	$(CC) $(REQUIRED_CFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ tools/reflect.c \
		$(ENGINE_SOURCES) $(RESOURCE_SOURCES) $(HANDWRITTEN_SOURCES) $(LDFLAGS) $(LDLIBS)

# The tables against sdk-python at the pinned sha and against the docs JSON.
# Red when the SET of accepted divergences in tests/reference/known-drift.json
# changes - in either direction - and never merely because it is non-empty.
# --require-pin is the release gate: a tag is not cut against an unknown
# upstream, but a developer with a branch checked out can still run this.
check-drift: tools/reflect
	python3 tools/drift.py --python=$(SDK_PYTHON) --docs=$(APP_DOCS)

check-drift-pinned: tools/reflect
	python3 tools/drift.py --python=$(SDK_PYTHON) --docs=$(APP_DOCS) --require-pin

# The Python tools are the only part of this repo a compiler does not check.
# slice.json carries the request bytes sdk-python emits for every payload in
# the slice, and nothing else re-derives it: check-drift reads the tables and
# the docs JSON and never runs the recorder. Without this target the goldens
# are a frozen file a contributor can hand-edit until tests/slice goes green.
#
# Needs core-python, ecdsa-python and sdk-python at the pin; touches no
# network. The re-recorded file is left in the tree on failure, because seeing
# the diff and committing it is the fix.
check-goldens:
	@cp $(GOLDEN) $(GOLDEN).committed
	@python3 tests/tools/record_fixtures.py
	@if cmp -s $(GOLDEN).committed $(GOLDEN); then \
		rm -f $(GOLDEN).committed; \
		echo "check-goldens: $(GOLDEN) is still what sdk-python emits"; \
	else \
		rm -f $(GOLDEN).committed; \
		echo "check-goldens: $(GOLDEN) is NOT what sdk-python emits;" >&2; \
		echo "  the re-recorded file is in your tree - diff it and commit it" >&2; \
		exit 1; \
	fi

check-tools:
	python3 tests/header/test_headerabi.py
	python3 tests/tools/test_drift.py
	python3 tests/tools/test_emit.py

# One client, eight threads, the fake transport. The header promises a
# configured client may be shared across threads including concurrent
# parse_and_verify, and nothing else in the suite is multi-threaded.
#
# Scope, honestly: this instruments sdk-c's own sources. libstarkcore.a is a
# prebuilt archive here, so a race inside core-c's key cache is core-c's own
# `make tsan` to catch, not this one. What this proves is that this tier adds
# no shared mutable state of its own.
test-threads: tests/threads.c $(ENGINE_SOURCES) $(RESOURCE_SOURCES) $(HANDWRITTEN_SOURCES)
	$(CC) $(REQUIRED_CFLAGS) $(TEST_CFLAGS) $(SLICE_CPPFLAGS) \
		-o tests/threads tests/threads.c $(ENGINE_SOURCES) $(RESOURCE_SOURCES) \
		$(HANDWRITTEN_SOURCES) $(LDFLAGS) $(LDLIBS) -pthread
	./tests/threads

tsan: tests/threads.c $(ENGINE_SOURCES) $(RESOURCE_SOURCES) $(HANDWRITTEN_SOURCES)
	$(CC) $(REQUIRED_CFLAGS) -O1 -g -fsanitize=thread $(SLICE_CPPFLAGS) \
		-o tests/tsan tests/threads.c $(ENGINE_SOURCES) $(RESOURCE_SOURCES) \
		$(HANDWRITTEN_SOURCES) $(LDFLAGS) $(LDLIBS) -pthread
	./tests/tsan

# Every docs sample is a program, compiled with the library's own warning set.
# No other language on that site can say that, and the golang create sample
# that does not compile is why it matters.
SAMPLE_SOURCES = $(wildcard samples/*.c)

samples: libstarkbank.a $(SAMPLE_SOURCES)
	@test -n "$(SAMPLE_SOURCES)" || { echo "samples: nothing emitted yet" >&2; exit 1; }
	@mkdir -p samples/build
	@for source in $(SAMPLE_SOURCES); do \
		out=samples/build/$$(basename $$source .c); \
		$(CC) $(REQUIRED_CFLAGS) $(CFLAGS) -Werror $(CPPFLAGS) -o $$out $$source \
			libstarkbank.a $(LDFLAGS) $(LDLIBS) || exit 1; \
	done
	@echo "samples: $(words $(SAMPLE_SOURCES)) compiled with -Werror"

samples-emit: tools/reflect
	python3 tools/emit.py --samples

# Derived from the public header, so the exported set cannot drift from the
# declared one. Matched against the STARKBANK_API lines only, never the whole
# file: the comment blocks name functions too, and a prose mention must not
# become an exported symbol.
exports.txt: include/starkbank.h
	grep '^STARKBANK_API' include/starkbank.h \
		| grep -oE 'starkbank_[a-z_0-9]+\(' | sed 's/($$//' | sort -u | sed 's/^/_/' > $@
	@test "$$(wc -l < $@)" -eq "$$(grep -c '^STARKBANK_API' include/starkbank.h)" \
		|| { echo "exports.txt: a declaration was missed or duplicated" >&2; exit 1; }

exports.map: include/starkbank.h
	printf 'STARKBANK_1 {\n  global: starkbank_*;\n  local: *;\n};\n' > $@

# Two checks, because they catch different things. The shared library must
# export only starkbank_*, or a host loading it gets our starkcore on top of
# its own. The archive is checked against the whole starkbank namespace, not
# the underscore spelling: an archive member has to give external linkage to
# every cross-file helper, so starkbankFieldFind is legitimately there. What
# must never appear in either is a bare starkcore_ or cJSON_ symbol we
# re-exported by linking a dependency in.
check-exports: libstarkbank.a shared
	@nm -g libstarkbank.a 2>/dev/null | awk '$$2 ~ /^[A-TV-Z]$$/ {print $$3}' \
		| sed 's/^_//' | grep -v '^starkbank' | sort -u > exports.unexpected || true
	@if [ -s exports.unexpected ]; then \
		echo "unexpected external symbols in libstarkbank.a:"; cat exports.unexpected; exit 1; fi
	@if [ "$(UNAME)" = "Darwin" ]; then nm -gU $(SHARED_NAME) | awk '{print $$3}' | sed 's/^_//'; \
	else nm -D --defined-only $(SHARED_NAME) | awk '$$2 != "A" {print $$3}' | sed 's/@@.*//'; fi \
	| grep -v '^starkbank_' > exports.unexpected || true
	@if [ -s exports.unexpected ]; then \
		echo "unexpected exports in $(SHARED_NAME):"; cat exports.unexpected; exit 1; fi
	@rm -f exports.unexpected
	@for bundle in $(BUNDLE_SHARED_NAME) $(BUNDLE_CURL_SHARED_NAME); do \
		test -f $$bundle || continue; \
		if [ "$(UNAME)" = "Darwin" ]; then nm -gU $$bundle | awk '{print $$3}' | sed 's/^_//'; \
		else nm -D --defined-only $$bundle | awk '$$2 != "A" {print $$3}' | sed 's/@@.*//'; fi \
		| grep -v '^starkbank_' > exports.unexpected || true; \
		if [ -s exports.unexpected ]; then \
			echo "unexpected exports in $$bundle:"; cat exports.unexpected; exit 1; fi; \
		echo "check-exports: $$bundle exports only starkbank_* with the dependencies inside"; \
	done
	@rm -f exports.unexpected
	@archive=$$(nm -g libstarkbank.a 2>/dev/null | awk '$$2 ~ /^[A-TV-Z]$$/ {print $$3}' \
		| sed 's/^_//' | grep -c '^starkbank'); \
	exported=$$(if [ "$(UNAME)" = "Darwin" ]; then nm -gU $(SHARED_NAME); \
		else nm -D --defined-only $(SHARED_NAME); fi | grep -c 'starkbank_'); \
	echo "check-exports: $$archive external symbols in libstarkbank.a, all starkbank*;" \
		"$$exported exported from $(SHARED_NAME), all starkbank_*"

# The one mitigation for macro-expanded verb bodies: when a -Werror diagnostic
# or a breakpoint lands on verbs.h, this is how you read what the table
# actually produced. Until step 3 adds starkbank/<resource>/, the only
# expandable file is the suite's own resource table, which is authored the
# same way.
expand:
	@test -n "$(R)" || { echo 'usage: make expand R=<resource>'; exit 1; }
	@source=starkbank/$(R)/$(R).c; \
	if [ ! -f "$$source" ]; then \
		source=tests/fixtures/testresources.c; \
		echo "/* no starkbank/$(R)/$(R).c yet; expanding $$source */"; \
	fi; \
	$(CC) -std=c99 $(TEST_CPPFLAGS) -E -P "$$source"

# The query-strictness knob, built and run so the loosening is a tested path
# and not a macro nobody has compiled. Writes stay strict in this build too:
# a wrong filter costs a retry where a wrong write costs money.
test-loose: $(TEST_ALL_SOURCES) $(SLICE_ALL_SOURCES)
	$(CC) $(REQUIRED_CFLAGS) $(TEST_CFLAGS) $(TEST_CPPFLAGS) -DSTARKBANK_LOOSE_QUERY \
		-o tests/loose $(TEST_ALL_SOURCES) $(LDFLAGS) $(LDLIBS)
	./tests/loose
	$(CC) $(REQUIRED_CFLAGS) $(TEST_CFLAGS) $(SLICE_CPPFLAGS) -DSTARKBANK_LOOSE_QUERY \
		-o tests/looseslice $(SLICE_ALL_SOURCES) $(LDFLAGS) $(LDLIBS)
	./tests/looseslice

# Every symbol the public header declares that no macro expands must have
# exactly one definition under handwritten/. Both halves matter: a missing
# definition is already a link error, but a symbol quietly defined in two
# places, or a hand-written file nobody declared, is not.
#
# The objects are the ones the normal build produces, beside their sources.
# Compiling them into one directory would collapse invoice/log.o onto
# transfer/log.o and silently lose a resource's worth of symbols.
EXPANDED_OBJECTS = $(ENGINE_SOURCES:.c=.o) $(RESOURCE_SOURCES:.c=.o)
HANDWRITTEN_OBJECTS = $(HANDWRITTEN_SOURCES:.c=.o)

check-handwritten: exports.txt $(EXPANDED_OBJECTS) $(HANDWRITTEN_OBJECTS)
	@nm -g $(EXPANDED_OBJECTS) | awk '$$2 ~ /^[A-TV-Z]$$/ {print $$3}' | sed 's/^_//' \
		| grep '^starkbank_' | sort -u > handwritten.expanded
	@nm -g $(HANDWRITTEN_OBJECTS) | awk '$$2 ~ /^[A-TV-Z]$$/ {print $$3}' | sed 's/^_//' \
		| grep '^starkbank_' | sort > handwritten.byhand
	@sed 's/^_//' exports.txt | sort > handwritten.declared
	@comm -23 handwritten.declared handwritten.expanded > handwritten.missing
	@if ! diff -q handwritten.missing handwritten.byhand >/dev/null; then \
		echo "check-handwritten: declared-but-unexpanded and handwritten/ disagree"; \
		echo "  < declared with no macro and no hand-written definition"; \
		echo "  > defined by hand but not declared, or also expanded"; \
		diff handwritten.missing handwritten.byhand; \
		rm -f handwritten.*; exit 1; fi
	@if [ "$$(sort -u handwritten.byhand | wc -l)" -ne "$$(wc -l < handwritten.byhand)" ]; then \
		echo "check-handwritten: a symbol is defined twice under handwritten/"; \
		rm -f handwritten.*; exit 1; fi
	@echo "check-handwritten: $$(wc -l < handwritten.byhand | tr -d ' ') hand-written," \
		"$$(wc -l < handwritten.expanded | tr -d ' ') expanded," \
		"$$(wc -l < handwritten.declared | tr -d ' ') declared"
	@rm -f handwritten.expanded handwritten.byhand handwritten.declared handwritten.missing

# tools/reflect first: the gate's last step is emit.py --check, which now also
# regenerates the samples and therefore needs the tables.
check-header: tools/reflect
	CC="$(CC)" CXX="$(CXX)" CORE_PREFIX="$(CORE_PREFIX)" sh tools/gate.sh

bindings:
	python3 tools/emit.py

clean:
	rm -f $(OBJECTS) $(DEPENDS) libstarkbank.a $(SHARED_NAME) libstarkbank.so.*
	rm -f exports.txt exports.map exports.defined exports.unexpected starkbank.pc
	rm -f libstarkbank_full*.a libstarkbank_full*.dylib libstarkbank_full*.so.* libstarkbank_full*.d
	rm -f tools/reflect tools/reflect.o
	rm -f handwritten.expanded handwritten.byhand handwritten.declared handwritten.missing
	rm -f tests/threads tests/tsan tests/run tests/asan tests/asanslice tests/slice tests/abi tests/loose tests/looseslice
	rm -f tests/fixtures/*.o tests/*.d
	rm -rf samples/build tests/threads.dSYM tests/tsan.dSYM tests/run.dSYM tests/asan.dSYM tests/asanslice.dSYM tests/slice.dSYM \
	       tests/abi.dSYM tests/loose.dSYM tests/looseslice.dSYM

-include $(DEPENDS)
