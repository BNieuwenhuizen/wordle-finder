// Copyright 2022 Bas Nieuwenhuizen
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

void
read_words(const std::string &filename, std::vector<uint32_t> &bitmasks,
           std::vector<std::string> &words)
{
   std::ifstream fin(filename);
   std::string word;

   std::vector<std::pair<std::uint32_t, std::string>> data;
   std::vector<bool> dups(1u << 26);
   while (fin >> word) {
      if (word.size() != 5)
         continue;
      uint32_t bitmask = 0;
      bool valid = true;
      for (auto c : word) {
         unsigned index = c - 'a';
         if (bitmask & (1u << index))
            valid = false;
         bitmask |= 1u << index;
      }

      if (!valid)
         continue;

      if (dups[bitmask])
         continue;

      data.push_back(std::make_pair(bitmask, word));
      dups[bitmask] = true;
   }

   std::sort(data.begin(), data.end());
   for (auto &&e : data) {
      bitmasks.push_back(e.first);
      words.push_back(e.second);
   }
}

void
sort_masks(std::vector<uint32_t> &bitmasks, std::vector<std::string> &words)
{
}
void
search(std::vector<const std::string *> &stack, unsigned mask,
       const std::vector<std::vector<unsigned>> &solutions,
       const std::vector<uint32_t> &bitmasks,
       const std::vector<std::string> &words)
{
   if (mask == 0) {
      for (unsigned i = 0; i < stack.size(); ++i) {
         if (i)
            std::cout << ", ";
         std::cout << *stack[i];
      }
      std::cout << "\n";
      return;
   }

   for (auto e : solutions[mask]) {
      stack.push_back(&words[e]);
      search(stack, mask & ~bitmasks[e], solutions, bitmasks, words);
      stack.pop_back();
   }
}

int
main(int argc, char *argv[])
{
   if (argc != 2) {
      std::cerr << "Expected usage: ./wordle-finder word-list-file\n";
      return 1;
   }

   std::vector<uint32_t> word_bitmasks;
   std::vector<std::string> words;

   read_words(argv[1], word_bitmasks, words);

   // The core Dynamic Programming state. has_solution[i] states whether
   // there is a solution that uses exactly the characters from mask i.
   std::vector<bool> has_solution(1u << 26);
   has_solution[0] = true;

   // These vectors are for figuring out the actual solutions. It contains
   // the indices of words that lead to a solution (subject to the clz
   // constraint described below).
   std::vector<std::vector<unsigned>> solutions(1u << 26);

   for (unsigned i = 0; i < (1u << 26); ++i) {
      if (!has_solution[i])
         continue;

      unsigned size = word_bitmasks.size();
      for (int j = word_bitmasks.size() - 1; j >= 0; --j) {
         if (word_bitmasks[j] <= i)
            break;

         if ((word_bitmasks[j] & i))
            continue;

         uint32_t new_mask = i | word_bitmasks[j];
         has_solution[new_mask] = true;
         solutions[new_mask].push_back(j);
      }
   }

   // We still end up doing a recursive search, but due to our
   // precomputation it never does an useless iteration, which makes
   // the time taken negligible.
   for (unsigned i = 0; i < 26; ++i) {
      uint32_t mask = ((1u << 26) - 1) & ~(1u << i);
      std::vector<const std::string *> stack;
      search(stack, mask, solutions, word_bitmasks, words);
   }
   return 0;
}
