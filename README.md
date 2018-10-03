# BPSpeller
Super-simple C language interface and DLL library for easy access to Windows 8+ system speller


The goal of this project is to simplify access to [Windows system speller](https://docs.microsoft.com/en-us/windows/desktop/intl/about-the-spell-checker-api) available since Windows 8. There's several spell-checking products available, including Aspell, Hunspell and SSCE, but using system speller has the following advantages:
* No need to worry about dictionaries, OS provides one for user language and also user can install more languages if needed - using familiar interface provided by OS.
* BPSpeller API is super-simple and easy to access from other languages (Delphi interface included). No COM wizardry needed.
* Libraries may be complex to build, or Windows may be unsupported at all (like Aspell). BPSpeller is super-easy to build.
* Easier on resources, they're shared thorought the system - no need to load up your process with >4MB dictionaries per applicaton.
* No license issues. No GPL bullshit, no fear of hostile takedowns by greedy corporations. BPSpeller uses MIT license and system speller is non-redistributable (integral part of OS).
* Free as in "free beer".

# What's provided?
* C library that can be compiled to either .lib or .dll and linked statically or dynamically
* Delphi interface (see api/BPSpellerApi.pas)

# What's missing?

Currently, BPSpeller is missing the following:
* Adding and removing words from dictionary.

Would be nice to have, as it would affect entire user experience - adding exception in one program would be instantly visible in other

~~* Accessing list of available languages~~

~~That's provided by the system interfaces, but left out for now~~ 

* No version checking

It probably should fail in some special way when ran on Windows 7 and earlier. Currently implemented in Delphi interface only.s

# How to build it?
* Get VS 2015 Community Edition (might work with earlier edition, assuming proper version of SDK is installed)
* Open "BPSpeller.sln"
* Pick the desired configuration and hit build
