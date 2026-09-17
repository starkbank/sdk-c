// C# binding for libstarkbank. Emitted from include/starkbank.h by
// tools/emit.py; edit the header, never this file.
//
// Handles are IntPtr: this first cut maps the ABI exactly and hides
// nothing. Strings come back as a borrowed pointer into the entity that
// owns them, so Marshal.PtrToStringAnsi copies before the entity is freed;
// buffers the library allocated are released with StarkbankFree, never by
// the .NET runtime.

using System;
using System.Runtime.InteropServices;

namespace StarkBank
{
    public static partial class Native
    {
        private const string Library = "starkbank";

        public const int StarkbankAbiVersion = 1;
        public const string StarkbankBalanceAmount = "amount";
        public const string StarkbankBalanceCurrency = "currency";
        public const string StarkbankBalanceId = "id";
        public const string StarkbankBalanceUpdated = "updated";
        public const string StarkbankBoletoAmount = "amount";
        public const string StarkbankBoletoBarCode = "barCode";
        public const string StarkbankBoletoCity = "city";
        public const string StarkbankBoletoCreated = "created";
        public const string StarkbankBoletoDescriptions = "descriptions";
        public const string StarkbankBoletoDiscounts = "discounts";
        public const string StarkbankBoletoDistrict = "district";
        public const string StarkbankBoletoDue = "due";
        public const string StarkbankBoletoFee = "fee";
        public const string StarkbankBoletoFine = "fine";
        public const string StarkbankBoletoId = "id";
        public const string StarkbankBoletoInterest = "interest";
        public const string StarkbankBoletoLine = "line";
        public const string StarkbankBoletoLogBoleto = "boleto";
        public const string StarkbankBoletoLogCreated = "created";
        public const string StarkbankBoletoLogErrors = "errors";
        public const string StarkbankBoletoLogId = "id";
        public const string StarkbankBoletoLogType = "type";
        public const string StarkbankBoletoName = "name";
        public const string StarkbankBoletoOurNumber = "ourNumber";
        public const string StarkbankBoletoOverdueLimit = "overdueLimit";
        public const string StarkbankBoletoPaymentAmount = "amount";
        public const string StarkbankBoletoPaymentBarCode = "barCode";
        public const string StarkbankBoletoPaymentCreated = "created";
        public const string StarkbankBoletoPaymentDescription = "description";
        public const string StarkbankBoletoPaymentFee = "fee";
        public const string StarkbankBoletoPaymentId = "id";
        public const string StarkbankBoletoPaymentLine = "line";
        public const string StarkbankBoletoPaymentLogCreated = "created";
        public const string StarkbankBoletoPaymentLogErrors = "errors";
        public const string StarkbankBoletoPaymentLogId = "id";
        public const string StarkbankBoletoPaymentLogPayment = "payment";
        public const string StarkbankBoletoPaymentLogType = "type";
        public const string StarkbankBoletoPaymentScheduled = "scheduled";
        public const string StarkbankBoletoPaymentStatus = "status";
        public const string StarkbankBoletoPaymentStatusCanceled = "canceled";
        public const string StarkbankBoletoPaymentStatusConfirmed = "confirmed";
        public const string StarkbankBoletoPaymentStatusCreated = "created";
        public const string StarkbankBoletoPaymentStatusFailed = "failed";
        public const string StarkbankBoletoPaymentStatusProcessing = "processing";
        public const string StarkbankBoletoPaymentStatusSuccess = "success";
        public const string StarkbankBoletoPaymentTags = "tags";
        public const string StarkbankBoletoPaymentTaxId = "taxId";
        public const string StarkbankBoletoPaymentTransactionIds = "transactionIds";
        public const string StarkbankBoletoPreviewAmount = "amount";
        public const string StarkbankBoletoPreviewBarCode = "barCode";
        public const string StarkbankBoletoPreviewDiscountAmount = "discountAmount";
        public const string StarkbankBoletoPreviewDue = "due";
        public const string StarkbankBoletoPreviewExpiration = "expiration";
        public const string StarkbankBoletoPreviewFineAmount = "fineAmount";
        public const string StarkbankBoletoPreviewInterestAmount = "interestAmount";
        public const string StarkbankBoletoPreviewLine = "line";
        public const string StarkbankBoletoPreviewName = "name";
        public const string StarkbankBoletoPreviewPayerName = "payerName";
        public const string StarkbankBoletoPreviewPayerTaxId = "payerTaxId";
        public const string StarkbankBoletoPreviewReceiverName = "receiverName";
        public const string StarkbankBoletoPreviewReceiverTaxId = "receiverTaxId";
        public const string StarkbankBoletoPreviewStatus = "status";
        public const string StarkbankBoletoPreviewTaxId = "taxId";
        public const string StarkbankBoletoReceiverName = "receiverName";
        public const string StarkbankBoletoReceiverTaxId = "receiverTaxId";
        public const string StarkbankBoletoStateCode = "stateCode";
        public const string StarkbankBoletoStatus = "status";
        public const string StarkbankBoletoStatusCanceled = "canceled";
        public const string StarkbankBoletoStatusCreated = "created";
        public const string StarkbankBoletoStatusOverdue = "overdue";
        public const string StarkbankBoletoStatusPaid = "paid";
        public const string StarkbankBoletoStatusRegistered = "registered";
        public const string StarkbankBoletoStreetLine1 = "streetLine1";
        public const string StarkbankBoletoStreetLine2 = "streetLine2";
        public const string StarkbankBoletoTags = "tags";
        public const string StarkbankBoletoTaxId = "taxId";
        public const string StarkbankBoletoTransactionIds = "transactionIds";
        public const string StarkbankBoletoWorkspaceId = "workspaceId";
        public const string StarkbankBoletoZipCode = "zipCode";
        public const string StarkbankBrcodePaymentAmount = "amount";
        public const string StarkbankBrcodePaymentBrcode = "brcode";
        public const string StarkbankBrcodePaymentCreated = "created";
        public const string StarkbankBrcodePaymentDescription = "description";
        public const string StarkbankBrcodePaymentFee = "fee";
        public const string StarkbankBrcodePaymentId = "id";
        public const string StarkbankBrcodePaymentLogCreated = "created";
        public const string StarkbankBrcodePaymentLogErrors = "errors";
        public const string StarkbankBrcodePaymentLogId = "id";
        public const string StarkbankBrcodePaymentLogPayment = "payment";
        public const string StarkbankBrcodePaymentLogType = "type";
        public const string StarkbankBrcodePaymentName = "name";
        public const string StarkbankBrcodePaymentRules = "rules";
        public const string StarkbankBrcodePaymentRuleKey = "key";
        public const string StarkbankBrcodePaymentRuleValue = "value";
        public const string StarkbankBrcodePaymentScheduled = "scheduled";
        public const string StarkbankBrcodePaymentStatus = "status";
        public const string StarkbankBrcodePaymentStatusCanceled = "canceled";
        public const string StarkbankBrcodePaymentStatusConfirmed = "confirmed";
        public const string StarkbankBrcodePaymentStatusCreated = "created";
        public const string StarkbankBrcodePaymentStatusFailed = "failed";
        public const string StarkbankBrcodePaymentStatusProcessing = "processing";
        public const string StarkbankBrcodePaymentStatusSuccess = "success";
        public const string StarkbankBrcodePaymentTags = "tags";
        public const string StarkbankBrcodePaymentTaxId = "taxId";
        public const string StarkbankBrcodePaymentTransactionIds = "transactionIds";
        public const string StarkbankBrcodePaymentType = "type";
        public const string StarkbankBrcodePaymentUpdated = "updated";
        public const string StarkbankBrcodePreviewAccountType = "accountType";
        public const string StarkbankBrcodePreviewAllowChange = "allowChange";
        public const string StarkbankBrcodePreviewAmount = "amount";
        public const string StarkbankBrcodePreviewBankCode = "bankCode";
        public const string StarkbankBrcodePreviewDescription = "description";
        public const string StarkbankBrcodePreviewDiscountAmount = "discountAmount";
        public const string StarkbankBrcodePreviewFineAmount = "fineAmount";
        public const string StarkbankBrcodePreviewInterestAmount = "interestAmount";
        public const string StarkbankBrcodePreviewName = "name";
        public const string StarkbankBrcodePreviewNominalAmount = "nominalAmount";
        public const string StarkbankBrcodePreviewReconciliationId = "reconciliationId";
        public const string StarkbankBrcodePreviewReductionAmount = "reductionAmount";
        public const string StarkbankBrcodePreviewStatus = "status";
        public const string StarkbankBrcodePreviewTaxId = "taxId";
        public const int StarkbankEnvironmentProduction = 0;
        public const int StarkbankEnvironmentSandbox = 1;
        public const int StarkbankErrorAbi = -106;
        public const int StarkbankErrorAbsent = -103;
        public const int StarkbankErrorField = -101;
        public const int StarkbankErrorMasked = -104;
        public const int StarkbankErrorResource = -105;
        public const int StarkbankErrorType = -102;
        public const string StarkbankEventAttemptCode = "code";
        public const string StarkbankEventAttemptCreated = "created";
        public const string StarkbankEventAttemptEventId = "eventId";
        public const string StarkbankEventAttemptId = "id";
        public const string StarkbankEventAttemptMessage = "message";
        public const string StarkbankEventAttemptWebhookId = "webhookId";
        public const string StarkbankEventCreated = "created";
        public const string StarkbankEventId = "id";
        public const string StarkbankEventIsDelivered = "isDelivered";
        public const string StarkbankEventLog = "log";
        public const string StarkbankEventSubscription = "subscription";
        public const string StarkbankEventSubscriptionBoleto = "boleto";
        public const string StarkbankEventSubscriptionBoletoPayment = "boleto-payment";
        public const string StarkbankEventSubscriptionBrcodePayment = "brcode-payment";
        public const string StarkbankEventSubscriptionDarfPayment = "darf-payment";
        public const string StarkbankEventSubscriptionDeposit = "deposit";
        public const string StarkbankEventSubscriptionHolmes = "holmes";
        public const string StarkbankEventSubscriptionInvoice = "invoice";
        public const string StarkbankEventSubscriptionTaxPayment = "tax-payment";
        public const string StarkbankEventSubscriptionTransfer = "transfer";
        public const string StarkbankEventSubscriptionUtilityPayment = "utility-payment";
        public const string StarkbankEventWorkspaceId = "workspaceId";
        public const int StarkbankFieldAmount = 1;
        public const int StarkbankFieldBool = 5;
        public const int StarkbankFieldDate = 6;
        public const int StarkbankFieldDatetime = 7;
        public const int StarkbankFieldDateOrDatetime = 8;
        public const int StarkbankFieldListObject = 10;
        public const int StarkbankFieldListResource = 11;
        public const int StarkbankFieldListString = 9;
        public const int StarkbankFieldNumber = 4;
        public const int StarkbankFieldObject = 13;
        public const int StarkbankFieldRate = 2;
        public const int StarkbankFieldResource = 12;
        public const int StarkbankFieldSeconds = 3;
        public const int StarkbankFieldString = 0;
        public const int StarkbankFlagCreate = 2;
        public const int StarkbankFlagPatch = 4;
        public const int StarkbankFlagRequired = 1;
        public const string StarkbankInvoiceAmount = "amount";
        public const string StarkbankInvoiceBrcode = "brcode";
        public const string StarkbankInvoiceCreated = "created";
        public const string StarkbankInvoiceDescriptions = "descriptions";
        public const string StarkbankInvoiceDiscounts = "discounts";
        public const string StarkbankInvoiceDiscountAmount = "discountAmount";
        public const string StarkbankInvoiceDue = "due";
        public const string StarkbankInvoiceExpiration = "expiration";
        public const string StarkbankInvoiceFee = "fee";
        public const string StarkbankInvoiceFine = "fine";
        public const string StarkbankInvoiceFineAmount = "fineAmount";
        public const string StarkbankInvoiceId = "id";
        public const string StarkbankInvoiceInterest = "interest";
        public const string StarkbankInvoiceInterestAmount = "interestAmount";
        public const string StarkbankInvoiceLink = "link";
        public const string StarkbankInvoiceLogCreated = "created";
        public const string StarkbankInvoiceLogErrors = "errors";
        public const string StarkbankInvoiceLogId = "id";
        public const string StarkbankInvoiceLogInvoice = "invoice";
        public const string StarkbankInvoiceLogType = "type";
        public const string StarkbankInvoiceName = "name";
        public const string StarkbankInvoiceNominalAmount = "nominalAmount";
        public const string StarkbankInvoicePaymentAccountNumber = "accountNumber";
        public const string StarkbankInvoicePaymentAccountType = "accountType";
        public const string StarkbankInvoicePaymentAmount = "amount";
        public const string StarkbankInvoicePaymentBankCode = "bankCode";
        public const string StarkbankInvoicePaymentBranchCode = "branchCode";
        public const string StarkbankInvoicePaymentEndToEndId = "endToEndId";
        public const string StarkbankInvoicePaymentMethod = "method";
        public const string StarkbankInvoicePaymentName = "name";
        public const string StarkbankInvoicePaymentTaxId = "taxId";
        public const string StarkbankInvoicePdf = "pdf";
        public const string StarkbankInvoiceRules = "rules";
        public const string StarkbankInvoiceRuleKey = "key";
        public const string StarkbankInvoiceRuleValue = "value";
        public const string StarkbankInvoiceSplits = "splits";
        public const string StarkbankInvoiceStatus = "status";
        public const string StarkbankInvoiceStatusCanceled = "canceled";
        public const string StarkbankInvoiceStatusCreated = "created";
        public const string StarkbankInvoiceStatusExpired = "expired";
        public const string StarkbankInvoiceStatusOverdue = "overdue";
        public const string StarkbankInvoiceStatusPaid = "paid";
        public const string StarkbankInvoiceStatusRegistered = "registered";
        public const string StarkbankInvoiceStatusReversed = "reversed";
        public const string StarkbankInvoiceTags = "tags";
        public const string StarkbankInvoiceTaxId = "taxId";
        public const string StarkbankInvoiceTransactionIds = "transactionIds";
        public const string StarkbankInvoiceUpdated = "updated";
        public const int StarkbankLanguageEnUs = 0;
        public const int StarkbankLanguagePtBr = 1;
        public const int StarkbankMethodDelete = 4;
        public const int StarkbankMethodGet = 0;
        public const int StarkbankMethodPatch = 3;
        public const int StarkbankMethodPost = 1;
        public const int StarkbankMethodPut = 2;
        public const int StarkbankOk = 0;
        public const string StarkbankPaymentPreviewId = "id";
        public const string StarkbankPaymentPreviewPayment = "payment";
        public const string StarkbankPaymentPreviewScheduled = "scheduled";
        public const string StarkbankPaymentPreviewType = "type";
        public const string StarkbankPaymentPreviewTypeBoletoPayment = "boleto-payment";
        public const string StarkbankPaymentPreviewTypeBrcodePayment = "brcode-payment";
        public const string StarkbankPaymentPreviewTypeTaxPayment = "tax-payment";
        public const string StarkbankPaymentPreviewTypeUtilityPayment = "utility-payment";
        public const string StarkbankSplitAmount = "amount";
        public const string StarkbankSplitCreated = "created";
        public const string StarkbankSplitExternalId = "externalId";
        public const string StarkbankSplitId = "id";
        public const string StarkbankSplitReceiverId = "receiverId";
        public const string StarkbankSplitScheduled = "scheduled";
        public const string StarkbankSplitSource = "source";
        public const string StarkbankSplitStatus = "status";
        public const string StarkbankSplitTags = "tags";
        public const string StarkbankSplitUpdated = "updated";
        public const string StarkbankTaxPreviewAmount = "amount";
        public const string StarkbankTaxPreviewBarCode = "barCode";
        public const string StarkbankTaxPreviewDescription = "description";
        public const string StarkbankTaxPreviewLine = "line";
        public const string StarkbankTaxPreviewName = "name";
        public const string StarkbankTransferAccountNumber = "accountNumber";
        public const string StarkbankTransferAccountType = "accountType";
        public const string StarkbankTransferAccountTypeChecking = "checking";
        public const string StarkbankTransferAccountTypePayment = "payment";
        public const string StarkbankTransferAccountTypeSalary = "salary";
        public const string StarkbankTransferAccountTypeSavings = "savings";
        public const string StarkbankTransferAmount = "amount";
        public const string StarkbankTransferBankCode = "bankCode";
        public const string StarkbankTransferBranchCode = "branchCode";
        public const string StarkbankTransferCreated = "created";
        public const string StarkbankTransferDescription = "description";
        public const string StarkbankTransferDisplayDescription = "displayDescription";
        public const string StarkbankTransferExternalId = "externalId";
        public const string StarkbankTransferFee = "fee";
        public const string StarkbankTransferId = "id";
        public const string StarkbankTransferLogCreated = "created";
        public const string StarkbankTransferLogErrors = "errors";
        public const string StarkbankTransferLogId = "id";
        public const string StarkbankTransferLogTransfer = "transfer";
        public const string StarkbankTransferLogType = "type";
        public const string StarkbankTransferMetadata = "metadata";
        public const string StarkbankTransferName = "name";
        public const string StarkbankTransferRules = "rules";
        public const string StarkbankTransferRuleKey = "key";
        public const string StarkbankTransferRuleValue = "value";
        public const string StarkbankTransferScheduled = "scheduled";
        public const string StarkbankTransferStatus = "status";
        public const string StarkbankTransferStatusCanceled = "canceled";
        public const string StarkbankTransferStatusCreated = "created";
        public const string StarkbankTransferStatusFailed = "failed";
        public const string StarkbankTransferStatusProcessing = "processing";
        public const string StarkbankTransferStatusSuccess = "success";
        public const string StarkbankTransferTags = "tags";
        public const string StarkbankTransferTaxId = "taxId";
        public const string StarkbankTransferTransactionIds = "transactionIds";
        public const string StarkbankTransferUpdated = "updated";
        public const string StarkbankUtilityPreviewAmount = "amount";
        public const string StarkbankUtilityPreviewBarCode = "barCode";
        public const string StarkbankUtilityPreviewDescription = "description";
        public const string StarkbankUtilityPreviewLine = "line";
        public const string StarkbankUtilityPreviewName = "name";
        public const string StarkbankVersion = "0.1.0";
        public const string StarkbankWebhookId = "id";
        public const string StarkbankWebhookSubscriptions = "subscriptions";
        public const string StarkbankWebhookSubscriptionBoleto = "boleto";
        public const string StarkbankWebhookSubscriptionBoletoHolmes = "boleto-holmes";
        public const string StarkbankWebhookSubscriptionBoletoPayment = "boleto-payment";
        public const string StarkbankWebhookSubscriptionBrcodePayment = "brcode-payment";
        public const string StarkbankWebhookSubscriptionDarfPayment = "darf-payment";
        public const string StarkbankWebhookSubscriptionDeposit = "deposit";
        public const string StarkbankWebhookSubscriptionInvoice = "invoice";
        public const string StarkbankWebhookSubscriptionPaymentRequest = "payment-request";
        public const string StarkbankWebhookSubscriptionTaxPayment = "tax-payment";
        public const string StarkbankWebhookSubscriptionTransfer = "transfer";
        public const string StarkbankWebhookSubscriptionUtilityPayment = "utility-payment";
        public const string StarkbankWebhookUrl = "url";

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int Transport(IntPtr context, int method,
            [MarshalAs(UnmanagedType.LPStr)] string url, IntPtr headers,
            IntPtr body, UIntPtr bodyLen, int timeoutSeconds, out IntPtr response);

