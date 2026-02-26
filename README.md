
# UFS370

**UFS370** is a Unix-like virtual file system for IBM MVS 3.8j, originally created by Michael Dean Rayborn. It provides a hierarchical file system with directories, files, symbolic links, and access control on top of MVS DASD, enabling Unix-style file I/O for applications running on Hercules-emulated MVS systems.

This project is maintained as part of the [mvslovers](https://github.com/mvslovers) community.

## Features

- Hierarchical directory structure with Unix-style paths
- File and directory operations (create, read, write, delete, rename)
- Symbolic links
- Access control (user/group/other permissions)
- Block-level disk management with pager/cache
- Device abstraction layer
- DASD-backed storage via MVS datasets
- fstab-based mount configuration

## Building

### Prerequisites

- Cross-compilation environment for MVS 3.8j
- `c2asm370` compiler (GCC-based, targeting S/370)
- `mvsasm` assembler script
- CRENT370 libraries by Mike Rayborn

### Clone

```bash
git clone --recursive https://github.com/mvslovers/ufs370.git
cd ufs370
```

### Configuration

Copy `.env.example` to `.env` and set your MVS host credentials and dataset names:

```bash
cp .env.example .env
# edit .env with your values
```

### Compile

```bash
make
make clean
```

The build pipeline is: C source → `c2asm370` → S/370 Assembly (.s) → `mvsasm` → Object decks (.o) → MVS datasets.

## Project Structure

```
src/          C source files
include/      Header files
  ufs/        Internal UFS headers (types, disk, inode, pager, etc.)
contrib/      Git submodules (SDK headers for dependencies)
scripts/      Build helper scripts
doc/          Changelog and documentation
```

## Acknowledgments

This project was created by **Michael Dean Rayborn**, whose extensive work on MVS 3.8j tooling forms the foundation of the entire MVS open-source ecosystem.

## License

This project is licensed under the [MIT License](LICENSE).
