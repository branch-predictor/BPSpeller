// BPSPELLer.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "BPSpeller.h"

BPSPELLER_API BPSpellerCtxPtr InitializeSpeller(void)
{
	BPSpellerCtxPtr ctx = (BPSpellerCtxPtr)calloc(1, sizeof(BPSpellerCtx));
	if (!ctx)
		return nullptr;

	HRESULT hr = CoCreateInstance(__uuidof(SpellCheckerFactory), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&ctx->iface));
	if (FAILED(hr))
	{
		free((void*)ctx);
		ctx = nullptr;
	}

	return ctx;
}

BPSPELLER_API int SetSpellerLanguage(BPSpellerCtxPtr ctx, wchar_t* lang)
{
	if (!ctx)
		return BPSPELLER_BAD_CTX;

	BOOL supported = FALSE;
	HRESULT hr = ctx->iface->IsSupported(lang, &supported);
	if (S_FALSE == hr) {
		return BPSPELLER_LANG_NOT_SUPPORTED;
	} else if (S_OK != hr) {
		return BPSPELLER_LANG_IFACE_ERROR;
	}

	if (ctx->spell_checker) {
		ctx->spell_checker->Release();
	}

	if (SUCCEEDED(ctx->iface->CreateSpellChecker(lang, &ctx->spell_checker))) {
		return BPSPELLER_OK;
	} 

	return BPSPELLER_LANG_IFACE_ERROR;
}

BPSPELLER_API int FreeSpeller(BPSpellerCtxPtr ctx)
{
	if (!ctx)
		return BPSPELLER_BAD_CTX;
	FreeSuggestions(ctx);
	if (ctx->last_errors)
		free((void*)(ctx->last_errors));
	if (ctx->spell_checker)
		ctx->spell_checker->Release();
	if (ctx->iface)
		ctx->iface->Release();
	free((void*)ctx);
	return BPSPELLER_OK;
}

BPSPELLER_API int CheckWord(BPSpellerCtxPtr ctx, wchar_t* tocheck)
{
	if (!ctx)
		return BPSPELLER_BAD_CTX;
	if (!ctx->spell_checker)
		return BPSPELLER_LANG_NOT_SET;
	if (!tocheck)
		return BPSPELLER_INVALID_ARGS;
	IEnumSpellingError* spell_error_enum;
	if (SUCCEEDED(ctx->spell_checker->Check(tocheck, &spell_error_enum))) {
		int spell_status = BPSPELLER_OK;
		ISpellingError* spelling_error = nullptr;
		if (S_OK == spell_error_enum->Next(&spelling_error)) {
			spell_status = BPSPELLER_BAD_WORD;
		}
		if (spelling_error)
			spelling_error->Release();
		spell_error_enum->Release();
		return spell_status;
	} else {
		return BPSPELLER_LANG_IFACE_ERROR;
	}
}

BPSPELLER_API int CheckSentence(BPSpellerCtxPtr ctx, wchar_t* tocheck, BPSpellError** errors)
{
	if (!ctx)
		return BPSPELLER_BAD_CTX;
	if (!ctx->spell_checker)
		return BPSPELLER_LANG_NOT_SET;
	if (!tocheck)
		return BPSPELLER_INVALID_ARGS;
	IEnumSpellingError* spell_error_enum;
	if (SUCCEEDED(ctx->spell_checker->Check(tocheck, &spell_error_enum))) {
		int misspelled_count = 0;
		ISpellingError* spelling_error = nullptr;
		while (S_OK == spell_error_enum->Next(&spelling_error)) {
			if (errors) {
				if (ctx->error_buf_size <= misspelled_count) {
					int newsize = (ctx->error_buf_size > 0) ? ctx->error_buf_size * 2 : 4;
					BPSpellError* newbuf = (BPSpellError*)realloc((void*)ctx->last_errors, newsize * sizeof(BPSpellError));
					if (newbuf) {
						ctx->last_errors = newbuf;
						ctx->error_buf_size = newsize;
					} else {
						spelling_error->Release();
						return BPSPELLER_LANG_IFACE_ERROR; // TODO: proper error
					}
				}

				ULONG res;
				spelling_error->get_Length(&res);
				ctx->last_errors[misspelled_count].error_length = res;

				spelling_error->get_StartIndex(&res);
				ctx->last_errors[misspelled_count].starting_at = res;

				CORRECTIVE_ACTION action;
				spelling_error->get_CorrectiveAction(&action);
				ctx->last_errors[misspelled_count].error_type = action;
			}

			misspelled_count++;
			if (spelling_error)
				spelling_error->Release();
		}
		spell_error_enum->Release();
		*errors = ctx->last_errors;
		return misspelled_count;
	} else {
		return BPSPELLER_LANG_IFACE_ERROR;
	}
}

