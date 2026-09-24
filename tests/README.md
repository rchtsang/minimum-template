# Student Tests

`hw1/` contains the initial public black-box tests. They run the packaged student
image and do not link against internal kernel functions. Later homework tests
use sibling directories such as `hw2/` without adding new Just recipes.

Run all released tests from the repository root:

```sh
just test-all hw1
```

Run one public test by homework and manifest name, for example:

```sh
just test hw1 echo
```

Students may add manifests or other tests under `tests/` using any reasonable
layout. Additional student-authored tests are encouraged but are not required
for Assignment 1.

These tests are deliberately small behavioral checks. In particular, the echo
case does not attempt to distinguish command output from every possible input
echo implementation; required command dispatch remains subject to source
review.
