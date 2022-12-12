
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


### Phase 3
- Bulit-in

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
- `line break '\'`
- `Hover on QualType`
- `Go to definition should link to the decl token`
- `Need a better tokenizer`
- `Should support builtin function documentation`
- `sin(1.)?` FIXED
- `Test\310.inheritMemory.frag?` FIXED
- `Test\450.geom?` FIXED
- `Test2\glsl.autosampledtextures.frag`
- `Test2\400.geom` Inlay hint disappear during scrolling
- Random crash
- Go to definition shouldn't work with default library function!!!!
- Hover on struct member is incorrect FIXED
- Hover with "invariant/flat" FIXED