wchar_t** SetSuggestionBuffer(BPSpellerCtxPtr ctx, size_t& max)
{
	wchar_t** ret = nullptr;
	if (ctx->last_suggestions) {
		ret = ctx->last_suggestions;
		max = ctx->suggest_buf_size;
		for (int i = 0; i < ctx->suggestion_count; ++i) {
			CoTaskMemFree(ctx->last_suggestions[i]);
		}
	} else {
		// preallocate 8 suggestions array 
		max = 8;
		ret = (wchar_t**)malloc(sizeof(wchar_t**) * max);
	}

	return ret;
}

BPSPELLER_API int GetSuggestions(BPSpellerCtxPtr ctx, wchar_t* tocheck, wchar_t*** suggestions, int* num)
{
	if (!ctx)
		return BPSPELLER_BAD_CTX;
	if (!ctx->spell_checker)
		return BPSPELLER_LANG_NOT_SET;
	if (!num || !suggestions || !tocheck)
		return BPSPELLER_INVALID_ARGS;
	
	IEnumSpellingError* spell_error_enum = nullptr;
	ISpellingError* spelling_error = nullptr;
	IEnumString* sug_enum = nullptr;
	CORRECTIVE_ACTION action = CORRECTIVE_ACTION_NONE;
	int spell_status = BPSPELLER_OK;
	size_t max_num = 0;

	if (SUCCEEDED(ctx->spell_checker->Check(tocheck, &spell_error_enum))) {
		if (S_OK == spell_error_enum->Next(&spelling_error)) {
			if (SUCCEEDED(spelling_error->get_CorrectiveAction(&action))) {
				switch (action) {
				case CORRECTIVE_ACTION_GET_SUGGESTIONS:
					size_t actual_num;
					actual_num = 0;
					*suggestions = SetSuggestionBuffer(ctx, max_num);
					if (*suggestions) {
						HRESULT hr = ctx->spell_checker->Suggest(tocheck, &sug_enum);
						if (S_OK == hr) {
							// check suggestions only for misspelled words
							LPOLESTR str = nullptr;
							while (sug_enum->Next(1, &str, nullptr) == S_OK) {
								(*suggestions)[actual_num++] = str;
								if (actual_num >= max_num) {
									max_num *= 2;
									wchar_t** new_suggestions = (wchar_t**)realloc((void*)(*suggestions), sizeof(wchar_t**) * max_num);
									if (new_suggestions) {
										*suggestions = new_suggestions;
									} else {
										// we ran out of mem while trying to realloc memory
										// keep old result and pass it to caller
										spell_status = BPSPELLER_LANG_IFACE_ERROR;
										*num = actual_num;
										ctx->suggestion_count = actual_num;
										ctx->suggest_buf_size = max_num / 2;
										goto exit;
									}
								}
								str = nullptr;
							}
						} else if (S_FALSE == hr) {
							// you shouldn't ask for suggestions on valid word...
							spell_status = BPSPELLER_INVALID_ARGS;
						} else {
							// no idea how to handle that error.
							spell_status = BPSPELLER_LANG_IFACE_ERROR;
						}
						*num = actual_num;
						ctx->suggestion_count = actual_num;
						ctx->last_suggestions = *suggestions;
						ctx->suggest_buf_size = max_num;
					} else {
						spell_status = BPSPELLER_LANG_IFACE_ERROR;
					}

					break;
				case CORRECTIVE_ACTION_REPLACE:
					// treat AutoCorrect replacement as a first and only suggestion					
					*suggestions = SetSuggestionBuffer(ctx, max_num);
					PWSTR replacement = nullptr;
					if (SUCCEEDED(spelling_error->get_Replacement(&replacement))) {
						if (*suggestions) {
							*suggestions[0] = replacement;
							*num = 1;
							ctx->suggestion_count = 1;
							ctx->last_suggestions = *suggestions;
						} else {
							CoTaskMemFree(replacement);
							spell_status = BPSPELLER_LANG_IFACE_ERROR;
						}
					}
					break;
				}
				// any other cases aree unsupported yet
				
			}
		}
	} else {
		spell_status = BPSPELLER_LANG_IFACE_ERROR;
	}

exit:
	if (spell_error_enum)
		spell_error_enum->Release();
	if (spelling_error)
		spelling_error->Release();
	if (sug_enum)
		sug_enum->Release();

	return spell_status;
}