        [DllImport(Library, EntryPoint = "starkbank_abi_version", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankAbiVersion();

        [DllImport(Library, EntryPoint = "starkbank_balance_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBalanceGet(IntPtr client, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoCreate(IntPtr client, IntPtr boletos, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_payment_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPaymentCreate(IntPtr client, IntPtr payments, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_payment_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPaymentDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_payment_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPaymentGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_payment_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPaymentLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_payment_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPaymentLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_payment_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPaymentLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_payment_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPaymentLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_payment_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPaymentNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_payment_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPaymentPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_payment_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPaymentParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_payment_pdf", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPaymentPdf(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out UIntPtr out_len, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_payment_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPaymentQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_pdf", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoPdf(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, [MarshalAs(UnmanagedType.LPStr)] string layout, [MarshalAs(UnmanagedType.LPStr)] string hidden_fields, out IntPtr @out, out UIntPtr out_len, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentCreate(IntPtr client, IntPtr payments, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_pdf", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentPdf(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out UIntPtr out_len, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_rule_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentRuleNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_brcode_payment_update", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBrcodePaymentUpdate(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, IntPtr patch, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_client_cache_clear", CallingConvention = CallingConvention.Cdecl)]
        public static extern void StarkbankClientCacheClear(IntPtr client);

        [DllImport(Library, EntryPoint = "starkbank_client_free", CallingConvention = CallingConvention.Cdecl)]
        public static extern void StarkbankClientFree(IntPtr client);

        [DllImport(Library, EntryPoint = "starkbank_client_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankClientNew(IntPtr user, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_client_set_curl_transport", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankClientSetCurlTransport(IntPtr client);

        [DllImport(Library, EntryPoint = "starkbank_client_set_language", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankClientSetLanguage(IntPtr client, int language);

        [DllImport(Library, EntryPoint = "starkbank_client_set_max_response_size", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankClientSetMaxResponseSize(IntPtr client, UIntPtr bytes);

        [DllImport(Library, EntryPoint = "starkbank_client_set_timeout", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankClientSetTimeout(IntPtr client, int seconds);

        [DllImport(Library, EntryPoint = "starkbank_client_set_transport", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankClientSetTransport(IntPtr client, Transport transport, IntPtr context);

        [DllImport(Library, EntryPoint = "starkbank_client_set_user", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankClientSetUser(IntPtr client, IntPtr user);

        [DllImport(Library, EntryPoint = "starkbank_client_set_user_agent_prefix", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankClientSetUserAgentPrefix(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string prefix);

        [DllImport(Library, EntryPoint = "starkbank_core_version", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankCoreVersion();

        [DllImport(Library, EntryPoint = "starkbank_entity_amount", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityAmount(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, out double @out);

        [DllImport(Library, EntryPoint = "starkbank_entity_append_entity", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityAppendEntity(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, IntPtr value);

        [DllImport(Library, EntryPoint = "starkbank_entity_append_string", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityAppendString(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, [MarshalAs(UnmanagedType.LPStr)] string value);

        [DllImport(Library, EntryPoint = "starkbank_entity_bool", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityBool(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, out int @out);

        [DllImport(Library, EntryPoint = "starkbank_entity_clone", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityClone(IntPtr entity, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_entity_datetime", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityDatetime(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, out int year, out int month, out int day, out int hour, out int minute, out int second, out int out_has_time);

        [DllImport(Library, EntryPoint = "starkbank_entity_dump", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityDump(IntPtr entity, out IntPtr @out, out UIntPtr out_len);

        [DllImport(Library, EntryPoint = "starkbank_entity_entity", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityEntity(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_entity_free", CallingConvention = CallingConvention.Cdecl)]
        public static extern void StarkbankEntityFree(IntPtr entity);

        [DllImport(Library, EntryPoint = "starkbank_entity_has", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityHas(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field);

        [DllImport(Library, EntryPoint = "starkbank_entity_id", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankEntityId(IntPtr entity);

        [DllImport(Library, EntryPoint = "starkbank_entity_json", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankEntityJson(IntPtr entity);

        [DllImport(Library, EntryPoint = "starkbank_entity_list_entity_at", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityListEntityAt(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, int index, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_entity_list_size", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityListSize(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, out int @out);

        [DllImport(Library, EntryPoint = "starkbank_entity_list_string_at", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityListStringAt(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, int index, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_entity_number", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityNumber(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, out double @out);

        [DllImport(Library, EntryPoint = "starkbank_entity_resource", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankEntityResource(IntPtr entity);

        [DllImport(Library, EntryPoint = "starkbank_entity_set_amount", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntitySetAmount(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, double cents);

        [DllImport(Library, EntryPoint = "starkbank_entity_set_bool", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntitySetBool(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, int value);

        [DllImport(Library, EntryPoint = "starkbank_entity_set_date", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntitySetDate(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, int year, int month, int day);

        [DllImport(Library, EntryPoint = "starkbank_entity_set_datetime", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntitySetDatetime(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, int year, int month, int day, int hour, int minute, int second);

        [DllImport(Library, EntryPoint = "starkbank_entity_set_json_raw", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntitySetJsonRaw(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, [MarshalAs(UnmanagedType.LPStr)] string json_text);

        [DllImport(Library, EntryPoint = "starkbank_entity_set_number", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntitySetNumber(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, double value);

        [DllImport(Library, EntryPoint = "starkbank_entity_set_seconds", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntitySetSeconds(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, int seconds);

        [DllImport(Library, EntryPoint = "starkbank_entity_set_string", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntitySetString(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, [MarshalAs(UnmanagedType.LPStr)] string value);

        [DllImport(Library, EntryPoint = "starkbank_entity_string", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityString(IntPtr entity, [MarshalAs(UnmanagedType.LPStr)] string field, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_entity_unknown_count", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEntityUnknownCount(IntPtr entity);

        [DllImport(Library, EntryPoint = "starkbank_errors_code_at", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankErrorsCodeAt(IntPtr errors, int index);

        [DllImport(Library, EntryPoint = "starkbank_errors_count", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankErrorsCount(IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_errors_free", CallingConvention = CallingConvention.Cdecl)]
        public static extern void StarkbankErrorsFree(IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_errors_message_at", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankErrorsMessageAt(IntPtr errors, int index);

        [DllImport(Library, EntryPoint = "starkbank_event_attempt_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEventAttemptGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_event_attempt_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEventAttemptPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_event_attempt_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEventAttemptParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_event_attempt_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEventAttemptQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_event_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEventDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_event_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEventGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_event_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEventPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_event_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEventParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_event_parse", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEventParse(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string content, UIntPtr content_len, [MarshalAs(UnmanagedType.LPStr)] string signature_base64, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_event_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEventQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_event_update", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankEventUpdate(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, IntPtr patch, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_free", CallingConvention = CallingConvention.Cdecl)]
        public static extern void StarkbankFree(IntPtr pointer);

        [DllImport(Library, EntryPoint = "starkbank_headers_count", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankHeadersCount(IntPtr headers);

        [DllImport(Library, EntryPoint = "starkbank_headers_name_at", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankHeadersNameAt(IntPtr headers, int index);

        [DllImport(Library, EntryPoint = "starkbank_headers_value_at", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankHeadersValueAt(IntPtr headers, int index);

        [DllImport(Library, EntryPoint = "starkbank_invoice_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceCreate(IntPtr client, IntPtr invoices, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_log_pdf", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceLogPdf(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out UIntPtr out_len, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_payment", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePayment(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pdf", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePdf(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out UIntPtr out_len, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_qrcode", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceQrcode(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, int size, out IntPtr @out, out UIntPtr out_len, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_rule_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceRuleNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_update", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoiceUpdate(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, IntPtr patch, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_iter_cursor", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankIterCursor(IntPtr iter);

        [DllImport(Library, EntryPoint = "starkbank_iter_free", CallingConvention = CallingConvention.Cdecl)]
        public static extern void StarkbankIterFree(IntPtr iter);

        [DllImport(Library, EntryPoint = "starkbank_iter_next", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankIterNext(IntPtr iter, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_list_append", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankListAppend(IntPtr list, IntPtr entity);

        [DllImport(Library, EntryPoint = "starkbank_list_at", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankListAt(IntPtr list, int index);

        [DllImport(Library, EntryPoint = "starkbank_list_count", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankListCount(IntPtr list);

        [DllImport(Library, EntryPoint = "starkbank_list_free", CallingConvention = CallingConvention.Cdecl)]
        public static extern void StarkbankListFree(IntPtr list);

        [DllImport(Library, EntryPoint = "starkbank_list_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankListNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_object_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankObjectNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_organization_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankOrganizationNew([MarshalAs(UnmanagedType.LPStr)] string id, int environment, [MarshalAs(UnmanagedType.LPStr)] string private_key_pem, [MarshalAs(UnmanagedType.LPStr)] string workspace_id, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_organization_replace", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankOrganizationReplace(IntPtr organization, [MarshalAs(UnmanagedType.LPStr)] string workspace_id, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_parse_and_verify", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankParseAndVerify(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string content, UIntPtr content_len, [MarshalAs(UnmanagedType.LPStr)] string signature_base64, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_payment_preview_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankPaymentPreviewCreate(IntPtr client, IntPtr previews, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_payment_preview_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankPaymentPreviewNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_project_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankProjectNew([MarshalAs(UnmanagedType.LPStr)] string id, int environment, [MarshalAs(UnmanagedType.LPStr)] string private_key_pem, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_resource_count", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankResourceCount();

        [DllImport(Library, EntryPoint = "starkbank_resource_field_count", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankResourceFieldCount([MarshalAs(UnmanagedType.LPStr)] string resource);

        [DllImport(Library, EntryPoint = "starkbank_resource_field_flags_at", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankResourceFieldFlagsAt([MarshalAs(UnmanagedType.LPStr)] string resource, int index);

        [DllImport(Library, EntryPoint = "starkbank_resource_field_name_at", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankResourceFieldNameAt([MarshalAs(UnmanagedType.LPStr)] string resource, int index);

        [DllImport(Library, EntryPoint = "starkbank_resource_field_type_at", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankResourceFieldTypeAt([MarshalAs(UnmanagedType.LPStr)] string resource, int index);

        [DllImport(Library, EntryPoint = "starkbank_resource_name_at", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankResourceNameAt(int index);

        [DllImport(Library, EntryPoint = "starkbank_response_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankResponseNew(int status, IntPtr content, UIntPtr content_len, IntPtr headers, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_split_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankSplitNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_strerror", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankStrerror(int code);

        [DllImport(Library, EntryPoint = "starkbank_transfer_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferCreate(IntPtr client, IntPtr transfers, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_transfer_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_transfer_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_transfer_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_transfer_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_transfer_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_transfer_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_transfer_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_transfer_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_transfer_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_transfer_pdf", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferPdf(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out UIntPtr out_len, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_transfer_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_transfer_rule_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransferRuleNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_user_access_id", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankUserAccessId(IntPtr user);

        [DllImport(Library, EntryPoint = "starkbank_user_environment", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUserEnvironment(IntPtr user);

        [DllImport(Library, EntryPoint = "starkbank_user_free", CallingConvention = CallingConvention.Cdecl)]
        public static extern void StarkbankUserFree(IntPtr user);

        [DllImport(Library, EntryPoint = "starkbank_version", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr StarkbankVersion();

        [DllImport(Library, EntryPoint = "starkbank_webhook_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWebhookCreate(IntPtr client, IntPtr webhook, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_webhook_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWebhookDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_webhook_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWebhookGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_webhook_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWebhookNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_webhook_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWebhookPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_webhook_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWebhookParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_webhook_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWebhookQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

    }
}
