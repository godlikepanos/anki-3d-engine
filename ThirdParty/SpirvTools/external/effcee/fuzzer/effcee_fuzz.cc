// Copyright 2019 The Effcee Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdint>
#include <cstdio>

#include "FuzzedDataProvider.h"
#include "effcee/effcee.h"

// Consumes standard input as a fuzzer input, breaks it apart into text
// and a check, then  runs a basic match.
int main(int argc, char* argv[]) {
  std::vector<uint8_t> input;
  // Read standard input into a buffer.
  {
    if (FILE* fp = freopen(nullptr, "rb", stdin)) {
      uint8_t chunk[1024];
      while (size_t len = fread(chunk, sizeof(uint8_t), sizeof(chunk), fp)) {
        input.insert(input.end(), chunk, chunk + len);
      }
      if (ftell(fp) == -1L) {
        if (ferror(fp)) {
          fprintf(stderr, "error: error reading standard input");
        }
        return 1;
      }
    } else {
      fprintf(stderr, "error: couldn't reopen stdin for binary reading");
    }
  }

  // This is very basic, but can find bugs.
  FuzzedDataProvider stream(input.data(), input.size());
  std::string text = stream.ConsumeRandomLengthString(input.size());
  std::string checks = stream.ConsumeRemainingBytesAsString();
  effcee::Match(text, checks);
  return 0;
}