#define MAX_BATCH 32
BPSPELLER_API int GetSupportedLanguages(BPSpellerCtxPtr ctx, wchar_t** destbuf, size_t bufsize)
{
	if (!ctx)
		return BPSPELLER_BAD_CTX;
	if (destbuf && (!bufsize))
		return BPSPELLER_INVALID_ARGS;
	IEnumString* langs = nullptr;
	int retval = 0;
	if (SUCCEEDED(ctx->iface->get_SupportedLanguages(&langs))) {
		ULONG fetched = 0;
		if (destbuf) {
			// we got dest buffer, copy as many ptrs as possible
			// ignore result, because it can return S_FALSE if it returns less
			// strings than buffer can hold.
			langs->Next(bufsize / sizeof(wchar_t), destbuf, &fetched);
			if (fetched > 0) {
				// swap suggestion buffer with supported languages list, so user don't have to
				// separately free it immediately
				FreeSuggestions(ctx);
				ctx->last_suggestions = (wchar_t**) malloc(sizeof(wchar_t*) * fetched);
				if (ctx->last_suggestions) {
					ctx->suggestion_count = fetched;
					ctx->suggest_buf_size = fetched;
					memcpy((void*)ctx->last_suggestions, (void*)destbuf, sizeof(wchar_t*) * fetched);
					retval = fetched;
				} else {
					// free result. we either leak memory or return error... I prefer error.
					for (ULONG i = 0; i < fetched; ++i) {
						CoTaskMemFree(destbuf[i]);
						memset(destbuf, 0, bufsize);
					}
					retval = BPSPELLER_LANG_IFACE_ERROR;
				}
			}
		} else {
			// go slowly through list and free each string immediately
			LPOLESTR strings[MAX_BATCH];
			while (langs->Next(MAX_BATCH, &strings[0], &fetched) == S_OK) {
				retval += fetched;
				for (ULONG i = 0; i < fetched; ++i) {
					CoTaskMemFree(strings[i]);
				}
				fetched = 0;
			}
			if (fetched) {
				for (ULONG i = 0; i < fetched; ++i) {
					CoTaskMemFree(strings[i]);
				}
				retval += fetched;
			}
		}
		langs->Release();
	}
	return retval;
}

BPSPELLER_API void FreeSuggestions(BPSpellerCtxPtr ctx)
{
	if (!ctx || !ctx->last_suggestions)
		return;
	for (int i = 0; i < ctx->suggestion_count; ++i) {
		CoTaskMemFree(ctx->last_suggestions[i]);
	}
	free((void*)ctx->last_suggestions);
	ctx->last_suggestions = nullptr;
	ctx->suggestion_count = 0;
}

