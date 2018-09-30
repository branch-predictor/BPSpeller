unit BPSpellerApi;

// Remove dot (define BPSPELLER_STATIC) to have your app link statically
// (during startup) to BPSpeller.dll and make it unable to run without it.
// Keep the dot (undefine BPSPELLER_STATIC) to try to load it on first
// call to InitializeSpeller.
{.$DEFINE BPSPELLER_STATIC}

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

  BPSpellerDll = 'BPSpeller.dll';
  
type
  TBPSpellerError = packed record
    error_type: integer;
    starting_at: integer;
    error_length: integer;
  end;
  PBPSpellerError = ^TBPSpellerError;

{$IFDEF BPSPELLER_STATIC}
function InitializeSpeller: pointer;
  cdecl; external BPSpellerDll;
function SetSpellerLanguage(ctx: pointer; lang: PWideChar): integer;
  cdecl; external BPSpellerDll;
function CheckWord(ctx: pointer; word: PWideChar): integer;
  cdecl; external BPSpellerDll;
function CheckSentence(ctx: pointer; sentence: PWideChar; var errors: PBPSpellerError): integer;
  cdecl; external BPSpellerDll;
function GetSuggestions(ctx: pointer; tocheck: PWideChar; var suggestions: PPWideChar; var num: integer): integer;
  cdecl; external BPSpellerDll;
function FreeSpeller(ctx: pointer): integer;
  cdecl; external BPSpellerDll;
procedure FreeSuggestions(ctx: pointer);
  cdecl; external BPSpellerDll;
{$ELSE}
var
 SetSpellerLanguage: function(ctx: pointer; lang: PWideChar): integer; cdecl;
 CheckWord: function (ctx: pointer; word: PWideChar): integer; cdecl;
 CheckSentence: function (ctx: pointer; sentence: PWideChar; var errors: PBPSpellerError): integer;;
 GetSuggestions: function(ctx: pointer; tocheck: PWideChar; var suggestions: PPWideChar; var num: integer): integer; cdecl;
 FreeSpeller: function(ctx: pointer): integer; cdecl;
 FreeSuggestions: procedure(ctx: pointer); cdecl;

function InitializeSpeller: pointer;
{$ENDIF}

implementation

{$IFNDEF BPSPELLER_STATIC}
var
 _bplibhandle: THandle = 0;
 _InitializeSpeller: function: pointer; cdecl;

function InitializeSpeller: pointer;
var
 bad_dll: boolean;

	function ImportProc(const name: string): Pointer;
	begin;
	result:=GetProcAddress(_bplibhandle, pointer(name));
	if result=nil then
		bad_dll:=true;
	end;

begin;
// it won't work on Windows 7 and older, don't bother trying.
if (Win32MajorVersion < 6) or (Win32MajorVersion = 6) and (Win32MinorVersion < 2) then
	begin;
	result:=nil;
	exit;
	end;
// try to load and import functions
bad_dll:=false;
if _bplibhandle = 0 then
	begin;
	_bplibhandle:=LoadLibrary(BPSpellerDll);
	if _bplibhandle=0 then
		begin;
		result:=nil;
		exit;
		end;
	_InitializeSpeller:=ImportProc('InitializeSpeller');
	SetSpellerLanguage:=ImportProc('SetSpellerLanguage');
	CheckWord:=ImportProc('CheckWord');
	CheckSentence:=ImportProc('CheckSentence');
	GetSuggestions:=ImportProc('GetSuggestions');
	FreeSpeller:=ImportProc('FreeSpeller');
	FreeSuggestions:=ImportProc('FreeSuggestions');
	if bad_dll then
		begin;
		// not all imported, bail out
		FreeLibrary(_bplibhandle);
		_bplibhandle:=0;
		result:=nil;
		exit;
		end;
	end;
result:=_InitializeSpeller;
end;
{$ENDIF}

end.
