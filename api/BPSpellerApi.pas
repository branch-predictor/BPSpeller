unit BPSpellerApi;

interface

uses Windows, Sysutils, Classes, ActiveX;

type
  PPWideChar = ^PWideChar;

const
  BPSPELLER_OK = 0;

// nullptr passed to api
  BPSPELLER_BAD_CTX = -1;

// system speller has no support for specified language
  BPSPELLER_LANG_NOT_SUPPORTED = -2;

// system speller api error (general)
  BPSPELLER_LANG_IFACE_ERROR = -3;

// CheckWord/Suggest on ctx without prior SetSpellerLanguage call
  BPSPELLER_LANG_NOT_SET = -4;

// The word passed is incorrect
  BPSPELLER_BAD_WORD = -5;

// Invalid arguments were passed
  BPSPELLER_INVALID_ARGS = -6;

function InitializeSpeller: pointer;
  cdecl; external 'BPSpeller.dll';
function SetSpellerLanguage(ctx: pointer; lang: PWideChar): integer;
  cdecl; external 'BPSpeller.dll';
function CheckWord(ctx: pointer; word: PWideChar): integer;
  cdecl; external 'BPSpeller.dll';
function GetSuggestions(ctx: pointer; tocheck: PWideChar; var suggestions: PPWideChar; var num: integer): integer;
  cdecl; external 'BPSpeller.dll';
function FreeSpeller(ctx: pointer): integer;
  cdecl; external 'BPSpeller.dll';
procedure FreeSuggestions(ctx: pointer);
  cdecl; external 'BPSpeller.dll';

implementation

// todo: dynamic loading

end.
