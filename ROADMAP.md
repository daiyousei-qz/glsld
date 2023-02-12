
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

DONE!

### Phase 3
- Bulit-in

DONE!

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
- `vec3 ta = vec3(0, -.5, -time-.5);` FIXED
- `add *= exp(-t*t*.1 -id.x*id.x*.001);` FIXED
- `for(i=1.5; i>0.; i-=.01){}` FIXED
- `const float EPSILON	= 1e-3;` FIXED
- `Hover parameter` FIXED
- `line break '\'` FIXED
- `Hover on QualType` FIXED
- `Go to definition should link to the decl token` FIXED
- `Need a better tokenizer` FIXED
- `sin(1.)?` FIXED
- `Test\310.inheritMemory.frag?` FIXED
- `Test\450.geom?` FIXED
- `Test2\glsl.autosampledtextures.frag` FIXED
- Hover with "invariant/flat" FIXED
- Go to definition shouldn't work with default library function!!!! FIXED
- Hover on struct member is incorrect FIXED
- `Should support builtin function documentation`
- `Test2\400.geom` Inlay hint disappear during scrolling
- `Test2\410.geom` assert failure
- `Test2\310.frag` Semantic tokens doesn't work
- Random crash
- Avoid provide completion if last token is a type