/*
 * Real sandbox webhook bodies and Stark's signatures over them, copied byte for
 * byte from sdk-python's tests/sdk/test_event.py (TesteEventProcess) and
 * tests/sdk/test_corporate_purchase.py (TestCorporatePurchaseParseRight) at
 * 7e817fafc039d11f858ea72abda8d8c3a0e29ff2, the sha in
 * tests/reference/sdk-python.sha. Regenerate them from there; never edit by hand.
 */

#ifndef SANDBOX_SIGNED_H
#define SANDBOX_SIGNED_H

static const char signedEventContent[] =
    "{\"event\": {\"created\": \"2021-04-26T20:16:51.866857+00:00\", \"id\": "
    "\"5415223380934656\", \"log\": {\"created\": \"2021-04-26T20:16:50.9277"
    "06+00:00\", \"errors\": [], \"id\": \"4687457496858624\", \"invoice\": {\""
    "amount\": 256, \"brcode\": \"00020101021226890014br.gov.bcb.pix2567i"
    "nvoice-h.sandbox.starkbank.com/v2/afdf94b770b0458a8440a335daf77c"
    "4c5204000053039865802BR5915Stark Bank S.A.6009Sao Paulo62070503*"
    "**6304CC32\", \"created\": \"2021-04-26T20:16:50.886319+00:00\", \"des"
    "criptions\": [{\"key\": \"Field1\", \"value\": \"Something\"}], \"discount"
    "Amount\": 0, \"discounts\": [{\"due\": \"2021-05-07T09:43:15+00:00\", \""
    "percentage\": 10.0}], \"due\": \"2021-05-09T19:11:39+00:00\", \"expira"
    "tion\": 123456789, \"fee\": 0, \"fine\": 2.5, \"fineAmount\": 0, \"id\": "
    "\"5941925571985408\", \"interest\": 1.3, \"interestAmount\": 0, \"link\""
    ": \"https://cdottori.sandbox.starkbank.com/invoicelink/afdf94b770"
    "b0458a8440a335daf77c4c\", \"name\": \"Oscar Cartwright\", \"nominalAmo"
    "unt\": 256, \"pdf\": \"https://invoice-h.sandbox.starkbank.com/pdf/a"
    "fdf94b770b0458a8440a335daf77c4c\", \"status\": \"created\", \"tags\": ["
    "\"war supply\", \"invoice #1234\"], \"taxId\": \"337.451.076-08\", \"tran"
    "sactionIds\": [], \"updated\": \"2021-04-26T20:16:51.442989+00:00\"},"
    " \"type\": \"created\"}, \"subscription\": \"invoice\", \"workspaceId\": \""
    "5078376503050240\"}}";

static const char signedEventSignature[] =
    "MEUCIG69+s7bcS9pvvbwN0Rx9xtsVQcIuavfdJvAi2wtyHMdAiEAh/vtDWJjI76I"
    "cJvci1BNw10iM2qV57Jb5VUOLcQAZmY=";

static const char signedPurchaseContent[] =
    "{\"acquirerId\": \"236090\", \"amount\": 100, \"cardId\": \"5671893688385"
    "536\", \"cardTags\": [], \"endToEndId\": \"2fa7ef9f-b889-4bae-ac02-167"
    "49c04a3b6\", \"holderId\": \"5917814565109760\", \"holderTags\": [], \"i"
    "sPartialAllowed\": false, \"issuerAmount\": 100, \"issuerCurrencyCod"
    "e\": \"BRL\", \"merchantAmount\": 100, \"merchantCategoryCode\": \"bookS"
    "tores\", \"merchantCountryCode\": \"BRA\", \"merchantCurrencyCode\": \"B"
    "RL\", \"merchantFee\": 0, \"merchantId\": \"204933612653639\", \"merchan"
    "tName\": \"COMPANY 123\", \"methodCode\": \"token\", \"purpose\": \"purcha"
    "se\", \"score\": null, \"tax\": 0, \"walletId\": \"\"}";

static const char signedPurchaseSignature[] =
    "MEUCIBxymWEpit50lDqFKFHYOgyyqvE5kiHERi0ZM6cJpcvmAiEA2wwIkxcsuexh"
    "9BjcyAbZxprpRUyjcZJ2vBAjdd7o28Q=";

#endif
