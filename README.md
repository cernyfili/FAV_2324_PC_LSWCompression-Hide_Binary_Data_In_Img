# FAV_2324_PC_LSWCompression-Hide_Binary_Data_In_Img

Semester project for **KIV/PC – Programming in ANSI C** (Digital steganography).  
The program **hides a binary payload inside an image** (PNG or BMP) using **LSB steganography** (least-significant bit) and can later **extract** it back. To reduce the amount of data stored in the image, the payload is **compressed with LZW** and protected with a **CRC32 checksum**.

Main documentation stored at [dokumentace.pdf](dokumentace.pdf)

## Features

- Hide arbitrary binary data into an image (`-h`)
- Extract hidden data from an image (`-x`)
- Supports input images:
  - **PNG** (24-bit RGB)
  - **BMP** (24-bit RGB)
- Payload pipeline:
  - payload wrap (size + signature + checksum)
  - **LZW compression**
  - embedding into image **blue channel LSB**
- Integrity verification:
  - **CRC32** to detect corruption / invalid extraction

## Data format stored in the image

The embedded stream is:

- **payload size**: 4 bytes
- **signature**: 18 bytes
- **CRC32**: 4 bytes
- **compressed payload data**: `payload size` bytes

## Build

### Requirements

- ANSI C compiler:
  - GCC (Linux/macOS/FreeBSD/Windows via MinGW), or
  - Microsoft Visual C/C++
- **libpng** (needed for PNG support)
- `make` (for provided Makefiles)

### Build (GCC)

From the repository root:

```sh
make
```

### Build (MSVC)

Use the Windows Makefile (if present in the repository as `Makefile.win`) and ensure `libpng.lib` is available/linked as expected by the build configuration.

> Note: The produced executable should be named **`stegim.exe`** (including the `.exe` suffix, even on Linux) to match the project/validation requirements.

## Usage

```sh
stegim.exe <image_filepath> -h <payload_filepath>
stegim.exe <image_filepath> -x <output_payload_filepath>
```

### Examples

**Hide data (Windows):**
```sh
stegim.exe "E:\My Work\Test\Holiday 01.png" -h secret\text.txt
```

**Extract data (Windows):**
```sh
stegim.exe "E:\Download\Beach.bmp" -x "E:\Documents\Tajná zpráva.txt"
```

**Hide data (Linux/macOS):**
```sh
stegim.exe /home/user/images/img001.png -h /home/user/doc/secret.txt
```

## Exit codes

- `0` — Success
- `1` — Wrong / missing arguments, missing files, cannot open files
- `2` — Invalid image format (not PNG/BMP 24-bit RGB)
- `3` — Payload too large for the image
- `4` — No payload found in the image
- `5` — Payload corrupted (checksum mismatch and/or decompression failed)
- `6` — Other error (allocation failure, internal error, etc.)

## Project structure (high level)

- `main.c` — CLI entry point / orchestration
- **Modules**
  - `Compression` — LZW compression/decompression
  - `Image` — PNG/BMP load/save and LSB embed/extract
  - `Payload` — payload wrapping (signature, length, CRC) + compression integration
- **utils/**
  - Binary data structures
  - LZW dictionary / trie (compression)
  - logging, memory wrappers, misc utilities

## Notes / limitations

- Only **24-bit RGB** PNG/BMP images are supported.
- Data is embedded sequentially from the **first pixel** onward (blue channel LSB).
- Any modification of pixel data (recompression, filtering, edits) may corrupt the hidden payload; CRC32 is used to detect this.

## License

Add a license if you intend to publish/reuse this project beyond coursework.
