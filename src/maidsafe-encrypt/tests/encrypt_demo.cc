/*******************************************************************************
 *  Copyright 2011 maidsafe.net limited                                        *
 *                                                                             *
 *  The following source code is property of maidsafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the license   *
 *  file LICENSE.TXT found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of maidsafe.net. *
 ***************************************************************************//**
 * @file  encrypt_demo.cc
 * @brief Main program for Encrypt demo app.
 * @date  2011-03-05
 */

#include <array>
#include <stdio.h>
#include <string>
#include <set>

#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"
#include "maidsafe/common/utils.h"
#include "maidsafe-encrypt/self_encryption.h"
#include "maidsafe-encrypt/utils.h"

namespace fs = boost::filesystem;
namespace mse = maidsafe::encrypt;

namespace maidsafe {

namespace encrypt {

namespace demo {

enum ReturnCodes {
  kSuccess = 0,
  kNoArgumentsError,
  kCommandError,
  kInvalidArgumentsError,
  kGenerateError,
  kEncryptError
};

/// Formats and scales a byte value with IEC units
std::string FormatByteValue(const std::uint64_t &value) {
  const std::array<std::string, 7> kUnits = {
      "Bytes", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"
  };
  double val(value);
  size_t mag(0);
  while (mag < kUnits.size() && val >= 1000.0) {
    ++mag;
    val /= 1024.0;
  }
  return (boost::format("%.3g %s") % val % kUnits[mag]).str();
}

int Generate(const int &chunk_size,
             const std::string &pattern,
             const fs::path &file_name) {
  if (chunk_size < 1) {
    printf("Error: Chunk size must be bigger than zero.\n");
    return kGenerateError;
  }

  std::string content;
  for (size_t i = 0; i < pattern.size(); ++i)
    if (pattern[i] == '#')
      content.append(RandomString(chunk_size));
    else
      content.append(chunk_size, pattern[i]);

  if (!utils::WriteFile(file_name, content)) {
    printf("Error: Could not write contents to file '%s'.\n",
           file_name.c_str());
    return kGenerateError;
  }

  return kSuccess;
}

int Encrypt(const fs::path &input_path, const fs::path &output_path,
            const SelfEncryptionParams &self_encryption_params) {
  if (!fs::exists(input_path) || fs::is_directory(input_path)) {
    printf("Error: Encryption input file not found.\n");
    return kEncryptError;
  }

  if (!fs::exists(output_path) || !fs::is_directory(output_path)) {
    printf("Error: Encryption output directory not found.\n");
    return kEncryptError;
  }

  bool error(false);
  std::uint64_t total_size(0), chunks_size(0), meta_size(0);
  std::set<std::string> chunks;

  printf("Processing %s ...\n", input_path.c_str());
  DataMap data_map;
  if (SelfEncrypt(input_path, output_path, self_encryption_params, &data_map)
                                                              == kSuccess) {
    total_size += data_map.size;
    meta_size += sizeof(DataMap) + data_map.content.size();
    for (auto it = data_map.chunks.begin(); it != data_map.chunks.end(); ++it) {
      meta_size += sizeof(ChunkDetails) + it->hash.size() +
                   it->pre_hash.size() + it->content.size();
      if (!it->hash.empty() && chunks.count(it->hash) == 0) {
        chunks.insert(it->hash);
        chunks_size += it->size;
      }
    }
  } else {
    printf("Error: Self-encryption failed for %s\n", input_path.c_str());
    error = true;
  }

  double chunk_ratio(0), meta_ratio(0);
  if (total_size > 0) {
    chunk_ratio = 100.0 * chunks_size / total_size;
    meta_ratio = 100.0 * meta_size / total_size;
  }

  printf("\nResults:\n"
         "  Max chunk size: %s\n"
         "  Data processed: %s\n"
         "  Unique chunks:  %u\n"
         "  Size of chunks: %s (%.1f%%)\n"
         "+ Meta data size: %s (%.1f%%)\n"
         "= Space required: %s (%.1f%%)\n",
         FormatByteValue(self_encryption_params.max_chunk_size).c_str(),
         FormatByteValue(total_size).c_str(), chunks.size(),
         FormatByteValue(chunks_size).c_str(), chunk_ratio,
         FormatByteValue(meta_size).c_str(), meta_ratio,
         FormatByteValue(chunks_size + meta_size).c_str(),
         chunk_ratio + meta_ratio);

  return error ? kEncryptError : kSuccess;
}

}  // namespace demo

}  // namespace encrypt

}  // namespace maidsafe

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Demo application for MaidSafe-Encrypt\n\n"
           "Usage: %s <command> [<argument>...]\n\n"
           "The following commands are available:\n"
           "  generate <chunk-size> <pattern> <file-name>\n"
           "    Generates a file by writing chunks of the given size according "
                "to a\n    pattern, in which each character represents the "
                "chunk contents. The given\n    character gets repeated, with "
                "the exception of '#', which results in a\n    random chunk. "
                "Example: \"gen 128 aabaa#aab file.dat\"\n"
           "  encrypt <input-file> <output-dir> [<chunk-sz> <inc-chunk-sz> "
              "<inc-data-sz>]\n"
           "    Applies self-encryption to the given file, with chunks being "
                "stored in the\n    given output directory. Optional "
                "parameters, in order, are:\n    - maximum chunk size (bytes)\n"
                "    - maximum includable chunk size (bytes)\n    - maximum "
                "includable data size (bytes)\n    Example: \"encrypt file.dat "
                "chunks/ 262144 256 1024\"\n"
           // "  encrypt <input-dir> <output-dir> [<chunk-sz> <inc-chunk-sz> "
           //   "<inc-data-sz>]\n"
           // "    Like above, but for each file in the given input directory "
           //     "(recursive).\n"
           ,
           argv[0]);
    return mse::demo::kNoArgumentsError;
  }

  std::string command(boost::to_lower_copy(std::string(argv[1])));
  if (command == "generate") {
    if (argc == 5) {
      int chunk_size(0);
      try {
        chunk_size = boost::lexical_cast<int>(std::string(argv[2]));
      }
      catch(...) {}
      return mse::demo::Generate(chunk_size, argv[3], argv[4]);
    }
  } else if (command == "encrypt") {
    if (argc == 4) {
      mse::SelfEncryptionParams sep;
      return mse::demo::Encrypt(argv[2], argv[3], sep);
    } else if (argc == 7) {
      try {
        mse::SelfEncryptionParams sep(
            boost::lexical_cast<std::uint32_t>(std::string(argv[4])),
            boost::lexical_cast<std::uint32_t>(std::string(argv[5])),
            boost::lexical_cast<std::uint32_t>(std::string(argv[6])));
        return mse::demo::Encrypt(argv[2], argv[3], sep);
      }
      catch(...) {
        printf("Error: Invalid size arguments passed.\n");
        return mse::demo::kInvalidArgumentsError;
      }
    }
  } else {
    printf("Error: Unrecognised command '%s'.\n", command.c_str());
    return mse::demo::kCommandError;
  }

  printf("Error: Wrong number of arguments supplied to command '%s' (%d).\n",
         command.c_str(), argc - 2);
  return mse::demo::kInvalidArgumentsError;
}