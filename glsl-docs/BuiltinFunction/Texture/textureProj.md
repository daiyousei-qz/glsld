Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.