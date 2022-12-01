
### Phase 1
- Basic compiler interface
- Treat preprocessor as comment
- File synchronization (full content)
- Document Symbol (global decl)
- Semantic Tokens (keyword/function name)
- Hover

DONE!

### Phase 2
- File synchronization (edit)
- Implement qualifiers
- Share compiled result
- Go to Declaration
- Go to Definition


### Should have LSP Feature
- Go to Declaration
- Go to Definition
- Go to Type Definition
- Hover
- Document Symbols
- Semantic Tokens
- Completion Proposals

### Nice to have LSP Feature
- Code Lens
- Folding Range
- Inlay Hint
- Signature Help
- Document Color
- Rename
- Find References
- Publish Diagnostics

# Known issues
- `int[] foo213234();` FIXED
- `switch` FIXED
- `GLSLANG 100scope.vert` FIXED
- error in constructor/declarator FIXED
- `!u;` FIXED
- `GLSLANG 310.tesc` FIXED
- `5u, 1.f` FIXED
- `const cag a0[3] = cag[3](cag(3, 2.0, true), cag(1, 5.0, true), cag(1, 9.0, false));` FIXED
- `void main() { switch(foo) {} }` FIXED