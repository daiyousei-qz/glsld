Returns the value

```
vec4(Sample_i0_j1(P, base).comp,
     Sample_i1_j1(P, base).comp,
     Sample_i1_j0(P, base).comp,
     Sample_i0_j0(P, base).comp)
```

If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.