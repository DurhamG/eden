/*
 *  Copyright (c) 2016-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "GitIgnore.h"

#include <algorithm>
#include "GitIgnorePattern.h"

using folly::ByteRange;
using folly::StringPiece;
using std::string;

namespace facebook {
namespace eden {

GitIgnore::GitIgnore() {}

GitIgnore::~GitIgnore() {}

void GitIgnore::loadFile(StringPiece contents) {
  std::vector<GitIgnorePattern> newRules;

  const char* currentPos = contents.begin();

  // Skip over any leading UTF-8 byte order marker
  if (contents.size() >= 3 && contents[0] == '\xef' && contents[1] == '\xbb' &&
      contents[2] == '\xbf') {
    currentPos += 3;
  }

  // Parse the file line-by-line
  while (currentPos < contents.end()) {
    const char* nextNewline = reinterpret_cast<const char*>(
        memchr(currentPos, '\n', contents.end() - currentPos));
    if (nextNewline == nullptr) {
      // git honors the final line even if it does not end with a newline
      nextNewline = contents.end();
    }

    auto line = StringPiece(currentPos, nextNewline);
    auto pattern = GitIgnorePattern::parseLine(line);
    if (pattern.hasValue()) {
      // Create a GitIgnorePattern and add it to the rules vector.
      newRules.emplace_back(std::move(pattern).value());
    }

    // Note that currentPos might end up pointing one past contents.end() here
    // (if the file does not end in a newline).  That's okay since we won't
    // ever try to dereference it if the (currentPos < contents.end()) check
    // fails at the start of the next loop.
    currentPos = nextNewline + 1;
  }

  // Reverse the loaded patterns.
  // Patterns in the gitignore file follow "last match wins" behavior.  We
  // reverse them so that we can do a forward walk through our patterns and
  // stop at the first match.
  std::reverse(newRules.begin(), newRules.end());
  std::swap(rules_, newRules);
}

GitIgnore::MatchResult GitIgnore::match(
    RelativePathPiece path,
    PathComponentPiece basename) const {
  for (const auto& pattern : rules_) {
    auto result = pattern.match(path, basename);
    if (result != NO_MATCH) {
      return result;
    }
  }

  return NO_MATCH;
}

string GitIgnore::matchString(MatchResult result) {
  switch (result) {
    case EXCLUDE:
      return "exclude";
    case INCLUDE:
      return "include";
    case NO_MATCH:
      return "no match";
    case HIDDEN:
      return "hidden";
  }
  return folly::to<string>("unexpected result", int(result));
}
}
}
