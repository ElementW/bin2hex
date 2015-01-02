# bin2hex

Useful tool for converting files to Intel HEX, for use with e.g. firmware flashers.

## How to build
Hardest way you've ever seen (i.e. does not require `autotools` crap):
1. Clone the repo
2. `make`

You need a C compiler that can compile C99 and understands GCC's way of passing compiler parameters (i.e. **not** MSVC) set as `make`'s `$(CC)`.

## License

GPLv3. Actually it's so small I could have made it MIT or Public Domain...