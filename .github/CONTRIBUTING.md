# Contributing to HorizonOS

Thank you for your interest in contributing to HorizonOS! Whether you're fixing bugs, adding features, improving documentation, or just asking questions, your contributions are welcome.

> [!TIP]
> Contributions in C++ and other languages outside of C, Assembly, and Shell are **not** accepted into the project at this time.

## Getting Started

### Prerequisites

Before you can build and work on HorizonOS, you'll need:

- **A UNIX-like development environment**
    * MacOS (Newer versions preferred)
    * Linux
- **Build tools:**
    * `i686-elf-gcc` toolchain
    * `make`
    * `xorriso`
    * `grub`
- **QEMU** for testing (or real hardware if you're brave)
    * the `qemu-system` target is `i386`
- **Git** for version control
- **Doxygen** for documentation generation

### Building HorizonOS
```bash
git clone https://github.com/horizonos-project/horizon.git
cd horizon
make raw # or make iso
make run # or make run-iso or make debug
```

See the main [SETUP](../SETUP.md) document for detailed build instructions.

## Ways to Contribute

### Reporting Bugs

Found something broken? Open an issue with:
- **What you expected to happen**
- **What actually happened**
- **Steps to reproduce** (if possible)
- **Your build environment** (OS, compiler version, etc.)
- **Debug information** (if applicable)

Remember to tag it with `bug` !

If your issue is a security issue, label it with `security` and `priority` !

Yes, we handle security issues publicly. This project is incredibly small so its impact is almost nil.

### Suggesting Features

Have an idea? Open an issue labeled `enhancement` and describe:
- **What problem it solves**
- **How it might work**
- **Why it's useful for HorizonOS**

Keep in mind HorizonOS is experimental and focused on learning/exploration, so not every feature may fit the project's goals.

### Submitting Code

#### Before You Start

1. **Check existing issues/PRs** to avoid duplicate work
2. **Discuss major changes** in an issue first - saves everyone time
3. **Keep changes focused** - one PR per feature/fix

> [!NOTE]
> Developing an OS is a highly technical process, code that does not compile will NOT be accepted into the kernel.

#### Code Style

- **C code:** Follow the existing style (K&R, 4-space indentation)
    * The code should compile under the `c11` standard, do not use any compiler-specific extensions!
- **Assembly:** Keep it readable with comments explaining non-obvious behavior
- **Comments:** Explain *why*, not *what* (code shows what)
- **Naming:** Use clear, descriptive names (`pmm_alloc_frame` not `paf`)

#### Pull Request Process

1. **Fork** the repository and create a branch (`feature/cool-thing` or `fix/broken-thing`)
2. **Write clear commits:** Each commit should do one logical thing

```
Good: "pmm: Fix off-by-one error in frame allocation"
Bad: "fixed stuff"
```
3. **Test your changes** in QEMU (and on real hardware if possible)
4. **Update documentation** if you're changing behavior or adding features
5. **Open a PR** with:
   - Clear description of what changed and why
   - Reference any related issues (`Fixes #123`)
   - Screenshots/logs if relevant

### Documentation

Documentation contributions are **just as valuable** as code! You can help by:
- Improving existing docs (clarity, accuracy, examples)
- Writing guides or tutorials
- Documenting undocumented features
- Fixing typos and grammar

We also use Doxygen for docs, be sure to review your new docs in a new doxygen build before contributing.

## Code of Conduct

**Be respectful.** We're all here to learn and build cool stuff. Everyone was a beginner once.

- Ask questions - there are no "stupid" questions in OS development
- Give constructive feedback
- Be patient with newcomers
- Share knowledge generously

**Don't:**
- Be dismissive or condescending
- Submit low-effort spam PRs
- Ignore feedback on your contributions

## Questions?

Not sure about something? Ask!

- **Open an issue** with the `question` label
- **Start a discussion** if you want broader community input
- **Check existing issues** - someone may have already asked

## License

By contributing to HorizonOS, you agree that your contributions will be licensed under the same license as the project (see [LICENSE](../LICENSE)).

---

**Happy hacking!**

Every contribution, no matter how small, helps make HorizonOS better. We're excited to see what you build!