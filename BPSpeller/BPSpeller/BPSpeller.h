#ifdef BPSPELLER_EXPORTS
#define BPSPELLER_API __declspec(dllexport)
#else
#define BPSPELLER_API __declspec(dllimport)
#endif

#include <spellcheck.h>
#include <objidl.h>

#ifdef __cplusplus
extern "C" {
#endif

struct BPSpellerCtx {
	ISpellCheckerFactory* iface;
	ISpellChecker* spell_checker;
	wchar_t** last_suggestions;
	int suggestion_count;
	int suggest_buf_size;
} ;

const int BPSPELLER_OK = 0;

// nullptr passed to api
const int BPSPELLER_BAD_CTX = -1;

// system speller has no support for specified language
const int BPSPELLER_LANG_NOT_SUPPORTED = -2;

// system speller api error (general)
const int BPSPELLER_LANG_IFACE_ERROR = -3;

// CheckWord/Suggest on ctx without prior SetSpellerLanguage call
const int BPSPELLER_LANG_NOT_SET = -4;

// The word passed is incorrect
const int BPSPELLER_BAD_WORD = -5;

// Invalid arguments were passed
const int BPSPELLER_INVALID_ARGS = -6;

typedef BPSpellerCtx* BPSpellerCtxPtr;

// Use this to create new speller context
BPSPELLER_API BPSpellerCtxPtr InitializeSpeller(void);

// pick dictionary to use with this. For example, L"pl-PL" or L"en-US".
BPSPELLER_API int SetSpellerLanguage(BPSpellerCtxPtr ctx, wchar_t* lang);

// check word for misspells with this. returns BPSPELLER_OK if correct, BPSPELLER_BAD_WORD if misspelled
BPSPELLER_API int CheckWord(BPSpellerCtxPtr ctx, wchar_t* tocheck);

// if misspelled, ask for suggestions with this
BPSPELLER_API int GetSuggestions(BPSpellerCtxPtr ctx, wchar_t* tocheck, wchar_t*** suggestions, int* num);

// get rid of speller context with this
BPSPELLER_API int FreeSpeller(BPSpellerCtxPtr ctx);

// use this to free recently acquired suggestions. 
BPSPELLER_API void FreeSuggestions(BPSpellerCtxPtr ctx);

#ifdef __cplusplus
}
#endif

