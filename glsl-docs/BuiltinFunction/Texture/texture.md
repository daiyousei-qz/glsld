Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.

For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)

For non-shadow forms: the array layer comes from the last component of P.