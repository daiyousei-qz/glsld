# Should have improvement
- Don't read from disk for every include file
- Proper file ID handling and jumping
- Clean up for alpha release
- Hover on Macro
- Find references
- Go to definition
- Auto generation of lexer on compilation

# Nice to have improvement
- Have feature similar to PCH
- Delta semantic tokens

# Known issues
- `Should support builtin function documentation`
- `Test2\400.geom` Inlay hint disappear during scrolling
- Random crash
- Avoid provide completion if we are typing a declarator.
- `Test/310AofA.vert` bad AST, type constructor call is treated as function declaration.
- `Test/400.vert` bad overload resolution
- Support initializer list
- `Test/constantUnaryConversion.comp` no Hover on TYPE_TO(bo^ol)
- nonuniformEXT qualifier