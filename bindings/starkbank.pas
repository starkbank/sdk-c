unit starkbank;

{ Delphi binding for libstarkbank. Emitted from include/starkbank.h by
  tools/emit.py; edit the header, never this file.

  Handles are opaque pointers and nothing about their layout crosses the
  boundary. Every string returned is borrowed from the entity or list it
  came from and must not be freed; every buffer the library allocates is
  released with starkbank_free and never with FreeMem. }

interface

const
{$IFDEF MSWINDOWS}
  StarkbankLib = 'starkbank.dll';
{$ENDIF}
{$IFDEF LINUX}
  StarkbankLib = 'libstarkbank.so.1';
{$ENDIF}
{$IFDEF MACOS}
  StarkbankLib = 'libstarkbank.dylib';
{$ENDIF}

  STARKBANK_ABI_VERSION = 1;
  STARKBANK_BALANCE_AMOUNT = 'amount';
  STARKBANK_BALANCE_CURRENCY = 'currency';
  STARKBANK_BALANCE_ID = 'id';
  STARKBANK_BALANCE_UPDATED = 'updated';
  STARKBANK_BOLETO_AMOUNT = 'amount';
  STARKBANK_BOLETO_BAR_CODE = 'barCode';
  STARKBANK_BOLETO_CITY = 'city';
  STARKBANK_BOLETO_CREATED = 'created';
  STARKBANK_BOLETO_DESCRIPTIONS = 'descriptions';
  STARKBANK_BOLETO_DISCOUNTS = 'discounts';
  STARKBANK_BOLETO_DISTRICT = 'district';
  STARKBANK_BOLETO_DUE = 'due';
  STARKBANK_BOLETO_FEE = 'fee';
  STARKBANK_BOLETO_FINE = 'fine';
  STARKBANK_BOLETO_ID = 'id';
  STARKBANK_BOLETO_INTEREST = 'interest';
  STARKBANK_BOLETO_LINE = 'line';
  STARKBANK_BOLETO_LOG_BOLETO = 'boleto';
  STARKBANK_BOLETO_LOG_CREATED = 'created';
  STARKBANK_BOLETO_LOG_ERRORS = 'errors';
  STARKBANK_BOLETO_LOG_ID = 'id';
  STARKBANK_BOLETO_LOG_TYPE = 'type';
  STARKBANK_BOLETO_NAME = 'name';
  STARKBANK_BOLETO_OUR_NUMBER = 'ourNumber';
  STARKBANK_BOLETO_OVERDUE_LIMIT = 'overdueLimit';
  STARKBANK_BOLETO_PAYMENT_AMOUNT = 'amount';
  STARKBANK_BOLETO_PAYMENT_BAR_CODE = 'barCode';
  STARKBANK_BOLETO_PAYMENT_CREATED = 'created';
  STARKBANK_BOLETO_PAYMENT_DESCRIPTION = 'description';
  STARKBANK_BOLETO_PAYMENT_FEE = 'fee';
  STARKBANK_BOLETO_PAYMENT_ID = 'id';
  STARKBANK_BOLETO_PAYMENT_LINE = 'line';
  STARKBANK_BOLETO_PAYMENT_LOG_CREATED = 'created';
  STARKBANK_BOLETO_PAYMENT_LOG_ERRORS = 'errors';
  STARKBANK_BOLETO_PAYMENT_LOG_ID = 'id';
  STARKBANK_BOLETO_PAYMENT_LOG_PAYMENT = 'payment';
  STARKBANK_BOLETO_PAYMENT_LOG_TYPE = 'type';
  STARKBANK_BOLETO_PAYMENT_SCHEDULED = 'scheduled';
  STARKBANK_BOLETO_PAYMENT_STATUS = 'status';
  STARKBANK_BOLETO_PAYMENT_STATUS_CANCELED = 'canceled';
  STARKBANK_BOLETO_PAYMENT_STATUS_CONFIRMED = 'confirmed';
  STARKBANK_BOLETO_PAYMENT_STATUS_CREATED = 'created';
  STARKBANK_BOLETO_PAYMENT_STATUS_FAILED = 'failed';
  STARKBANK_BOLETO_PAYMENT_STATUS_PROCESSING = 'processing';
  STARKBANK_BOLETO_PAYMENT_STATUS_SUCCESS = 'success';
  STARKBANK_BOLETO_PAYMENT_TAGS = 'tags';
  STARKBANK_BOLETO_PAYMENT_TAX_ID = 'taxId';
  STARKBANK_BOLETO_PAYMENT_TRANSACTION_IDS = 'transactionIds';
  STARKBANK_BOLETO_PREVIEW_AMOUNT = 'amount';
  STARKBANK_BOLETO_PREVIEW_BAR_CODE = 'barCode';
  STARKBANK_BOLETO_PREVIEW_DISCOUNT_AMOUNT = 'discountAmount';
  STARKBANK_BOLETO_PREVIEW_DUE = 'due';
  STARKBANK_BOLETO_PREVIEW_EXPIRATION = 'expiration';
  STARKBANK_BOLETO_PREVIEW_FINE_AMOUNT = 'fineAmount';
  STARKBANK_BOLETO_PREVIEW_INTEREST_AMOUNT = 'interestAmount';
  STARKBANK_BOLETO_PREVIEW_LINE = 'line';
  STARKBANK_BOLETO_PREVIEW_NAME = 'name';
  STARKBANK_BOLETO_PREVIEW_PAYER_NAME = 'payerName';
  STARKBANK_BOLETO_PREVIEW_PAYER_TAX_ID = 'payerTaxId';
  STARKBANK_BOLETO_PREVIEW_RECEIVER_NAME = 'receiverName';
  STARKBANK_BOLETO_PREVIEW_RECEIVER_TAX_ID = 'receiverTaxId';
  STARKBANK_BOLETO_PREVIEW_STATUS = 'status';
  STARKBANK_BOLETO_PREVIEW_TAX_ID = 'taxId';
  STARKBANK_BOLETO_RECEIVER_NAME = 'receiverName';
  STARKBANK_BOLETO_RECEIVER_TAX_ID = 'receiverTaxId';
  STARKBANK_BOLETO_STATE_CODE = 'stateCode';
  STARKBANK_BOLETO_STATUS = 'status';
  STARKBANK_BOLETO_STATUS_CANCELED = 'canceled';
  STARKBANK_BOLETO_STATUS_CREATED = 'created';
  STARKBANK_BOLETO_STATUS_OVERDUE = 'overdue';
  STARKBANK_BOLETO_STATUS_PAID = 'paid';
  STARKBANK_BOLETO_STATUS_REGISTERED = 'registered';
  STARKBANK_BOLETO_STREET_LINE_1 = 'streetLine1';
  STARKBANK_BOLETO_STREET_LINE_2 = 'streetLine2';
  STARKBANK_BOLETO_TAGS = 'tags';
  STARKBANK_BOLETO_TAX_ID = 'taxId';
  STARKBANK_BOLETO_TRANSACTION_IDS = 'transactionIds';
  STARKBANK_BOLETO_WORKSPACE_ID = 'workspaceId';
  STARKBANK_BOLETO_ZIP_CODE = 'zipCode';
  STARKBANK_BRCODE_PAYMENT_AMOUNT = 'amount';
  STARKBANK_BRCODE_PAYMENT_BRCODE = 'brcode';
  STARKBANK_BRCODE_PAYMENT_CREATED = 'created';
  STARKBANK_BRCODE_PAYMENT_DESCRIPTION = 'description';
  STARKBANK_BRCODE_PAYMENT_FEE = 'fee';
  STARKBANK_BRCODE_PAYMENT_ID = 'id';
  STARKBANK_BRCODE_PAYMENT_LOG_CREATED = 'created';
  STARKBANK_BRCODE_PAYMENT_LOG_ERRORS = 'errors';
  STARKBANK_BRCODE_PAYMENT_LOG_ID = 'id';
  STARKBANK_BRCODE_PAYMENT_LOG_PAYMENT = 'payment';
  STARKBANK_BRCODE_PAYMENT_LOG_TYPE = 'type';
  STARKBANK_BRCODE_PAYMENT_NAME = 'name';
  STARKBANK_BRCODE_PAYMENT_RULES = 'rules';
  STARKBANK_BRCODE_PAYMENT_RULE_KEY = 'key';
  STARKBANK_BRCODE_PAYMENT_RULE_VALUE = 'value';
  STARKBANK_BRCODE_PAYMENT_SCHEDULED = 'scheduled';
  STARKBANK_BRCODE_PAYMENT_STATUS = 'status';
  STARKBANK_BRCODE_PAYMENT_STATUS_CANCELED = 'canceled';
  STARKBANK_BRCODE_PAYMENT_STATUS_CONFIRMED = 'confirmed';
  STARKBANK_BRCODE_PAYMENT_STATUS_CREATED = 'created';
  STARKBANK_BRCODE_PAYMENT_STATUS_FAILED = 'failed';
  STARKBANK_BRCODE_PAYMENT_STATUS_PROCESSING = 'processing';
  STARKBANK_BRCODE_PAYMENT_STATUS_SUCCESS = 'success';
  STARKBANK_BRCODE_PAYMENT_TAGS = 'tags';
  STARKBANK_BRCODE_PAYMENT_TAX_ID = 'taxId';
  STARKBANK_BRCODE_PAYMENT_TRANSACTION_IDS = 'transactionIds';
  STARKBANK_BRCODE_PAYMENT_TYPE = 'type';
  STARKBANK_BRCODE_PAYMENT_UPDATED = 'updated';
  STARKBANK_BRCODE_PREVIEW_ACCOUNT_TYPE = 'accountType';
  STARKBANK_BRCODE_PREVIEW_ALLOW_CHANGE = 'allowChange';
  STARKBANK_BRCODE_PREVIEW_AMOUNT = 'amount';
  STARKBANK_BRCODE_PREVIEW_BANK_CODE = 'bankCode';
  STARKBANK_BRCODE_PREVIEW_DESCRIPTION = 'description';
  STARKBANK_BRCODE_PREVIEW_DISCOUNT_AMOUNT = 'discountAmount';
  STARKBANK_BRCODE_PREVIEW_FINE_AMOUNT = 'fineAmount';
  STARKBANK_BRCODE_PREVIEW_INTEREST_AMOUNT = 'interestAmount';
  STARKBANK_BRCODE_PREVIEW_NAME = 'name';
  STARKBANK_BRCODE_PREVIEW_NOMINAL_AMOUNT = 'nominalAmount';
  STARKBANK_BRCODE_PREVIEW_RECONCILIATION_ID = 'reconciliationId';
  STARKBANK_BRCODE_PREVIEW_REDUCTION_AMOUNT = 'reductionAmount';
  STARKBANK_BRCODE_PREVIEW_STATUS = 'status';
  STARKBANK_BRCODE_PREVIEW_TAX_ID = 'taxId';
  STARKBANK_ENVIRONMENT_PRODUCTION = 0;
  STARKBANK_ENVIRONMENT_SANDBOX = 1;
  STARKBANK_ERROR_ABI = -106;
  STARKBANK_ERROR_ABSENT = -103;
  STARKBANK_ERROR_FIELD = -101;
  STARKBANK_ERROR_MASKED = -104;
  STARKBANK_ERROR_RESOURCE = -105;
  STARKBANK_ERROR_TYPE = -102;
  STARKBANK_EVENT_ATTEMPT_CODE = 'code';
  STARKBANK_EVENT_ATTEMPT_CREATED = 'created';
  STARKBANK_EVENT_ATTEMPT_EVENT_ID = 'eventId';
  STARKBANK_EVENT_ATTEMPT_ID = 'id';
  STARKBANK_EVENT_ATTEMPT_MESSAGE = 'message';
  STARKBANK_EVENT_ATTEMPT_WEBHOOK_ID = 'webhookId';
  STARKBANK_EVENT_CREATED = 'created';
  STARKBANK_EVENT_ID = 'id';
  STARKBANK_EVENT_IS_DELIVERED = 'isDelivered';
  STARKBANK_EVENT_LOG = 'log';
  STARKBANK_EVENT_SUBSCRIPTION = 'subscription';
  STARKBANK_EVENT_SUBSCRIPTION_BOLETO = 'boleto';
  STARKBANK_EVENT_SUBSCRIPTION_BOLETO_PAYMENT = 'boleto-payment';
  STARKBANK_EVENT_SUBSCRIPTION_BRCODE_PAYMENT = 'brcode-payment';
  STARKBANK_EVENT_SUBSCRIPTION_DARF_PAYMENT = 'darf-payment';
  STARKBANK_EVENT_SUBSCRIPTION_DEPOSIT = 'deposit';
  STARKBANK_EVENT_SUBSCRIPTION_HOLMES = 'holmes';
  STARKBANK_EVENT_SUBSCRIPTION_INVOICE = 'invoice';
  STARKBANK_EVENT_SUBSCRIPTION_TAX_PAYMENT = 'tax-payment';
  STARKBANK_EVENT_SUBSCRIPTION_TRANSFER = 'transfer';
  STARKBANK_EVENT_SUBSCRIPTION_UTILITY_PAYMENT = 'utility-payment';
  STARKBANK_EVENT_WORKSPACE_ID = 'workspaceId';
  STARKBANK_FIELD_AMOUNT = 1;
  STARKBANK_FIELD_BOOL = 5;
  STARKBANK_FIELD_DATE = 6;
  STARKBANK_FIELD_DATETIME = 7;
  STARKBANK_FIELD_DATE_OR_DATETIME = 8;
  STARKBANK_FIELD_LIST_OBJECT = 10;
  STARKBANK_FIELD_LIST_RESOURCE = 11;
  STARKBANK_FIELD_LIST_STRING = 9;
  STARKBANK_FIELD_NUMBER = 4;
  STARKBANK_FIELD_OBJECT = 13;
  STARKBANK_FIELD_RATE = 2;
  STARKBANK_FIELD_RESOURCE = 12;
  STARKBANK_FIELD_SECONDS = 3;
  STARKBANK_FIELD_STRING = 0;
  STARKBANK_FLAG_CREATE = 2;
  STARKBANK_FLAG_PATCH = 4;
  STARKBANK_FLAG_REQUIRED = 1;
  STARKBANK_INVOICE_AMOUNT = 'amount';
  STARKBANK_INVOICE_BRCODE = 'brcode';
  STARKBANK_INVOICE_CREATED = 'created';
  STARKBANK_INVOICE_DESCRIPTIONS = 'descriptions';
  STARKBANK_INVOICE_DISCOUNTS = 'discounts';
  STARKBANK_INVOICE_DISCOUNT_AMOUNT = 'discountAmount';
  STARKBANK_INVOICE_DUE = 'due';
  STARKBANK_INVOICE_EXPIRATION = 'expiration';
  STARKBANK_INVOICE_FEE = 'fee';
  STARKBANK_INVOICE_FINE = 'fine';
  STARKBANK_INVOICE_FINE_AMOUNT = 'fineAmount';
  STARKBANK_INVOICE_ID = 'id';
  STARKBANK_INVOICE_INTEREST = 'interest';
  STARKBANK_INVOICE_INTEREST_AMOUNT = 'interestAmount';
  STARKBANK_INVOICE_LINK = 'link';
  STARKBANK_INVOICE_LOG_CREATED = 'created';
  STARKBANK_INVOICE_LOG_ERRORS = 'errors';
  STARKBANK_INVOICE_LOG_ID = 'id';
  STARKBANK_INVOICE_LOG_INVOICE = 'invoice';
  STARKBANK_INVOICE_LOG_TYPE = 'type';
  STARKBANK_INVOICE_NAME = 'name';
  STARKBANK_INVOICE_NOMINAL_AMOUNT = 'nominalAmount';
  STARKBANK_INVOICE_PAYMENT_ACCOUNT_NUMBER = 'accountNumber';
  STARKBANK_INVOICE_PAYMENT_ACCOUNT_TYPE = 'accountType';
  STARKBANK_INVOICE_PAYMENT_AMOUNT = 'amount';
  STARKBANK_INVOICE_PAYMENT_BANK_CODE = 'bankCode';
  STARKBANK_INVOICE_PAYMENT_BRANCH_CODE = 'branchCode';
  STARKBANK_INVOICE_PAYMENT_END_TO_END_ID = 'endToEndId';
  STARKBANK_INVOICE_PAYMENT_METHOD = 'method';
  STARKBANK_INVOICE_PAYMENT_NAME = 'name';
  STARKBANK_INVOICE_PAYMENT_TAX_ID = 'taxId';
  STARKBANK_INVOICE_PDF = 'pdf';
  STARKBANK_INVOICE_RULES = 'rules';
  STARKBANK_INVOICE_RULE_KEY = 'key';
  STARKBANK_INVOICE_RULE_VALUE = 'value';
  STARKBANK_INVOICE_SPLITS = 'splits';
  STARKBANK_INVOICE_STATUS = 'status';
  STARKBANK_INVOICE_STATUS_CANCELED = 'canceled';
  STARKBANK_INVOICE_STATUS_CREATED = 'created';
  STARKBANK_INVOICE_STATUS_EXPIRED = 'expired';
  STARKBANK_INVOICE_STATUS_OVERDUE = 'overdue';
  STARKBANK_INVOICE_STATUS_PAID = 'paid';
  STARKBANK_INVOICE_STATUS_REGISTERED = 'registered';
  STARKBANK_INVOICE_STATUS_REVERSED = 'reversed';
  STARKBANK_INVOICE_TAGS = 'tags';
  STARKBANK_INVOICE_TAX_ID = 'taxId';
  STARKBANK_INVOICE_TRANSACTION_IDS = 'transactionIds';
  STARKBANK_INVOICE_UPDATED = 'updated';
  STARKBANK_LANGUAGE_EN_US = 0;
  STARKBANK_LANGUAGE_PT_BR = 1;
  STARKBANK_METHOD_DELETE = 4;
  STARKBANK_METHOD_GET = 0;
  STARKBANK_METHOD_PATCH = 3;
  STARKBANK_METHOD_POST = 1;
  STARKBANK_METHOD_PUT = 2;
  STARKBANK_OK = 0;
  STARKBANK_PAYMENT_PREVIEW_ID = 'id';
  STARKBANK_PAYMENT_PREVIEW_PAYMENT = 'payment';
  STARKBANK_PAYMENT_PREVIEW_SCHEDULED = 'scheduled';
  STARKBANK_PAYMENT_PREVIEW_TYPE = 'type';
  STARKBANK_PAYMENT_PREVIEW_TYPE_BOLETO_PAYMENT = 'boleto-payment';
  STARKBANK_PAYMENT_PREVIEW_TYPE_BRCODE_PAYMENT = 'brcode-payment';
  STARKBANK_PAYMENT_PREVIEW_TYPE_TAX_PAYMENT = 'tax-payment';
  STARKBANK_PAYMENT_PREVIEW_TYPE_UTILITY_PAYMENT = 'utility-payment';
  STARKBANK_SPLIT_AMOUNT = 'amount';
  STARKBANK_SPLIT_CREATED = 'created';
  STARKBANK_SPLIT_EXTERNAL_ID = 'externalId';
  STARKBANK_SPLIT_ID = 'id';
  STARKBANK_SPLIT_RECEIVER_ID = 'receiverId';
  STARKBANK_SPLIT_SCHEDULED = 'scheduled';
  STARKBANK_SPLIT_SOURCE = 'source';
  STARKBANK_SPLIT_STATUS = 'status';
  STARKBANK_SPLIT_TAGS = 'tags';
  STARKBANK_SPLIT_UPDATED = 'updated';
  STARKBANK_TAX_PREVIEW_AMOUNT = 'amount';
  STARKBANK_TAX_PREVIEW_BAR_CODE = 'barCode';
  STARKBANK_TAX_PREVIEW_DESCRIPTION = 'description';
  STARKBANK_TAX_PREVIEW_LINE = 'line';
  STARKBANK_TAX_PREVIEW_NAME = 'name';
  STARKBANK_TRANSFER_ACCOUNT_NUMBER = 'accountNumber';
  STARKBANK_TRANSFER_ACCOUNT_TYPE = 'accountType';
  STARKBANK_TRANSFER_ACCOUNT_TYPE_CHECKING = 'checking';
  STARKBANK_TRANSFER_ACCOUNT_TYPE_PAYMENT = 'payment';
  STARKBANK_TRANSFER_ACCOUNT_TYPE_SALARY = 'salary';
  STARKBANK_TRANSFER_ACCOUNT_TYPE_SAVINGS = 'savings';
  STARKBANK_TRANSFER_AMOUNT = 'amount';
  STARKBANK_TRANSFER_BANK_CODE = 'bankCode';
  STARKBANK_TRANSFER_BRANCH_CODE = 'branchCode';
  STARKBANK_TRANSFER_CREATED = 'created';
  STARKBANK_TRANSFER_DESCRIPTION = 'description';
  STARKBANK_TRANSFER_DISPLAY_DESCRIPTION = 'displayDescription';
  STARKBANK_TRANSFER_EXTERNAL_ID = 'externalId';
  STARKBANK_TRANSFER_FEE = 'fee';
  STARKBANK_TRANSFER_ID = 'id';
  STARKBANK_TRANSFER_LOG_CREATED = 'created';
  STARKBANK_TRANSFER_LOG_ERRORS = 'errors';
  STARKBANK_TRANSFER_LOG_ID = 'id';
  STARKBANK_TRANSFER_LOG_TRANSFER = 'transfer';
  STARKBANK_TRANSFER_LOG_TYPE = 'type';
  STARKBANK_TRANSFER_METADATA = 'metadata';
  STARKBANK_TRANSFER_NAME = 'name';
  STARKBANK_TRANSFER_RULES = 'rules';
  STARKBANK_TRANSFER_RULE_KEY = 'key';
  STARKBANK_TRANSFER_RULE_VALUE = 'value';
  STARKBANK_TRANSFER_SCHEDULED = 'scheduled';
  STARKBANK_TRANSFER_STATUS = 'status';
  STARKBANK_TRANSFER_STATUS_CANCELED = 'canceled';
  STARKBANK_TRANSFER_STATUS_CREATED = 'created';
  STARKBANK_TRANSFER_STATUS_FAILED = 'failed';
  STARKBANK_TRANSFER_STATUS_PROCESSING = 'processing';
  STARKBANK_TRANSFER_STATUS_SUCCESS = 'success';
  STARKBANK_TRANSFER_TAGS = 'tags';
  STARKBANK_TRANSFER_TAX_ID = 'taxId';
  STARKBANK_TRANSFER_TRANSACTION_IDS = 'transactionIds';
  STARKBANK_TRANSFER_UPDATED = 'updated';
  STARKBANK_UTILITY_PREVIEW_AMOUNT = 'amount';
  STARKBANK_UTILITY_PREVIEW_BAR_CODE = 'barCode';
  STARKBANK_UTILITY_PREVIEW_DESCRIPTION = 'description';
  STARKBANK_UTILITY_PREVIEW_LINE = 'line';
  STARKBANK_UTILITY_PREVIEW_NAME = 'name';
  STARKBANK_VERSION = '0.1.0';
  STARKBANK_WEBHOOK_ID = 'id';
  STARKBANK_WEBHOOK_SUBSCRIPTIONS = 'subscriptions';
  STARKBANK_WEBHOOK_SUBSCRIPTION_BOLETO = 'boleto';
  STARKBANK_WEBHOOK_SUBSCRIPTION_BOLETO_HOLMES = 'boleto-holmes';
  STARKBANK_WEBHOOK_SUBSCRIPTION_BOLETO_PAYMENT = 'boleto-payment';
  STARKBANK_WEBHOOK_SUBSCRIPTION_BRCODE_PAYMENT = 'brcode-payment';
  STARKBANK_WEBHOOK_SUBSCRIPTION_DARF_PAYMENT = 'darf-payment';
  STARKBANK_WEBHOOK_SUBSCRIPTION_DEPOSIT = 'deposit';
  STARKBANK_WEBHOOK_SUBSCRIPTION_INVOICE = 'invoice';
  STARKBANK_WEBHOOK_SUBSCRIPTION_PAYMENT_REQUEST = 'payment-request';
  STARKBANK_WEBHOOK_SUBSCRIPTION_TAX_PAYMENT = 'tax-payment';
  STARKBANK_WEBHOOK_SUBSCRIPTION_TRANSFER = 'transfer';
  STARKBANK_WEBHOOK_SUBSCRIPTION_UTILITY_PAYMENT = 'utility-payment';
  STARKBANK_WEBHOOK_URL = 'url';

