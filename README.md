### Environment Setup
Please run `setup/setup.sh`

### Testing

Please run `bazel test //:hashmap_test`

### Benchmarking

Please run `bazel run -c opt //:hashmap_bench`

### Development

Recommended to install the `clangd` extension for VS Code to serve as C++ language server.
May need to update the `clangd.path` setting for the extension, e.g.
```json
// .vscode/settings.json
{
  "clangd.path": "clangd-18",
}
```
Running `bazel-compile-commands` will regenerate the `compile_commands.json` used by `clangd`.