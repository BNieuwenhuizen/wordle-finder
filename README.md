Five by Five word finder
========================

Solution to a random puzzle I saw online and got nerdsniped by through Twitter.

Problem statement:

Given a list of words, find all combinations of five five-letter words, that
have no repeated letters, i.e. use 25 letters from the alphabet.

Also for some reason we also have to dedupe words with the same set of letters
from the wordlist.

Source video for the problem:

[![ Can you find: five five-letter words with twenty-five unique letters? ](https://img.youtube.com/vi/_-AfhLQfb6w/0.jpg)](https://www.youtube.com/watch?v=_-AfhLQfb6w)

Other solution  that I used for comparison: https://github.com/jekstrand/wordle-finder

## Insights of solution

The key insight I used is that this can be done by dynamic programming, where the
state space is the bitset of letters used (which has `1 << 26` values), the value
per state is whether there is a solution of five-letter words with no repetition
(of course not necessarily five words if less bits are set).

So for the computation we can do the following:

1) State 0 obviously has a solution of zero words.
2) For other states, there is a solution if there is a word that is a subset of
   the state, and the state minus the letters from the word also has a solution.
   This can be found by iterating over the word list.
   

This gives an algorithm where the inner loop has less than `(1u << 26) * wordlist.size()`
iterations. We can then add the usual accounting to extract the actual solutions
from a dynamic programming algorithm.

This gives me a runtime of less than two seconds for the `words_alpha.txt`.
## Building & Running

You can build with a standard

```
   make
```

and then run with

```
   ./wordle-finder third_party/english-words/words_alpha.txt
```

for a large word list. For the wordle list you have to concatenate the [answers](
https://gist.githubusercontent.com/cfreshman/a03ef2cba789d8cf00c08f767e0fad7b/raw/28804271b5a226628d36ee831b0e36adef9cf449/wordle-answers-alphabetical.txt) and [guesses](https://gist.githubusercontent.com/cfreshman/cdcdf777450c5b5301e439061d29694c/raw/b8375870720504ecf89c1970ea4532454f12de94/wordle-allowed-guesses.txt).