type
  { One machine word. Declared distinctly so a Transfer handle cannot be
    passed where an Invoice entity is wanted by accident. }
  Pstarkbank_client = type Pointer;
  Pstarkbank_entity = type Pointer;
  Pstarkbank_errors = type Pointer;
  Pstarkbank_headers = type Pointer;
  Pstarkbank_iter = type Pointer;
  Pstarkbank_list = type Pointer;
  Pstarkbank_response = type Pointer;
  Pstarkbank_user = type Pointer;
  Pstarkcore_json = type Pointer;
  PPstarkbank_client = ^Pstarkbank_client;
  PPstarkbank_entity = ^Pstarkbank_entity;
  PPstarkbank_errors = ^Pstarkbank_errors;
  PPstarkbank_headers = ^Pstarkbank_headers;
  PPstarkbank_iter = ^Pstarkbank_iter;
  PPstarkbank_list = ^Pstarkbank_list;
  PPstarkbank_response = ^Pstarkbank_response;
  PPstarkbank_user = ^Pstarkbank_user;
  PPstarkcore_json = ^Pstarkcore_json;

  TStarkbankTransport = function(context: Pointer; method: Integer;
    url: PAnsiChar; headers: Pstarkbank_headers; body: PAnsiChar;
    bodyLen: NativeUInt; timeoutSeconds: Integer;
    out response: Pstarkbank_response): Integer; cdecl;

