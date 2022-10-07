# How To Contribute

Thanks for showing an interest in improving LeanHRPT, these guidelines outline how you should go about contributing to the project.

## Did you find a bug?

1. Make sure the bug has not already been reported.
2. If an issue already exists, leave a thumbs up to "bump" the issue
3. If an issue doesn't exist, open a new one with a clear title and description and, if relevant, any files that cause the behaviour

## Do you have a patch that fixes a bug/adds a feature?

1. Fork this repository and put your changes on a *new branch*.
2. Make sure to use sensible commit names (no "Oops" commits)
3. Open a new pull request into master.

If you don't have a GitHub account, you can email me the patch at `xerbo (at) protonmail (dot) com`

## Coding style

The coding style of LeanHRPT is based off the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with minor modifications (see `.clang-format`), in addition to this all files should use LF line endings and end in a newline. Use American english (i.e. color, not colour).

To automate this LeanHRPT comes with [pre-commit](https://pre-commit.com/) hooks which will enforce these rules.

### Development environment

LeanHRPT is developed with VSCode with the following extensions, while not required they may make your life easier:

- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
- [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
- [CMake](https://marketplace.visualstudio.com/items?itemName=twxs.cmake)
- [Code Spell Checker](https://marketplace.visualstudio.com/items?itemName=streetsidesoftware.code-spell-checker)
- [Clang-Format](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format)

## Commit message style

- Keep titles short to prevent wrapping (descriptions exist)
- Split up large changes into multiple commits
- Never use hastags for sequential counting in commits, as this interferes with issue/PR referencing
