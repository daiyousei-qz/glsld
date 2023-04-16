Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λbase] and sets the partial derivatives as follows:
(See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)

∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0