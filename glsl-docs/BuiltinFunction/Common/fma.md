Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:

- fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
- The precision of fma() can differ from the precision of the expression a * b + c.
- fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.

Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.