function starkbank_abi_version: Integer; cdecl; external StarkbankLib name 'starkbank_abi_version';

function starkbank_balance_get(client: Pstarkbank_client; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_balance_get';

function starkbank_boleto_create(client: Pstarkbank_client; boletos: Pstarkbank_list; &out: PPstarkbank_list; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_create';

function starkbank_boleto_delete(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_delete';

function starkbank_boleto_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_get';

function starkbank_boleto_log_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_log_get';

function starkbank_boleto_log_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_log_page';

function starkbank_boleto_log_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_log_params_new';

function starkbank_boleto_log_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_log_query';

function starkbank_boleto_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_new';

function starkbank_boleto_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_page';

function starkbank_boleto_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_params_new';

function starkbank_boleto_payment_create(client: Pstarkbank_client; payments: Pstarkbank_list; &out: PPstarkbank_list; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_payment_create';

function starkbank_boleto_payment_delete(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_payment_delete';

function starkbank_boleto_payment_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_payment_get';

function starkbank_boleto_payment_log_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_payment_log_get';

function starkbank_boleto_payment_log_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_payment_log_page';

function starkbank_boleto_payment_log_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_payment_log_params_new';

function starkbank_boleto_payment_log_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_payment_log_query';

function starkbank_boleto_payment_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_payment_new';

function starkbank_boleto_payment_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_payment_page';

function starkbank_boleto_payment_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_payment_params_new';

function starkbank_boleto_payment_pdf(client: Pstarkbank_client; id: PAnsiChar; &out: PPByte; out_len: PNativeUInt; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_payment_pdf';

function starkbank_boleto_payment_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_payment_query';

function starkbank_boleto_pdf(client: Pstarkbank_client; id: PAnsiChar; layout: PAnsiChar; hidden_fields: PAnsiChar; &out: PPByte; out_len: PNativeUInt; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_pdf';

function starkbank_boleto_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_boleto_query';

function starkbank_brcode_payment_create(client: Pstarkbank_client; payments: Pstarkbank_list; &out: PPstarkbank_list; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_create';

function starkbank_brcode_payment_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_get';

function starkbank_brcode_payment_log_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_log_get';

function starkbank_brcode_payment_log_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_log_page';

function starkbank_brcode_payment_log_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_log_params_new';

function starkbank_brcode_payment_log_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_log_query';

function starkbank_brcode_payment_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_new';

function starkbank_brcode_payment_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_page';

function starkbank_brcode_payment_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_params_new';

function starkbank_brcode_payment_pdf(client: Pstarkbank_client; id: PAnsiChar; &out: PPByte; out_len: PNativeUInt; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_pdf';

function starkbank_brcode_payment_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_query';

function starkbank_brcode_payment_rule_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_rule_new';

function starkbank_brcode_payment_update(client: Pstarkbank_client; id: PAnsiChar; patch: Pstarkbank_entity; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_brcode_payment_update';

procedure starkbank_client_cache_clear(client: Pstarkbank_client); cdecl; external StarkbankLib name 'starkbank_client_cache_clear';

procedure starkbank_client_free(client: Pstarkbank_client); cdecl; external StarkbankLib name 'starkbank_client_free';

function starkbank_client_new(user: Pstarkbank_user; &out: PPstarkbank_client): Integer; cdecl; external StarkbankLib name 'starkbank_client_new';

function starkbank_client_set_curl_transport(client: Pstarkbank_client): Integer; cdecl; external StarkbankLib name 'starkbank_client_set_curl_transport';

function starkbank_client_set_language(client: Pstarkbank_client; language: Integer): Integer; cdecl; external StarkbankLib name 'starkbank_client_set_language';

function starkbank_client_set_max_response_size(client: Pstarkbank_client; bytes: NativeUInt): Integer; cdecl; external StarkbankLib name 'starkbank_client_set_max_response_size';

function starkbank_client_set_timeout(client: Pstarkbank_client; seconds: Integer): Integer; cdecl; external StarkbankLib name 'starkbank_client_set_timeout';

function starkbank_client_set_transport(client: Pstarkbank_client; transport: TStarkbankTransport; context: Pointer): Integer; cdecl; external StarkbankLib name 'starkbank_client_set_transport';

function starkbank_client_set_user(client: Pstarkbank_client; user: Pstarkbank_user): Integer; cdecl; external StarkbankLib name 'starkbank_client_set_user';

function starkbank_client_set_user_agent_prefix(client: Pstarkbank_client; prefix: PAnsiChar): Integer; cdecl; external StarkbankLib name 'starkbank_client_set_user_agent_prefix';

function starkbank_core_version: PAnsiChar; cdecl; external StarkbankLib name 'starkbank_core_version';

function starkbank_entity_amount(entity: Pstarkbank_entity; field: PAnsiChar; &out: PDouble): Integer; cdecl; external StarkbankLib name 'starkbank_entity_amount';

function starkbank_entity_append_entity(entity: Pstarkbank_entity; field: PAnsiChar; value: Pstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_entity_append_entity';

function starkbank_entity_append_string(entity: Pstarkbank_entity; field: PAnsiChar; value: PAnsiChar): Integer; cdecl; external StarkbankLib name 'starkbank_entity_append_string';

function starkbank_entity_bool(entity: Pstarkbank_entity; field: PAnsiChar; &out: PInteger): Integer; cdecl; external StarkbankLib name 'starkbank_entity_bool';

function starkbank_entity_clone(entity: Pstarkbank_entity; &out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_entity_clone';

function starkbank_entity_datetime(entity: Pstarkbank_entity; field: PAnsiChar; year: PInteger; month: PInteger; day: PInteger; hour: PInteger; minute: PInteger; second: PInteger; out_has_time: PInteger): Integer; cdecl; external StarkbankLib name 'starkbank_entity_datetime';

function starkbank_entity_dump(entity: Pstarkbank_entity; &out: PPAnsiChar; out_len: PNativeUInt): Integer; cdecl; external StarkbankLib name 'starkbank_entity_dump';

function starkbank_entity_entity(entity: Pstarkbank_entity; field: PAnsiChar; &out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_entity_entity';

procedure starkbank_entity_free(entity: Pstarkbank_entity); cdecl; external StarkbankLib name 'starkbank_entity_free';

function starkbank_entity_has(entity: Pstarkbank_entity; field: PAnsiChar): Integer; cdecl; external StarkbankLib name 'starkbank_entity_has';

function starkbank_entity_id(entity: Pstarkbank_entity): PAnsiChar; cdecl; external StarkbankLib name 'starkbank_entity_id';

function starkbank_entity_json(entity: Pstarkbank_entity): Pstarkcore_json; cdecl; external StarkbankLib name 'starkbank_entity_json';

function starkbank_entity_list_entity_at(entity: Pstarkbank_entity; field: PAnsiChar; index: Integer; &out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_entity_list_entity_at';

function starkbank_entity_list_size(entity: Pstarkbank_entity; field: PAnsiChar; &out: PInteger): Integer; cdecl; external StarkbankLib name 'starkbank_entity_list_size';

function starkbank_entity_list_string_at(entity: Pstarkbank_entity; field: PAnsiChar; index: Integer; &out: PPAnsiChar): Integer; cdecl; external StarkbankLib name 'starkbank_entity_list_string_at';

function starkbank_entity_number(entity: Pstarkbank_entity; field: PAnsiChar; &out: PDouble): Integer; cdecl; external StarkbankLib name 'starkbank_entity_number';

function starkbank_entity_resource(entity: Pstarkbank_entity): PAnsiChar; cdecl; external StarkbankLib name 'starkbank_entity_resource';

function starkbank_entity_set_amount(entity: Pstarkbank_entity; field: PAnsiChar; cents: Double): Integer; cdecl; external StarkbankLib name 'starkbank_entity_set_amount';

function starkbank_entity_set_bool(entity: Pstarkbank_entity; field: PAnsiChar; value: Integer): Integer; cdecl; external StarkbankLib name 'starkbank_entity_set_bool';

function starkbank_entity_set_date(entity: Pstarkbank_entity; field: PAnsiChar; year: Integer; month: Integer; day: Integer): Integer; cdecl; external StarkbankLib name 'starkbank_entity_set_date';

function starkbank_entity_set_datetime(entity: Pstarkbank_entity; field: PAnsiChar; year: Integer; month: Integer; day: Integer; hour: Integer; minute: Integer; second: Integer): Integer; cdecl; external StarkbankLib name 'starkbank_entity_set_datetime';

function starkbank_entity_set_json_raw(entity: Pstarkbank_entity; field: PAnsiChar; json_text: PAnsiChar): Integer; cdecl; external StarkbankLib name 'starkbank_entity_set_json_raw';

function starkbank_entity_set_number(entity: Pstarkbank_entity; field: PAnsiChar; value: Double): Integer; cdecl; external StarkbankLib name 'starkbank_entity_set_number';

function starkbank_entity_set_seconds(entity: Pstarkbank_entity; field: PAnsiChar; seconds: Integer): Integer; cdecl; external StarkbankLib name 'starkbank_entity_set_seconds';

function starkbank_entity_set_string(entity: Pstarkbank_entity; field: PAnsiChar; value: PAnsiChar): Integer; cdecl; external StarkbankLib name 'starkbank_entity_set_string';

function starkbank_entity_string(entity: Pstarkbank_entity; field: PAnsiChar; &out: PPAnsiChar): Integer; cdecl; external StarkbankLib name 'starkbank_entity_string';

function starkbank_entity_unknown_count(entity: Pstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_entity_unknown_count';

function starkbank_errors_code_at(errors: Pstarkbank_errors; index: Integer): PAnsiChar; cdecl; external StarkbankLib name 'starkbank_errors_code_at';

function starkbank_errors_count(errors: Pstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_errors_count';

procedure starkbank_errors_free(errors: Pstarkbank_errors); cdecl; external StarkbankLib name 'starkbank_errors_free';

function starkbank_errors_message_at(errors: Pstarkbank_errors; index: Integer): PAnsiChar; cdecl; external StarkbankLib name 'starkbank_errors_message_at';

function starkbank_event_attempt_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_event_attempt_get';

function starkbank_event_attempt_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_event_attempt_page';

function starkbank_event_attempt_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_event_attempt_params_new';

function starkbank_event_attempt_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_event_attempt_query';

function starkbank_event_delete(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_event_delete';

function starkbank_event_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_event_get';

function starkbank_event_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_event_page';

function starkbank_event_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_event_params_new';

function starkbank_event_parse(client: Pstarkbank_client; content: PAnsiChar; content_len: NativeUInt; signature_base64: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_event_parse';

function starkbank_event_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_event_query';

function starkbank_event_update(client: Pstarkbank_client; id: PAnsiChar; patch: Pstarkbank_entity; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_event_update';

procedure starkbank_free(pointer: Pointer); cdecl; external StarkbankLib name 'starkbank_free';

function starkbank_headers_count(headers: Pstarkbank_headers): Integer; cdecl; external StarkbankLib name 'starkbank_headers_count';

function starkbank_headers_name_at(headers: Pstarkbank_headers; index: Integer): PAnsiChar; cdecl; external StarkbankLib name 'starkbank_headers_name_at';

function starkbank_headers_value_at(headers: Pstarkbank_headers; index: Integer): PAnsiChar; cdecl; external StarkbankLib name 'starkbank_headers_value_at';

function starkbank_invoice_create(client: Pstarkbank_client; invoices: Pstarkbank_list; &out: PPstarkbank_list; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_create';

function starkbank_invoice_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_get';

function starkbank_invoice_log_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_log_get';

function starkbank_invoice_log_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_log_page';

function starkbank_invoice_log_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_log_params_new';

function starkbank_invoice_log_pdf(client: Pstarkbank_client; id: PAnsiChar; &out: PPByte; out_len: PNativeUInt; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_log_pdf';

function starkbank_invoice_log_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_log_query';

function starkbank_invoice_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_new';

function starkbank_invoice_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_page';

function starkbank_invoice_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_params_new';

function starkbank_invoice_payment(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_payment';

function starkbank_invoice_pdf(client: Pstarkbank_client; id: PAnsiChar; &out: PPByte; out_len: PNativeUInt; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_pdf';

function starkbank_invoice_qrcode(client: Pstarkbank_client; id: PAnsiChar; size: Integer; &out: PPByte; out_len: PNativeUInt; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_qrcode';

function starkbank_invoice_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_query';

function starkbank_invoice_rule_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_rule_new';

function starkbank_invoice_update(client: Pstarkbank_client; id: PAnsiChar; patch: Pstarkbank_entity; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_invoice_update';

function starkbank_iter_cursor(iter: Pstarkbank_iter): PAnsiChar; cdecl; external StarkbankLib name 'starkbank_iter_cursor';

procedure starkbank_iter_free(iter: Pstarkbank_iter); cdecl; external StarkbankLib name 'starkbank_iter_free';

function starkbank_iter_next(iter: Pstarkbank_iter; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_iter_next';

function starkbank_list_append(list: Pstarkbank_list; entity: Pstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_list_append';

function starkbank_list_at(list: Pstarkbank_list; index: Integer): Pstarkbank_entity; cdecl; external StarkbankLib name 'starkbank_list_at';

function starkbank_list_count(list: Pstarkbank_list): Integer; cdecl; external StarkbankLib name 'starkbank_list_count';

procedure starkbank_list_free(list: Pstarkbank_list); cdecl; external StarkbankLib name 'starkbank_list_free';

function starkbank_list_new(&out: PPstarkbank_list): Integer; cdecl; external StarkbankLib name 'starkbank_list_new';

function starkbank_object_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_object_new';

function starkbank_organization_new(id: PAnsiChar; environment: Integer; private_key_pem: PAnsiChar; workspace_id: PAnsiChar; &out: PPstarkbank_user): Integer; cdecl; external StarkbankLib name 'starkbank_organization_new';

function starkbank_organization_replace(organization: Pstarkbank_user; workspace_id: PAnsiChar; &out: PPstarkbank_user): Integer; cdecl; external StarkbankLib name 'starkbank_organization_replace';

function starkbank_parse_and_verify(client: Pstarkbank_client; content: PAnsiChar; content_len: NativeUInt; signature_base64: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_parse_and_verify';

function starkbank_payment_preview_create(client: Pstarkbank_client; previews: Pstarkbank_list; &out: PPstarkbank_list; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_payment_preview_create';

function starkbank_payment_preview_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_payment_preview_new';

function starkbank_project_new(id: PAnsiChar; environment: Integer; private_key_pem: PAnsiChar; &out: PPstarkbank_user): Integer; cdecl; external StarkbankLib name 'starkbank_project_new';

function starkbank_resource_count: Integer; cdecl; external StarkbankLib name 'starkbank_resource_count';

function starkbank_resource_field_count(resource: PAnsiChar): Integer; cdecl; external StarkbankLib name 'starkbank_resource_field_count';

function starkbank_resource_field_flags_at(resource: PAnsiChar; index: Integer): Integer; cdecl; external StarkbankLib name 'starkbank_resource_field_flags_at';

function starkbank_resource_field_name_at(resource: PAnsiChar; index: Integer): PAnsiChar; cdecl; external StarkbankLib name 'starkbank_resource_field_name_at';

function starkbank_resource_field_type_at(resource: PAnsiChar; index: Integer): Integer; cdecl; external StarkbankLib name 'starkbank_resource_field_type_at';

function starkbank_resource_name_at(index: Integer): PAnsiChar; cdecl; external StarkbankLib name 'starkbank_resource_name_at';

function starkbank_response_new(status: Integer; content: PByte; content_len: NativeUInt; headers: Pstarkbank_headers; &out: PPstarkbank_response): Integer; cdecl; external StarkbankLib name 'starkbank_response_new';

function starkbank_split_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_split_new';

function starkbank_strerror(code: Integer): PAnsiChar; cdecl; external StarkbankLib name 'starkbank_strerror';

function starkbank_transfer_create(client: Pstarkbank_client; transfers: Pstarkbank_list; &out: PPstarkbank_list; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_create';

function starkbank_transfer_delete(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_delete';

function starkbank_transfer_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_get';

function starkbank_transfer_log_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_log_get';

function starkbank_transfer_log_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_log_page';

function starkbank_transfer_log_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_log_params_new';

function starkbank_transfer_log_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_log_query';

function starkbank_transfer_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_new';

function starkbank_transfer_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_page';

function starkbank_transfer_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_params_new';

function starkbank_transfer_pdf(client: Pstarkbank_client; id: PAnsiChar; &out: PPByte; out_len: PNativeUInt; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_pdf';

function starkbank_transfer_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_query';

function starkbank_transfer_rule_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_transfer_rule_new';

function starkbank_user_access_id(user: Pstarkbank_user): PAnsiChar; cdecl; external StarkbankLib name 'starkbank_user_access_id';

function starkbank_user_environment(user: Pstarkbank_user): Integer; cdecl; external StarkbankLib name 'starkbank_user_environment';

procedure starkbank_user_free(user: Pstarkbank_user); cdecl; external StarkbankLib name 'starkbank_user_free';

function starkbank_version: PAnsiChar; cdecl; external StarkbankLib name 'starkbank_version';

function starkbank_webhook_create(client: Pstarkbank_client; webhook: Pstarkbank_entity; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_webhook_create';

function starkbank_webhook_delete(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_webhook_delete';

function starkbank_webhook_get(client: Pstarkbank_client; id: PAnsiChar; &out: PPstarkbank_entity; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_webhook_get';

function starkbank_webhook_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_webhook_new';

function starkbank_webhook_page(client: Pstarkbank_client; params: Pstarkbank_entity; &out: PPstarkbank_list; out_cursor: PPAnsiChar; errors: PPstarkbank_errors): Integer; cdecl; external StarkbankLib name 'starkbank_webhook_page';

function starkbank_webhook_params_new(&out: PPstarkbank_entity): Integer; cdecl; external StarkbankLib name 'starkbank_webhook_params_new';

function starkbank_webhook_query(client: Pstarkbank_client; params: Pstarkbank_entity; limit: Integer; &out: PPstarkbank_iter): Integer; cdecl; external StarkbankLib name 'starkbank_webhook_query';


implementation

end.
