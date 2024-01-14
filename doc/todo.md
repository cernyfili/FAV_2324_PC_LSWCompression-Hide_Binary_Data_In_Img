# TODO

- [x] malloc functions - counter
- [x] png reader - fix libpng
- [x] bmp_dataoffset_type reader
- [x] lzw compression
- [x] extract bmp_dataoffset_type
- [x] save payload after extract
- [x] payload prepare
- [x] payload encrypt
- [x] return right error codes
  - [x] hide_png 
  - [x] hide - file not existing 1
  - [x] prepare payload data
- [x] TESTING
  - [x] temp file for hiding
  - [x] wrong saving payload to image - image corupted
    - [x] change exclude_bmp same as hide_bmp
  - [x] payload_hide not working
- [x] MEMORY LEAKS
- [x] KOMENTARE
- [x] size ulozit do payloadu

**Secondary**

- [x] check array types (vice omezeni)
- [x] warinings resolve
- [x] sanity check all functions
- [x] komentare funkce
- [x] struktura souboru jednotna
- [x] prepsat vse na @brief
- [x] zjistit jestli is_succes check je dobry napad
- [x] zlepsit logovani chyb - at neni vsude
- [x] sjednotit result nebo is_succes
- [x] odstranit printf
- [x] odstranit magicke promenne
- [x] prepsat vsude TRACKED_FREE aby se pointer nastavil na null a predavat s &
- [x] make static all private functions
- [x] check malloc and free if used my functions
- [x] check if all varables are freed when error
- [x] dynamic array to separete file
- [x] check if functions not too big