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
#include <cstring>
#include <fstream>
#include <iostream>
#include <tuple>
#include <vector>

void
read_words(const std::string &filename, bool dedupe_anagrams,
           std::vector<uint32_t> &bitmasks,
           std::vector<std::vector<std::string>> &words)
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

      data.push_back(std::make_pair(bitmask, word));
      dups[bitmask] = true;
   }

   std::sort(data.begin(), data.end());
   for (unsigned i = 0; i < data.size(); ++i) {
      if (i && data[i].first == data[i - 1].first) {
         if (!dedupe_anagrams)
            words.back().push_back(data[i].second);
      } else {
         bitmasks.push_back(data[i].first);
         words.push_back({data[i].second});
      }
   }
}

void
search(std::vector<const std::string *> &stack, unsigned mask,
       const std::vector<std::tuple<uint32_t, unsigned, int>> &results,
       const std::vector<int> &result_ptrs,
       const std::vector<uint32_t> &bitmasks,
       const std::vector<std::vector<std::string>> &words)
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

   int r = result_ptrs[mask];
   while (r >= 0) {
      auto e = std::get<1>(results[r]);
      r = std::get<2>(results[r]);
      for (const auto &w : words[e]) {
         stack.push_back(&w);
         search(stack, mask & ~bitmasks[e], results, result_ptrs, bitmasks,
                words);
         stack.pop_back();
      }
   }
}

int
main(int argc, char *argv[])
{
   if (argc < 2 || argc > 3) {
      std::cerr << "Expected usage: ./wordle-finder [--dedupe-anagrams] "
                   "word-list-file\n";
      return 1;
   }

   if (argc == 3 && std::strcmp(argv[1], "--dedupe-anagrams") != 0) {
      std::cerr << "Invalid option \"" << argv[1]
                << "\", expected --dedupe-anagrams\n";
      return 1;
   }

   std::vector<uint32_t> word_bitmasks;
   std::vector<std::vector<std::string>> words;

   bool dedupe_anagrams = argc == 3;
   read_words(argv[argc - 1], dedupe_anagrams, word_bitmasks, words);

   // The core Dynamic Programming state. has_solution[i] states whether
   // there is a solution that uses exactly the characters from mask i.
   std::vector<bool> has_solution(1u << 26);
   has_solution[0] = true;

   std::vector<int> inner_loop_bounds(1u << 26);
   for (int i = 1, j = 0; i < (int)inner_loop_bounds.size(); ++i) {
      while (j < word_bitmasks.size() &&
             __builtin_clz(word_bitmasks[j]) >= __builtin_clz(i))
         ++j;
      inner_loop_bounds[i] = j;
   }

   std::vector<std::tuple<uint32_t, unsigned, int>> results;
   std::vector<int> result_ptrs(1u << 26, -1);

   for (unsigned i = 0; i < (1u << 26); ++i) {
      if (!has_solution[i])
         continue;

      unsigned size = word_bitmasks.size();
      int bound = inner_loop_bounds[i];
      for (int j = word_bitmasks.size() - 1; j >= bound; --j) {
         if (word_bitmasks[j] & i)
            continue;

         uint32_t new_mask = i | word_bitmasks[j];
         has_solution[new_mask] = true;
         int r = results.size();
         results.push_back(std::tuple<uint32_t, unsigned, int>(
            new_mask, j, result_ptrs[new_mask]));
         result_ptrs[new_mask] = r;
      }
   }

   // We still end up doing a recursive search, but due to our
   // precomputation it never does an useless iteration, which makes
   // the time taken negligible.
   for (unsigned i = 0; i < 26; ++i) {
      uint32_t mask = ((1u << 26) - 1) & ~(1u << i);
      std::vector<const std::string *> stack;
      search(stack, mask, results, result_ptrs, word_bitmasks, words);
   }
   return 0;
}
