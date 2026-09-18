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
        public const string StarkbankAllowedInstallmentCount = "count";
        public const string StarkbankAllowedInstallmentTotalAmount = "totalAmount";
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
        public const string StarkbankBoletoHolmesBoletoId = "boletoId";
        public const string StarkbankBoletoHolmesCreated = "created";
        public const string StarkbankBoletoHolmesId = "id";
        public const string StarkbankBoletoHolmesLogCreated = "created";
        public const string StarkbankBoletoHolmesLogHolmes = "holmes";
        public const string StarkbankBoletoHolmesLogId = "id";
        public const string StarkbankBoletoHolmesLogType = "type";
        public const string StarkbankBoletoHolmesLogUpdated = "updated";
        public const string StarkbankBoletoHolmesResult = "result";
        public const string StarkbankBoletoHolmesResultCancelled = "cancelled";
        public const string StarkbankBoletoHolmesResultPaid = "paid";
        public const string StarkbankBoletoHolmesStatus = "status";
        public const string StarkbankBoletoHolmesStatusSolved = "solved";
        public const string StarkbankBoletoHolmesStatusSolving = "solving";
        public const string StarkbankBoletoHolmesTags = "tags";
        public const string StarkbankBoletoHolmesUpdated = "updated";
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
        public const string StarkbankCardMethodCode = "code";
        public const string StarkbankCardMethodCodeChip = "chip";
        public const string StarkbankCardMethodCodeContactless = "contactless";
        public const string StarkbankCardMethodCodeMagstripe = "magstripe";
        public const string StarkbankCardMethodCodeManual = "manual";
        public const string StarkbankCardMethodCodeServer = "server";
        public const string StarkbankCardMethodCodeToken = "token";
        public const string StarkbankCardMethodName = "name";
        public const string StarkbankCardMethodNumber = "number";
        public const string StarkbankCorporateBalanceAmount = "amount";
        public const string StarkbankCorporateBalanceCurrency = "currency";
        public const string StarkbankCorporateBalanceId = "id";
        public const string StarkbankCorporateBalanceLimit = "limit";
        public const string StarkbankCorporateBalanceMaxLimit = "maxLimit";
        public const string StarkbankCorporateBalanceUpdated = "updated";
        public const string StarkbankCorporateCardCity = "city";
        public const string StarkbankCorporateCardCreated = "created";
        public const string StarkbankCorporateCardDisplayName = "displayName";
        public const string StarkbankCorporateCardDistrict = "district";
        public const string StarkbankCorporateCardExpiration = "expiration";
        public const string StarkbankCorporateCardHolderId = "holderId";
        public const string StarkbankCorporateCardHolderName = "holderName";
        public const string StarkbankCorporateCardId = "id";
        public const string StarkbankCorporateCardLogCard = "card";
        public const string StarkbankCorporateCardLogCreated = "created";
        public const string StarkbankCorporateCardLogId = "id";
        public const string StarkbankCorporateCardLogType = "type";
        public const string StarkbankCorporateCardNumber = "number";
        public const string StarkbankCorporateCardPin = "pin";
        public const string StarkbankCorporateCardRules = "rules";
        public const string StarkbankCorporateCardSecurityCode = "securityCode";
        public const string StarkbankCorporateCardStateCode = "stateCode";
        public const string StarkbankCorporateCardStatus = "status";
        public const string StarkbankCorporateCardStatusActive = "active";
        public const string StarkbankCorporateCardStatusBlocked = "blocked";
        public const string StarkbankCorporateCardStatusCanceled = "canceled";
        public const string StarkbankCorporateCardStatusExpired = "expired";
        public const string StarkbankCorporateCardStatusPending = "pending";
        public const string StarkbankCorporateCardStreetLine1 = "streetLine1";
        public const string StarkbankCorporateCardStreetLine2 = "streetLine2";
        public const string StarkbankCorporateCardTags = "tags";
        public const string StarkbankCorporateCardType = "type";
        public const string StarkbankCorporateCardTypePhysical = "physical";
        public const string StarkbankCorporateCardTypeVirtual = "virtual";
        public const string StarkbankCorporateCardTypeWallet = "wallet";
        public const string StarkbankCorporateCardUpdated = "updated";
        public const string StarkbankCorporateCardZipCode = "zipCode";
        public const string StarkbankCorporateHolderCenterId = "centerId";
        public const string StarkbankCorporateHolderCreated = "created";
        public const string StarkbankCorporateHolderId = "id";
        public const string StarkbankCorporateHolderLogCreated = "created";
        public const string StarkbankCorporateHolderLogHolder = "holder";
        public const string StarkbankCorporateHolderLogId = "id";
        public const string StarkbankCorporateHolderLogType = "type";
        public const string StarkbankCorporateHolderName = "name";
        public const string StarkbankCorporateHolderPermissions = "permissions";
        public const string StarkbankCorporateHolderRules = "rules";
        public const string StarkbankCorporateHolderStatus = "status";
        public const string StarkbankCorporateHolderStatusActive = "active";
        public const string StarkbankCorporateHolderStatusBlocked = "blocked";
        public const string StarkbankCorporateHolderStatusCanceled = "canceled";
        public const string StarkbankCorporateHolderTags = "tags";
        public const string StarkbankCorporateHolderUpdated = "updated";
        public const string StarkbankCorporateInvoiceAmount = "amount";
        public const string StarkbankCorporateInvoiceBrcode = "brcode";
        public const string StarkbankCorporateInvoiceCorporateTransactionId = "corporateTransactionId";
        public const string StarkbankCorporateInvoiceCreated = "created";
        public const string StarkbankCorporateInvoiceDue = "due";
        public const string StarkbankCorporateInvoiceId = "id";
        public const string StarkbankCorporateInvoiceLink = "link";
        public const string StarkbankCorporateInvoiceName = "name";
        public const string StarkbankCorporateInvoiceStatus = "status";
        public const string StarkbankCorporateInvoiceStatusCreated = "created";
        public const string StarkbankCorporateInvoiceStatusExpired = "expired";
        public const string StarkbankCorporateInvoiceStatusOverdue = "overdue";
        public const string StarkbankCorporateInvoiceStatusPaid = "paid";
        public const string StarkbankCorporateInvoiceTags = "tags";
        public const string StarkbankCorporateInvoiceTaxId = "taxId";
        public const string StarkbankCorporateInvoiceUpdated = "updated";
        public const string StarkbankCorporatePurchaseAmount = "amount";
        public const string StarkbankCorporatePurchaseCardEnding = "cardEnding";
        public const string StarkbankCorporatePurchaseCardId = "cardId";
        public const string StarkbankCorporatePurchaseCenterId = "centerId";
        public const string StarkbankCorporatePurchaseCorporateTransactionIds = "corporateTransactionIds";
        public const string StarkbankCorporatePurchaseCreated = "created";
        public const string StarkbankCorporatePurchaseDescription = "description";
        public const string StarkbankCorporatePurchaseHolderId = "holderId";
        public const string StarkbankCorporatePurchaseHolderName = "holderName";
        public const string StarkbankCorporatePurchaseId = "id";
        public const string StarkbankCorporatePurchaseIssuerAmount = "issuerAmount";
        public const string StarkbankCorporatePurchaseIssuerCurrencyCode = "issuerCurrencyCode";
        public const string StarkbankCorporatePurchaseIssuerCurrencySymbol = "issuerCurrencySymbol";
        public const string StarkbankCorporatePurchaseLogCorporateTransactionId = "corporateTransactionId";
        public const string StarkbankCorporatePurchaseLogCreated = "created";
        public const string StarkbankCorporatePurchaseLogDescription = "description";
        public const string StarkbankCorporatePurchaseLogErrors = "errors";
        public const string StarkbankCorporatePurchaseLogId = "id";
        public const string StarkbankCorporatePurchaseLogPurchase = "purchase";
        public const string StarkbankCorporatePurchaseLogType = "type";
        public const string StarkbankCorporatePurchaseMerchantAmount = "merchantAmount";
        public const string StarkbankCorporatePurchaseMerchantCategoryCode = "merchantCategoryCode";
        public const string StarkbankCorporatePurchaseMerchantCategoryType = "merchantCategoryType";
        public const string StarkbankCorporatePurchaseMerchantCountryCode = "merchantCountryCode";
        public const string StarkbankCorporatePurchaseMerchantCurrencyCode = "merchantCurrencyCode";
        public const string StarkbankCorporatePurchaseMerchantCurrencySymbol = "merchantCurrencySymbol";
        public const string StarkbankCorporatePurchaseMerchantDisplayName = "merchantDisplayName";
        public const string StarkbankCorporatePurchaseMerchantDisplayUrl = "merchantDisplayUrl";
        public const string StarkbankCorporatePurchaseMerchantFee = "merchantFee";
        public const string StarkbankCorporatePurchaseMerchantName = "merchantName";
        public const string StarkbankCorporatePurchaseMethodCode = "methodCode";
        public const string StarkbankCorporatePurchaseStatus = "status";
        public const string StarkbankCorporatePurchaseStatusApproved = "approved";
        public const string StarkbankCorporatePurchaseStatusCanceled = "canceled";
        public const string StarkbankCorporatePurchaseStatusConfirmed = "confirmed";
        public const string StarkbankCorporatePurchaseStatusDenied = "denied";
        public const string StarkbankCorporatePurchaseStatusVoided = "voided";
        public const string StarkbankCorporatePurchaseTags = "tags";
        public const string StarkbankCorporatePurchaseTax = "tax";
        public const string StarkbankCorporatePurchaseUpdated = "updated";
        public const string StarkbankCorporateRuleAmount = "amount";
        public const string StarkbankCorporateRuleCategories = "categories";
        public const string StarkbankCorporateRuleCounterAmount = "counterAmount";
        public const string StarkbankCorporateRuleCountries = "countries";
        public const string StarkbankCorporateRuleCurrencyCode = "currencyCode";
        public const string StarkbankCorporateRuleCurrencyName = "currencyName";
        public const string StarkbankCorporateRuleCurrencySymbol = "currencySymbol";
        public const string StarkbankCorporateRuleId = "id";
        public const string StarkbankCorporateRuleInterval = "interval";
        public const string StarkbankCorporateRuleIntervalDay = "day";
        public const string StarkbankCorporateRuleIntervalInstant = "instant";
        public const string StarkbankCorporateRuleIntervalLifetime = "lifetime";
        public const string StarkbankCorporateRuleIntervalMonth = "month";
        public const string StarkbankCorporateRuleIntervalWeek = "week";
        public const string StarkbankCorporateRuleIntervalYear = "year";
        public const string StarkbankCorporateRuleMethods = "methods";
        public const string StarkbankCorporateRuleName = "name";
        public const string StarkbankCorporateRulePurposes = "purposes";
        public const string StarkbankCorporateRulePurposePurchase = "purchase";
        public const string StarkbankCorporateRulePurposeVerification = "verification";
        public const string StarkbankCorporateRulePurposeWithdrawal = "withdrawal";
        public const string StarkbankCorporateRuleSchedule = "schedule";
        public const string StarkbankCorporateTransactionAmount = "amount";
        public const string StarkbankCorporateTransactionBalance = "balance";
        public const string StarkbankCorporateTransactionCreated = "created";
        public const string StarkbankCorporateTransactionDescription = "description";
        public const string StarkbankCorporateTransactionId = "id";
        public const string StarkbankCorporateTransactionSource = "source";
        public const string StarkbankCorporateTransactionTags = "tags";
        public const string StarkbankCorporateWithdrawalAmount = "amount";
        public const string StarkbankCorporateWithdrawalCorporateTransactionId = "corporateTransactionId";
        public const string StarkbankCorporateWithdrawalCreated = "created";
        public const string StarkbankCorporateWithdrawalExternalId = "externalId";
        public const string StarkbankCorporateWithdrawalId = "id";
        public const string StarkbankCorporateWithdrawalTags = "tags";
        public const string StarkbankCorporateWithdrawalTransactionId = "transactionId";
        public const string StarkbankCorporateWithdrawalUpdated = "updated";
        public const string StarkbankDarfPaymentAmount = "amount";
        public const string StarkbankDarfPaymentCompetence = "competence";
        public const string StarkbankDarfPaymentCreated = "created";
        public const string StarkbankDarfPaymentDescription = "description";
        public const string StarkbankDarfPaymentDue = "due";
        public const string StarkbankDarfPaymentFee = "fee";
        public const string StarkbankDarfPaymentFineAmount = "fineAmount";
        public const string StarkbankDarfPaymentId = "id";
        public const string StarkbankDarfPaymentInterestAmount = "interestAmount";
        public const string StarkbankDarfPaymentLogCreated = "created";
        public const string StarkbankDarfPaymentLogErrors = "errors";
        public const string StarkbankDarfPaymentLogId = "id";
        public const string StarkbankDarfPaymentLogPayment = "payment";
        public const string StarkbankDarfPaymentLogType = "type";
        public const string StarkbankDarfPaymentNominalAmount = "nominalAmount";
        public const string StarkbankDarfPaymentReferenceNumber = "referenceNumber";
        public const string StarkbankDarfPaymentRevenueCode = "revenueCode";
        public const string StarkbankDarfPaymentScheduled = "scheduled";
        public const string StarkbankDarfPaymentStatus = "status";
        public const string StarkbankDarfPaymentStatusCanceled = "canceled";
        public const string StarkbankDarfPaymentStatusConfirmed = "confirmed";
        public const string StarkbankDarfPaymentStatusCreated = "created";
        public const string StarkbankDarfPaymentStatusFailed = "failed";
        public const string StarkbankDarfPaymentStatusProcessing = "processing";
        public const string StarkbankDarfPaymentStatusSuccess = "success";
        public const string StarkbankDarfPaymentTags = "tags";
        public const string StarkbankDarfPaymentTaxId = "taxId";
        public const string StarkbankDarfPaymentTransactionIds = "transactionIds";
        public const string StarkbankDarfPaymentUpdated = "updated";
        public const string StarkbankDepositAccountNumber = "accountNumber";
        public const string StarkbankDepositAccountType = "accountType";
        public const string StarkbankDepositAmount = "amount";
        public const string StarkbankDepositBankCode = "bankCode";
        public const string StarkbankDepositBranchCode = "branchCode";
        public const string StarkbankDepositCreated = "created";
        public const string StarkbankDepositFee = "fee";
        public const string StarkbankDepositId = "id";
        public const string StarkbankDepositLogCreated = "created";
        public const string StarkbankDepositLogDeposit = "deposit";
        public const string StarkbankDepositLogErrors = "errors";
        public const string StarkbankDepositLogId = "id";
        public const string StarkbankDepositLogType = "type";
        public const string StarkbankDepositName = "name";
        public const string StarkbankDepositStatus = "status";
        public const string StarkbankDepositStatusCreated = "created";
        public const string StarkbankDepositStatusVoid = "void";
        public const string StarkbankDepositTags = "tags";
        public const string StarkbankDepositTaxId = "taxId";
        public const string StarkbankDepositTransactionIds = "transactionIds";
        public const string StarkbankDepositType = "type";
        public const string StarkbankDepositUpdated = "updated";
        public const string StarkbankDictKeyAccountNumber = "accountNumber";
        public const string StarkbankDictKeyAccountType = "accountType";
        public const string StarkbankDictKeyBankName = "bankName";
        public const string StarkbankDictKeyBranchCode = "branchCode";
        public const string StarkbankDictKeyId = "id";
        public const string StarkbankDictKeyIspb = "ispb";
        public const string StarkbankDictKeyName = "name";
        public const string StarkbankDictKeyOwnerType = "ownerType";
        public const string StarkbankDictKeyStatus = "status";
        public const string StarkbankDictKeyStatusCanceled = "canceled";
        public const string StarkbankDictKeyStatusCreated = "created";
        public const string StarkbankDictKeyStatusFailed = "failed";
        public const string StarkbankDictKeyStatusRegistered = "registered";
        public const string StarkbankDictKeyTaxId = "taxId";
        public const string StarkbankDictKeyType = "type";
        public const string StarkbankDictKeyTypeCnpj = "cnpj";
        public const string StarkbankDictKeyTypeCpf = "cpf";
        public const string StarkbankDictKeyTypeEmail = "email";
        public const string StarkbankDictKeyTypeEvp = "evp";
        public const string StarkbankDictKeyTypePhone = "phone";
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
        public const string StarkbankInstitutionDisplayName = "displayName";
        public const string StarkbankInstitutionName = "name";
        public const string StarkbankInstitutionSpiCode = "spiCode";
        public const string StarkbankInstitutionStrCode = "strCode";
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
        public const string StarkbankInvoicePullRequestAttemptType = "attemptType";
        public const string StarkbankInvoicePullRequestAttemptTypeDefault = "default";
        public const string StarkbankInvoicePullRequestAttemptTypeRetry = "retry";
        public const string StarkbankInvoicePullRequestCreated = "created";
        public const string StarkbankInvoicePullRequestDisplayDescription = "displayDescription";
        public const string StarkbankInvoicePullRequestDue = "due";
        public const string StarkbankInvoicePullRequestExternalId = "externalId";
        public const string StarkbankInvoicePullRequestId = "id";
        public const string StarkbankInvoicePullRequestInstallmentId = "installmentId";
        public const string StarkbankInvoicePullRequestInvoiceId = "invoiceId";
        public const string StarkbankInvoicePullRequestLogCreated = "created";
        public const string StarkbankInvoicePullRequestLogErrors = "errors";
        public const string StarkbankInvoicePullRequestLogId = "id";
        public const string StarkbankInvoicePullRequestLogRequest = "request";
        public const string StarkbankInvoicePullRequestLogType = "type";
        public const string StarkbankInvoicePullRequestStatus = "status";
        public const string StarkbankInvoicePullRequestStatusCanceled = "canceled";
        public const string StarkbankInvoicePullRequestStatusFailed = "failed";
        public const string StarkbankInvoicePullRequestStatusPending = "pending";
        public const string StarkbankInvoicePullRequestStatusScheduled = "scheduled";
        public const string StarkbankInvoicePullRequestStatusSuccess = "success";
        public const string StarkbankInvoicePullRequestSubscriptionId = "subscriptionId";
        public const string StarkbankInvoicePullRequestTags = "tags";
        public const string StarkbankInvoicePullRequestUpdated = "updated";
        public const string StarkbankInvoicePullSubscriptionAmount = "amount";
        public const string StarkbankInvoicePullSubscriptionAmountMinLimit = "amountMinLimit";
        public const string StarkbankInvoicePullSubscriptionBacenId = "bacenId";
        public const string StarkbankInvoicePullSubscriptionBrcode = "brcode";
        public const string StarkbankInvoicePullSubscriptionCreated = "created";
        public const string StarkbankInvoicePullSubscriptionData = "data";
        public const string StarkbankInvoicePullSubscriptionDisplayDescription = "displayDescription";
        public const string StarkbankInvoicePullSubscriptionDue = "due";
        public const string StarkbankInvoicePullSubscriptionEnd = "end";
        public const string StarkbankInvoicePullSubscriptionExternalId = "externalId";
        public const string StarkbankInvoicePullSubscriptionId = "id";
        public const string StarkbankInvoicePullSubscriptionInterval = "interval";
        public const string StarkbankInvoicePullSubscriptionIntervalMonth = "month";
        public const string StarkbankInvoicePullSubscriptionIntervalQuarter = "quarter";
        public const string StarkbankInvoicePullSubscriptionIntervalSemester = "semester";
        public const string StarkbankInvoicePullSubscriptionIntervalWeek = "week";
        public const string StarkbankInvoicePullSubscriptionIntervalYear = "year";
        public const string StarkbankInvoicePullSubscriptionLogCreated = "created";
        public const string StarkbankInvoicePullSubscriptionLogErrors = "errors";
        public const string StarkbankInvoicePullSubscriptionLogId = "id";
        public const string StarkbankInvoicePullSubscriptionLogSubscription = "subscription";
        public const string StarkbankInvoicePullSubscriptionLogType = "type";
        public const string StarkbankInvoicePullSubscriptionName = "name";
        public const string StarkbankInvoicePullSubscriptionPullMode = "pullMode";
        public const string StarkbankInvoicePullSubscriptionPullModeAutomatic = "automatic";
        public const string StarkbankInvoicePullSubscriptionPullModeManual = "manual";
        public const string StarkbankInvoicePullSubscriptionPullRetryLimit = "pullRetryLimit";
        public const string StarkbankInvoicePullSubscriptionReferenceCode = "referenceCode";
        public const string StarkbankInvoicePullSubscriptionStart = "start";
        public const string StarkbankInvoicePullSubscriptionStatus = "status";
        public const string StarkbankInvoicePullSubscriptionStatusActive = "active";
        public const string StarkbankInvoicePullSubscriptionStatusCanceled = "canceled";
        public const string StarkbankInvoicePullSubscriptionTags = "tags";
        public const string StarkbankInvoicePullSubscriptionTaxId = "taxId";
        public const string StarkbankInvoicePullSubscriptionType = "type";
        public const string StarkbankInvoicePullSubscriptionTypePaymentAndOrQrcode = "paymentAndOrQrcode";
        public const string StarkbankInvoicePullSubscriptionTypePush = "push";
        public const string StarkbankInvoicePullSubscriptionTypeQrcode = "qrcode";
        public const string StarkbankInvoicePullSubscriptionTypeQrcodeAndPayment = "qrcodeAndPayment";
        public const string StarkbankInvoicePullSubscriptionUpdated = "updated";
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
        public const string StarkbankMerchantCardCreated = "created";
        public const string StarkbankMerchantCardEnding = "ending";
        public const string StarkbankMerchantCardExpiration = "expiration";
        public const string StarkbankMerchantCardFundingType = "fundingType";
        public const string StarkbankMerchantCardHolderName = "holderName";
        public const string StarkbankMerchantCardId = "id";
        public const string StarkbankMerchantCardLogCard = "card";
        public const string StarkbankMerchantCardLogCreated = "created";
        public const string StarkbankMerchantCardLogErrors = "errors";
        public const string StarkbankMerchantCardLogId = "id";
        public const string StarkbankMerchantCardLogType = "type";
        public const string StarkbankMerchantCardLogUpdated = "updated";
        public const string StarkbankMerchantCardNetwork = "network";
        public const string StarkbankMerchantCardStatus = "status";
        public const string StarkbankMerchantCardStatusActive = "active";
        public const string StarkbankMerchantCardStatusBlocked = "blocked";
        public const string StarkbankMerchantCardStatusCanceled = "canceled";
        public const string StarkbankMerchantCardStatusExpired = "expired";
        public const string StarkbankMerchantCardTags = "tags";
        public const string StarkbankMerchantCardUpdated = "updated";
        public const string StarkbankMerchantCategoryCode = "code";
        public const string StarkbankMerchantCategoryName = "name";
        public const string StarkbankMerchantCategoryNumber = "number";
        public const string StarkbankMerchantCategoryType = "type";
        public const string StarkbankMerchantCountryCode = "code";
        public const string StarkbankMerchantCountryName = "name";
        public const string StarkbankMerchantCountryNumber = "number";
        public const string StarkbankMerchantCountryShortCode = "shortCode";
        public const string StarkbankMerchantInstallmentAmount = "amount";
        public const string StarkbankMerchantInstallmentCreated = "created";
        public const string StarkbankMerchantInstallmentDue = "due";
        public const string StarkbankMerchantInstallmentFee = "fee";
        public const string StarkbankMerchantInstallmentFundingType = "fundingType";
        public const string StarkbankMerchantInstallmentId = "id";
        public const string StarkbankMerchantInstallmentLogCreated = "created";
        public const string StarkbankMerchantInstallmentLogErrors = "errors";
        public const string StarkbankMerchantInstallmentLogId = "id";
        public const string StarkbankMerchantInstallmentLogInstallment = "installment";
        public const string StarkbankMerchantInstallmentLogType = "type";
        public const string StarkbankMerchantInstallmentLogUpdated = "updated";
        public const string StarkbankMerchantInstallmentNetwork = "network";
        public const string StarkbankMerchantInstallmentPurchaseId = "purchaseId";
        public const string StarkbankMerchantInstallmentStatus = "status";
        public const string StarkbankMerchantInstallmentStatusCreated = "created";
        public const string StarkbankMerchantInstallmentStatusFailed = "failed";
        public const string StarkbankMerchantInstallmentStatusSuccess = "success";
        public const string StarkbankMerchantInstallmentTags = "tags";
        public const string StarkbankMerchantInstallmentTransactionIds = "transactionIds";
        public const string StarkbankMerchantInstallmentUpdated = "updated";
        public const string StarkbankMerchantPurchaseAmount = "amount";
        public const string StarkbankMerchantPurchaseBillingCity = "billingCity";
        public const string StarkbankMerchantPurchaseBillingCountryCode = "billingCountryCode";
        public const string StarkbankMerchantPurchaseBillingStateCode = "billingStateCode";
        public const string StarkbankMerchantPurchaseBillingStreetLine1 = "billingStreetLine1";
        public const string StarkbankMerchantPurchaseBillingStreetLine2 = "billingStreetLine2";
        public const string StarkbankMerchantPurchaseBillingZipCode = "billingZipCode";
        public const string StarkbankMerchantPurchaseCardEnding = "cardEnding";
        public const string StarkbankMerchantPurchaseCardExpiration = "cardExpiration";
        public const string StarkbankMerchantPurchaseCardId = "cardId";
        public const string StarkbankMerchantPurchaseCardNumber = "cardNumber";
        public const string StarkbankMerchantPurchaseCardSecurityCode = "cardSecurityCode";
        public const string StarkbankMerchantPurchaseChallengeMode = "challengeMode";
        public const string StarkbankMerchantPurchaseChallengeUrl = "challengeUrl";
        public const string StarkbankMerchantPurchaseCreated = "created";
        public const string StarkbankMerchantPurchaseCurrencyCode = "currencyCode";
        public const string StarkbankMerchantPurchaseEndToEndId = "endToEndId";
        public const string StarkbankMerchantPurchaseFee = "fee";
        public const string StarkbankMerchantPurchaseFundingType = "fundingType";
        public const string StarkbankMerchantPurchaseHolderEmail = "holderEmail";
        public const string StarkbankMerchantPurchaseHolderId = "holderId";
        public const string StarkbankMerchantPurchaseHolderName = "holderName";
        public const string StarkbankMerchantPurchaseHolderPhone = "holderPhone";
        public const string StarkbankMerchantPurchaseId = "id";
        public const string StarkbankMerchantPurchaseInstallmentCount = "installmentCount";
        public const string StarkbankMerchantPurchaseLogCreated = "created";
        public const string StarkbankMerchantPurchaseLogErrors = "errors";
        public const string StarkbankMerchantPurchaseLogId = "id";
        public const string StarkbankMerchantPurchaseLogPurchase = "purchase";
        public const string StarkbankMerchantPurchaseLogType = "type";
        public const string StarkbankMerchantPurchaseMetadata = "metadata";
        public const string StarkbankMerchantPurchaseNetwork = "network";
        public const string StarkbankMerchantPurchaseSoftDescriptor = "softDescriptor";
        public const string StarkbankMerchantPurchaseSource = "source";
        public const string StarkbankMerchantPurchaseStatus = "status";
        public const string StarkbankMerchantPurchaseStatusApproved = "approved";
        public const string StarkbankMerchantPurchaseStatusCanceled = "canceled";
        public const string StarkbankMerchantPurchaseStatusConfirmed = "confirmed";
        public const string StarkbankMerchantPurchaseStatusReversed = "reversed";
        public const string StarkbankMerchantPurchaseStatusVoided = "voided";
        public const string StarkbankMerchantPurchaseTags = "tags";
        public const string StarkbankMerchantPurchaseUpdated = "updated";
        public const string StarkbankMerchantSessionAllowedFundingTypes = "allowedFundingTypes";
        public const string StarkbankMerchantSessionAllowedInstallments = "allowedInstallments";
        public const string StarkbankMerchantSessionAllowedIps = "allowedIps";
        public const string StarkbankMerchantSessionChallengeMode = "challengeMode";
        public const string StarkbankMerchantSessionChallengeModeDisabled = "disabled";
        public const string StarkbankMerchantSessionChallengeModeEnabled = "enabled";
        public const string StarkbankMerchantSessionCreated = "created";
        public const string StarkbankMerchantSessionExpiration = "expiration";
        public const string StarkbankMerchantSessionFundingTypeCredit = "credit";
        public const string StarkbankMerchantSessionFundingTypeDebit = "debit";
        public const string StarkbankMerchantSessionHolderId = "holderId";
        public const string StarkbankMerchantSessionId = "id";
        public const string StarkbankMerchantSessionLogCreated = "created";
        public const string StarkbankMerchantSessionLogErrors = "errors";
        public const string StarkbankMerchantSessionLogId = "id";
        public const string StarkbankMerchantSessionLogSession = "session";
        public const string StarkbankMerchantSessionLogType = "type";
        public const string StarkbankMerchantSessionSoftDescriptor = "softDescriptor";
        public const string StarkbankMerchantSessionStatus = "status";
        public const string StarkbankMerchantSessionStatusActive = "active";
        public const string StarkbankMerchantSessionStatusCreated = "created";
        public const string StarkbankMerchantSessionStatusExpired = "expired";
        public const string StarkbankMerchantSessionStatusSuccess = "success";
        public const string StarkbankMerchantSessionTags = "tags";
        public const string StarkbankMerchantSessionUpdated = "updated";
        public const string StarkbankMerchantSessionUuid = "uuid";
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
        public const string StarkbankPermissionCreated = "created";
        public const string StarkbankPermissionOwnerEmail = "ownerEmail";
        public const string StarkbankPermissionOwnerId = "ownerId";
        public const string StarkbankPermissionOwnerName = "ownerName";
        public const string StarkbankPermissionOwnerPictureUrl = "ownerPictureUrl";
        public const string StarkbankPermissionOwnerStatus = "ownerStatus";
        public const string StarkbankPermissionOwnerType = "ownerType";
        public const string StarkbankPurchaseAmount = "amount";
        public const string StarkbankPurchaseBillingCity = "billingCity";
        public const string StarkbankPurchaseBillingCountryCode = "billingCountryCode";
        public const string StarkbankPurchaseBillingStateCode = "billingStateCode";
        public const string StarkbankPurchaseBillingStreetLine1 = "billingStreetLine1";
        public const string StarkbankPurchaseBillingStreetLine2 = "billingStreetLine2";
        public const string StarkbankPurchaseBillingZipCode = "billingZipCode";
        public const string StarkbankPurchaseCardEnding = "cardEnding";
        public const string StarkbankPurchaseCardExpiration = "cardExpiration";
        public const string StarkbankPurchaseCardId = "cardId";
        public const string StarkbankPurchaseCardNumber = "cardNumber";
        public const string StarkbankPurchaseCardSecurityCode = "cardSecurityCode";
        public const string StarkbankPurchaseChallengeMode = "challengeMode";
        public const string StarkbankPurchaseChallengeUrl = "challengeUrl";
        public const string StarkbankPurchaseCreated = "created";
        public const string StarkbankPurchaseCurrencyCode = "currencyCode";
        public const string StarkbankPurchaseEndToEndId = "endToEndId";
        public const string StarkbankPurchaseFee = "fee";
        public const string StarkbankPurchaseFundingType = "fundingType";
        public const string StarkbankPurchaseHolderEmail = "holderEmail";
        public const string StarkbankPurchaseHolderId = "holderId";
        public const string StarkbankPurchaseHolderName = "holderName";
        public const string StarkbankPurchaseHolderPhone = "holderPhone";
        public const string StarkbankPurchaseId = "id";
        public const string StarkbankPurchaseInstallmentCount = "installmentCount";
        public const string StarkbankPurchaseMetadata = "metadata";
        public const string StarkbankPurchaseNetwork = "network";
        public const string StarkbankPurchaseSoftDescriptor = "softDescriptor";
        public const string StarkbankPurchaseSource = "source";
        public const string StarkbankPurchaseStatus = "status";
        public const string StarkbankPurchaseTags = "tags";
        public const string StarkbankPurchaseUpdated = "updated";
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
        public const string StarkbankTaxPaymentAmount = "amount";
        public const string StarkbankTaxPaymentBarCode = "barCode";
        public const string StarkbankTaxPaymentCreated = "created";
        public const string StarkbankTaxPaymentDescription = "description";
        public const string StarkbankTaxPaymentFee = "fee";
        public const string StarkbankTaxPaymentId = "id";
        public const string StarkbankTaxPaymentLine = "line";
        public const string StarkbankTaxPaymentLogCreated = "created";
        public const string StarkbankTaxPaymentLogErrors = "errors";
        public const string StarkbankTaxPaymentLogId = "id";
        public const string StarkbankTaxPaymentLogPayment = "payment";
        public const string StarkbankTaxPaymentLogType = "type";
        public const string StarkbankTaxPaymentScheduled = "scheduled";
        public const string StarkbankTaxPaymentStatus = "status";
        public const string StarkbankTaxPaymentStatusCanceled = "canceled";
        public const string StarkbankTaxPaymentStatusConfirmed = "confirmed";
        public const string StarkbankTaxPaymentStatusCreated = "created";
        public const string StarkbankTaxPaymentStatusFailed = "failed";
        public const string StarkbankTaxPaymentStatusProcessing = "processing";
        public const string StarkbankTaxPaymentStatusSuccess = "success";
        public const string StarkbankTaxPaymentTags = "tags";
        public const string StarkbankTaxPaymentTransactionIds = "transactionIds";
        public const string StarkbankTaxPaymentType = "type";
        public const string StarkbankTaxPaymentUpdated = "updated";
        public const string StarkbankTaxPreviewAmount = "amount";
        public const string StarkbankTaxPreviewBarCode = "barCode";
        public const string StarkbankTaxPreviewDescription = "description";
        public const string StarkbankTaxPreviewLine = "line";
        public const string StarkbankTaxPreviewName = "name";
        public const string StarkbankTransactionAmount = "amount";
        public const string StarkbankTransactionBalance = "balance";
        public const string StarkbankTransactionCreated = "created";
        public const string StarkbankTransactionDescription = "description";
        public const string StarkbankTransactionExternalId = "externalId";
        public const string StarkbankTransactionFee = "fee";
        public const string StarkbankTransactionId = "id";
        public const string StarkbankTransactionReceiverId = "receiverId";
        public const string StarkbankTransactionSenderId = "senderId";
        public const string StarkbankTransactionSource = "source";
        public const string StarkbankTransactionTags = "tags";
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
        public const string StarkbankUtilityPaymentAmount = "amount";
        public const string StarkbankUtilityPaymentBarCode = "barCode";
        public const string StarkbankUtilityPaymentCreated = "created";
        public const string StarkbankUtilityPaymentDescription = "description";
        public const string StarkbankUtilityPaymentFee = "fee";
        public const string StarkbankUtilityPaymentId = "id";
        public const string StarkbankUtilityPaymentLine = "line";
        public const string StarkbankUtilityPaymentLogCreated = "created";
        public const string StarkbankUtilityPaymentLogErrors = "errors";
        public const string StarkbankUtilityPaymentLogId = "id";
        public const string StarkbankUtilityPaymentLogPayment = "payment";
        public const string StarkbankUtilityPaymentLogType = "type";
        public const string StarkbankUtilityPaymentScheduled = "scheduled";
        public const string StarkbankUtilityPaymentStatus = "status";
        public const string StarkbankUtilityPaymentStatusCanceled = "canceled";
        public const string StarkbankUtilityPaymentStatusConfirmed = "confirmed";
        public const string StarkbankUtilityPaymentStatusCreated = "created";
        public const string StarkbankUtilityPaymentStatusFailed = "failed";
        public const string StarkbankUtilityPaymentStatusProcessing = "processing";
        public const string StarkbankUtilityPaymentStatusSuccess = "success";
        public const string StarkbankUtilityPaymentTags = "tags";
        public const string StarkbankUtilityPaymentTransactionIds = "transactionIds";
        public const string StarkbankUtilityPaymentType = "type";
        public const string StarkbankUtilityPaymentUpdated = "updated";
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
        public const string StarkbankWorkspaceAllowedTaxIds = "allowedTaxIds";
        public const string StarkbankWorkspaceCreated = "created";
        public const string StarkbankWorkspaceId = "id";
        public const string StarkbankWorkspaceName = "name";
        public const string StarkbankWorkspaceOrganizationId = "organizationId";
        public const string StarkbankWorkspacePicture = "picture";
        public const string StarkbankWorkspacePictureUrl = "pictureUrl";
        public const string StarkbankWorkspaceStatus = "status";
        public const string StarkbankWorkspaceStatusActive = "active";
        public const string StarkbankWorkspaceStatusBlocked = "blocked";
        public const string StarkbankWorkspaceStatusClosed = "closed";
        public const string StarkbankWorkspaceStatusFrozen = "frozen";
        public const string StarkbankWorkspaceUsername = "username";

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int Transport(IntPtr context, int method,
            [MarshalAs(UnmanagedType.LPStr)] string url, IntPtr headers,
            IntPtr body, UIntPtr bodyLen, int timeoutSeconds, out IntPtr response);

        [DllImport(Library, EntryPoint = "starkbank_abi_version", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankAbiVersion();

        [DllImport(Library, EntryPoint = "starkbank_allowed_installment_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankAllowedInstallmentNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_balance_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBalanceGet(IntPtr client, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoCreate(IntPtr client, IntPtr boletos, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_holmes_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoHolmesCreate(IntPtr client, IntPtr holmes, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_holmes_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoHolmesGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_holmes_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoHolmesLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_holmes_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoHolmesLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_holmes_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoHolmesLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_holmes_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoHolmesLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_holmes_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoHolmesNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_holmes_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoHolmesPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_boleto_holmes_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoHolmesParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_boleto_holmes_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankBoletoHolmesQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

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

        [DllImport(Library, EntryPoint = "starkbank_card_method_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCardMethodParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_card_method_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCardMethodQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

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

        [DllImport(Library, EntryPoint = "starkbank_corporate_balance_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateBalanceGet(IntPtr client, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_card_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateCardCreate(IntPtr client, IntPtr card, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_card_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateCardDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_card_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateCardGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_card_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateCardLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_card_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateCardLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_card_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateCardLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_card_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateCardLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_card_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateCardNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_card_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateCardPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_card_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateCardParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_card_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateCardQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_card_update", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateCardUpdate(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, IntPtr patch, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_holder_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateHolderCreate(IntPtr client, IntPtr holders, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_holder_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateHolderDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_holder_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateHolderGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_holder_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateHolderLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_holder_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateHolderLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_holder_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateHolderLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_holder_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateHolderLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_holder_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateHolderNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_holder_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateHolderPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_holder_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateHolderParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_holder_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateHolderQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_holder_update", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateHolderUpdate(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, IntPtr patch, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_invoice_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateInvoiceCreate(IntPtr client, IntPtr invoice, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_invoice_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateInvoiceNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_invoice_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateInvoicePage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_invoice_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateInvoiceParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_invoice_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateInvoiceQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_purchase_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporatePurchaseGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_purchase_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporatePurchaseLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_purchase_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporatePurchaseLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_purchase_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporatePurchaseLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_purchase_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporatePurchaseLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_purchase_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporatePurchasePage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_purchase_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporatePurchaseParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_purchase_parse", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporatePurchaseParse(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string content, UIntPtr content_len, [MarshalAs(UnmanagedType.LPStr)] string signature_base64, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_purchase_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporatePurchaseQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_purchase_response", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporatePurchaseResponse([MarshalAs(UnmanagedType.LPStr)] string status, int has_amount, double amount, [MarshalAs(UnmanagedType.LPStr)] string reason, [MarshalAs(UnmanagedType.LPStr)] string tags, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_rule_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateRuleNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_transaction_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateTransactionGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_transaction_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateTransactionPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_transaction_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateTransactionParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_transaction_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateTransactionQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_withdrawal_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateWithdrawalCreate(IntPtr client, IntPtr withdrawal, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_withdrawal_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateWithdrawalGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_withdrawal_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateWithdrawalNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_withdrawal_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateWithdrawalPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_corporate_withdrawal_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateWithdrawalParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_corporate_withdrawal_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankCorporateWithdrawalQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_darf_payment_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDarfPaymentCreate(IntPtr client, IntPtr payments, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_darf_payment_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDarfPaymentDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_darf_payment_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDarfPaymentGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_darf_payment_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDarfPaymentLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_darf_payment_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDarfPaymentLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_darf_payment_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDarfPaymentLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_darf_payment_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDarfPaymentLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_darf_payment_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDarfPaymentNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_darf_payment_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDarfPaymentPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_darf_payment_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDarfPaymentParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_darf_payment_pdf", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDarfPaymentPdf(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out UIntPtr out_len, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_darf_payment_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDarfPaymentQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_deposit_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDepositGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_deposit_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDepositLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_deposit_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDepositLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_deposit_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDepositLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_deposit_log_pdf", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDepositLogPdf(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out UIntPtr out_len, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_deposit_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDepositLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_deposit_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDepositPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_deposit_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDepositParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_deposit_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDepositQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_deposit_update", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDepositUpdate(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, IntPtr patch, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_dict_key_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDictKeyGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_dict_key_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDictKeyPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_dict_key_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDictKeyParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_dict_key_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankDictKeyQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

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

        [DllImport(Library, EntryPoint = "starkbank_institution_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInstitutionPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_institution_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInstitutionParamsNew(out IntPtr @out);

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

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_request_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullRequestCreate(IntPtr client, IntPtr requests, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_request_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullRequestDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_request_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullRequestGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_request_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullRequestLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_request_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullRequestLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_request_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullRequestLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_request_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullRequestLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_request_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullRequestNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_request_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullRequestPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_request_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullRequestParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_request_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullRequestQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_subscription_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullSubscriptionCreate(IntPtr client, IntPtr subscriptions, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_subscription_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullSubscriptionDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_subscription_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullSubscriptionGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_subscription_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullSubscriptionLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_subscription_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullSubscriptionLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_subscription_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullSubscriptionLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_subscription_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullSubscriptionLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_subscription_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullSubscriptionNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_subscription_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullSubscriptionPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_subscription_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullSubscriptionParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_invoice_pull_subscription_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankInvoicePullSubscriptionQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

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

        [DllImport(Library, EntryPoint = "starkbank_merchant_card_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantCardGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_card_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantCardLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_card_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantCardLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_card_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantCardLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_card_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantCardLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_card_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantCardPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_card_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantCardParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_card_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantCardQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_category_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantCategoryParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_category_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantCategoryQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_country_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantCountryParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_country_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantCountryQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_installment_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantInstallmentGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_installment_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantInstallmentLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_installment_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantInstallmentLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_installment_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantInstallmentLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_installment_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantInstallmentLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_installment_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantInstallmentPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_installment_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantInstallmentParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_installment_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantInstallmentQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_purchase_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantPurchaseCreate(IntPtr client, IntPtr purchase, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_purchase_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantPurchaseGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_purchase_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantPurchaseLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_purchase_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantPurchaseLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_purchase_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantPurchaseLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_purchase_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantPurchaseLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_purchase_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantPurchaseNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_purchase_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantPurchasePage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_purchase_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantPurchaseParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_purchase_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantPurchaseQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_purchase_update", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantPurchaseUpdate(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, IntPtr patch, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_session_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantSessionCreate(IntPtr client, IntPtr session, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_session_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantSessionGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_session_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantSessionLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_session_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantSessionLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_session_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantSessionLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_session_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantSessionLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_session_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantSessionNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_session_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantSessionPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_session_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantSessionParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_merchant_session_purchase", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantSessionPurchase(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string uuid, IntPtr purchase, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_merchant_session_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankMerchantSessionQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

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

        [DllImport(Library, EntryPoint = "starkbank_permission_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankPermissionNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_project_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankProjectNew([MarshalAs(UnmanagedType.LPStr)] string id, int environment, [MarshalAs(UnmanagedType.LPStr)] string private_key_pem, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_purchase_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankPurchaseNew(out IntPtr @out);

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

        [DllImport(Library, EntryPoint = "starkbank_tax_payment_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTaxPaymentCreate(IntPtr client, IntPtr payments, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_tax_payment_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTaxPaymentDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_tax_payment_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTaxPaymentGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_tax_payment_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTaxPaymentLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_tax_payment_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTaxPaymentLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_tax_payment_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTaxPaymentLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_tax_payment_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTaxPaymentLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_tax_payment_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTaxPaymentNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_tax_payment_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTaxPaymentPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_tax_payment_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTaxPaymentParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_tax_payment_pdf", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTaxPaymentPdf(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out UIntPtr out_len, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_tax_payment_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTaxPaymentQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_transaction_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransactionGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_transaction_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransactionPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_transaction_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransactionParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_transaction_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankTransactionQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

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

        [DllImport(Library, EntryPoint = "starkbank_utility_payment_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUtilityPaymentCreate(IntPtr client, IntPtr payments, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_utility_payment_delete", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUtilityPaymentDelete(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_utility_payment_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUtilityPaymentGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_utility_payment_log_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUtilityPaymentLogGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_utility_payment_log_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUtilityPaymentLogPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_utility_payment_log_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUtilityPaymentLogParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_utility_payment_log_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUtilityPaymentLogQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_utility_payment_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUtilityPaymentNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_utility_payment_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUtilityPaymentPage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_utility_payment_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUtilityPaymentParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_utility_payment_pdf", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUtilityPaymentPdf(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out UIntPtr out_len, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_utility_payment_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankUtilityPaymentQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

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

        [DllImport(Library, EntryPoint = "starkbank_workspace_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWorkspaceCreate(IntPtr client, IntPtr workspace, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_workspace_get", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWorkspaceGet(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, out IntPtr @out, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_workspace_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWorkspaceNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_workspace_page", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWorkspacePage(IntPtr client, IntPtr @params, out IntPtr @out, out IntPtr out_cursor, out IntPtr errors);

        [DllImport(Library, EntryPoint = "starkbank_workspace_params_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWorkspaceParamsNew(out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_workspace_query", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWorkspaceQuery(IntPtr client, IntPtr @params, int limit, out IntPtr @out);

        [DllImport(Library, EntryPoint = "starkbank_workspace_update", CallingConvention = CallingConvention.Cdecl)]
        public static extern int StarkbankWorkspaceUpdate(IntPtr client, [MarshalAs(UnmanagedType.LPStr)] string id, IntPtr patch, out IntPtr @out, out IntPtr errors);

    }
}
