﻿
/*******************************************************************************
 *  Copyright 2008-2011 maidsafe.net limited                                   *
 *                                                                             *
 *  The following source code is property of maidsafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the license   *
 *  file LICENSE.TXT found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of maidsafe.net. *
 *******************************************************************************
 * @file  utils.cc
 * @brief Helper functions for self-encryption engine.
 * @date  2008-09-09
 */

#include "maidsafe/encrypt/self_encrypt.h"

#include <omp.h>

#include <tuple>
#include <algorithm>
#include <limits>
#include <set>
#include <vector>

#ifdef __MSVC__
#  pragma warning(push, 1)
#endif
#include "cryptopp/aes.h"
#include "cryptopp/gzip.h"
#include "cryptopp/modes.h"
#ifdef __MSVC__
#  pragma warning(pop)
#endif
#include "boost/shared_array.hpp"
#include "boost/thread.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/scoped_array.hpp"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/chunk_store.h"

#include "maidsafe/encrypt/config.h"
#include "maidsafe/encrypt/data_map.h"
#include "maidsafe/encrypt/log.h"


namespace fs = boost::filesystem;

namespace maidsafe {
namespace encrypt {

/// Implementation of XOR transformation filter to allow pipe-lining
size_t XORFilter::Put2(const byte *in_string,
                       size_t length,
                       int message_end,
                       bool blocking) {
  if (length == 0) {
    return AttachedTransformation()->Put2(in_string, length, message_end,
                                          blocking);
  }
  boost::scoped_array<byte> buffer(new byte[length]);

  size_t i(0);
  size_t offset((3 * crypto::SHA512::DIGESTSIZE) - crypto::AES256_KeySize -
                crypto::AES256_IVSize);
// #pragma omp parallel for shared(buffer, in_string) private(i)
  for (; i != length; ++i) {
    buffer[i] = in_string[i] ^ pad_[count_ % offset];
    ++count_;
  }

  return AttachedTransformation()->Put2(buffer.get(), length, message_end,
                                        blocking);
}

SelfEncryptor::~SelfEncryptor() {
  ProcessMainQueue();  // to pick up unprocessed whole chunks
  EmptySequencer();
  while (retrievable_from_queue_ > 0) {
    chunk_size_ = retrievable_from_queue_ / 3;
    if (chunk_size_ < kMinChunkSize) {
      WriteExtraAndEnc0and1();
    }
    AddReleventSeqDataToQueue();
    ProcessMainQueue();
  }
  WriteExtraAndEnc0and1();
}

bool SelfEncryptor::Write(const char *data,
                          uint32_t length,
                          uint64_t position) {
  if (length == 0)
    return true;

  PrepareToWrite();

  uint32_t written = PutToInitialChunks(data, &length, &position);

  uint32_t data_offset(0), queue_offset(0);
  if (GetDataOffsetForEnqueuing(length, position, &data_offset,
                                &queue_offset)) {
    PutToEncryptQueue(data + written, length, data_offset, queue_offset);
  }

  if (GetLengthForSequencer(position, &length))
    sequencer_.Add(data + written, length, position);

//  } else {  // we went backwards or rewriting !!!
//    if (!rewriting_ && data_map_->complete) {
//      rewriting_ = true;
//      SequenceAllNonStandardChunksAndExtraContent();
//      data_map_->complete = false;
//    }
//    sequencer_.Add(position, const_cast<char*>(data), length);
//  }
////   AttemptProcessQueue();
  AddReleventSeqDataToQueue();  // gets any relevent data from sequencer
//  AttemptProcessQueue();
  return true;
}

void SelfEncryptor::PrepareToWrite() {
  if (prepared_for_writing_)
    return;

  if (!main_encrypt_queue_) {
    main_encrypt_queue_.reset(new byte[kDefaultByteArraySize_]);
    memset(main_encrypt_queue_.get(), 0, kDefaultByteArraySize_);
  }

  if (!chunk0_raw_) {
    chunk0_raw_.reset(new byte[kDefaultChunkSize]);
    memset(chunk0_raw_.get(), 0, kDefaultChunkSize);
  }

  if (!chunk1_raw_) {
    chunk1_raw_.reset(new byte[kDefaultChunkSize]);
    memset(chunk1_raw_.get(), 0, kDefaultChunkSize);
  }

  if (data_map_->content_size == 0) {
    BOOST_ASSERT(data_map_->chunks.empty() || data_map_->chunks.size() >= 3);
    uint32_t i(0);
    ByteArray temp(new byte[kDefaultChunkSize + 1023]);
    while (i != static_cast<uint32_t>(data_map_->chunks.size()) &&
            current_position_ < 2 * kDefaultChunkSize) {
      ReadChunk(i, temp.get());
      uint32_t length(data_map_->chunks[i].size);
      uint64_t position(current_position_);
      PutToInitialChunks(reinterpret_cast<char*>(temp.get()), &length,
                          &position);
      chunk_store_->Delete(std::string(
          reinterpret_cast<char*>(data_map_->chunks[i].hash),
          crypto::SHA512::DIGESTSIZE));
    }
  } else {
    uint32_t length(data_map_->content_size);
    uint64_t position(0);
    PutToInitialChunks(data_map_->content.data(), &length, &position);
  }

  data_map_->content.clear();
  data_map_->content_size = 0;
  data_map_->complete = false;
}

uint32_t SelfEncryptor::PutToInitialChunks(const char *data,
                                           uint32_t *length,
                                           uint64_t *position) {
  uint32_t copy_length0(0);
  // Handle Chunk 0
  if (*position < kDefaultChunkSize) {
    copy_length0 =
        std::min(*length, kDefaultChunkSize - static_cast<uint32_t>(*position));
    memcpy(&chunk0_raw_[static_cast<uint32_t>(*position)], data, copy_length0);
    // Don't decrease current_position_ (could be a rewrite - this shouldn't
    // change current_position_).
    if (current_position_ < copy_length0)
      current_position_ = copy_length0;
    *length -= copy_length0;
    *position += copy_length0;
  }

  // Handle Chunk 1
  uint32_t copy_length1(0);
  if ((*position >= kDefaultChunkSize) && (*position < 2 * kDefaultChunkSize)) {
    copy_length1 = std::min(*length,
        (2 * kDefaultChunkSize) - static_cast<uint32_t>(*position));
    memcpy(&chunk1_raw_[static_cast<uint32_t>(*position - kDefaultChunkSize)],
           data + copy_length0, copy_length1);
    // Don't decrease current_position_ (could be a rewrite - this shouldn't
    // change current_position_).
    if (current_position_ < kDefaultChunkSize + copy_length1)
      current_position_ = kDefaultChunkSize + copy_length1;
    *length -= copy_length1;
    *position += copy_length1;
  }

  return copy_length0 + copy_length1;
}

bool SelfEncryptor::GetDataOffsetForEnqueuing(const uint32_t &length,
                                              const uint64_t &position,
                                              uint32_t *data_offset,
                                              uint32_t *queue_offset) {
  // Cover most common case first
  if (position == current_position_) {
    *data_offset = 0;
    *queue_offset =
        static_cast<uint32_t>(current_position_ - queue_start_position_);
    return true;
  }

  if (length == 0)
    return false;

  if (position < queue_start_position_) {
    // We don't care if this overflows as in this case we return false
    *data_offset = static_cast<uint32_t>(queue_start_position_ - position);
    *queue_offset = 0;
    return (position + length >= queue_start_position_);
  }

  *data_offset = 0;
  // We don't care if this overflows as in this case we return false
  *queue_offset = static_cast<uint32_t>(position - queue_start_position_);
  return (position <= queue_start_position_ + kDefaultByteArraySize_);
}

void SelfEncryptor::PutToEncryptQueue(const char *data,
                                      uint32_t length,
                                      uint32_t data_offset,
                                      uint32_t queue_offset) {
  length -= data_offset;
  uint32_t copy_length(std::min(length,
                       kDefaultByteArraySize_ - retrievable_from_queue_));
  while (copy_length != 0) {
    memcpy(&main_encrypt_queue_[queue_offset], data + data_offset, copy_length);
    retrievable_from_queue_ += copy_length;
    current_position_ += copy_length;
    if (retrievable_from_queue_ == kDefaultByteArraySize_)
      ProcessMainQueue();
    data_offset += copy_length;
    queue_offset = 0;
    length -= copy_length;
    copy_length = std::min(length, kDefaultByteArraySize_);
//                                                                  handle extra last chunk
  }
}

bool SelfEncryptor::GetLengthForSequencer(const uint64_t &position,
                                          uint32_t *length) {
  if (*length == 0)
    return false;
  BOOST_ASSERT(position >= 2 * kDefaultChunkSize);
  if (position < queue_start_position_) {
    *length = static_cast<uint32_t>(std::min(static_cast<uint64_t>(*length),
                                             queue_start_position_ - position));
    return true;
  }
  return (position > queue_start_position_ + kDefaultByteArraySize_);
}

void SelfEncryptor::AddReleventSeqDataToQueue() {
  SequenceData extra(sequencer_.Get(current_position_));
  while (extra.second != 0) {
    PutToEncryptQueue(extra.first, extra.second, 0,
                      static_cast<uint32_t>(current_position_ -
                                            queue_start_position_));
    extra = sequencer_.Get(current_position_);
  }
}

void SelfEncryptor::ReadChunk(uint32_t chunk_num, byte *data) {
//  if ((data_map_->chunks.size() < chunk_num) ||
//      (data_map_->chunks.size() == 0)) {
  if (data_map_->chunks.size() < chunk_num) {
    read_ok_ = false;
    return;
  }

  // still in process of writing so read raw arrays
  if (chunk_one_two_q_full_ && (chunk_num < 2)) {
    if (chunk_num == 0) {
      for (uint32_t i = 0; i != c0_and_1_chunk_size_; ++i)
        data[i] = static_cast<byte>(chunk0_raw_[i]);
    } else {
      for (uint32_t i = 0; i != c0_and_1_chunk_size_; ++i)
        data[i] = static_cast<byte>(chunk1_raw_[i]);
    }
    return;
  }

  std::string hash(reinterpret_cast<char*>(data_map_->chunks[chunk_num].hash),
                   crypto::SHA512::DIGESTSIZE);
  uint32_t length = data_map_->chunks[chunk_num].size;
  ByteArray pad(new byte[(3 * crypto::SHA512::DIGESTSIZE) -
                         crypto::AES256_KeySize - crypto::AES256_IVSize]);
  ByteArray key(new byte[crypto::AES256_KeySize]);
  ByteArray iv(new byte[crypto::AES256_IVSize]);
  GetPadIvKey(chunk_num, key, iv, pad);
  std::string content;
#pragma omp critical
  {  // NOLINT (Fraser)
    content = chunk_store_->Get(hash);
  }

  if (content.empty()) {
    DLOG(ERROR) << "Could not find chunk number : " << chunk_num
                << " which is " << EncodeToHex(hash);
    read_ok_ = false;
    return;
  }

  CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryptor(
      key.get(), crypto::AES256_KeySize, iv.get());
//   CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryptor(key.get(),
//       crypto::AES256_KeySize, iv.get());
//           CryptoPP::StringSource filter(content, true,
//             new XORFilter(
//             new CryptoPP::StreamTransformationFilter(decryptor,
//               new CryptoPP::MessageQueue),
//             pad.get()));
  CryptoPP::StringSource filter(content, true,
      new XORFilter(
          new CryptoPP::StreamTransformationFilter(decryptor,
              new CryptoPP::Gunzip(new CryptoPP::MessageQueue())), pad.get()));
  filter.Get(data, length);
}

void SelfEncryptor::GetPadIvKey(uint32_t this_chunk_num,
                                ByteArray key,
                                ByteArray iv,
                                ByteArray pad) {
  uint32_t num_chunks = static_cast<uint32_t>(data_map_->chunks.size());
  uint32_t n_1_chunk = (this_chunk_num + num_chunks - 1) % num_chunks;
  uint32_t n_2_chunk = (this_chunk_num + num_chunks - 2) % num_chunks;

  for (uint32_t i = 0; i != crypto::AES256_KeySize; ++i)
    key[i] = data_map_->chunks[n_1_chunk].pre_hash[i];
  for (uint32_t i = 0; i != crypto::AES256_IVSize; ++i)
    iv[i] = data_map_->chunks[n_1_chunk].pre_hash[i + crypto::AES256_KeySize];

  for (uint32_t i = 0; i != crypto::SHA512::DIGESTSIZE; ++i) {
    pad[i] = data_map_->chunks[n_1_chunk].pre_hash[i];
    pad[i + crypto::SHA512::DIGESTSIZE] =
        data_map_->chunks[this_chunk_num].pre_hash[i];
  }

  uint32_t pad_offset(2 * crypto::SHA512::DIGESTSIZE);
  uint32_t hash_offset(crypto::AES256_KeySize + crypto::AES256_IVSize);
  for (uint32_t i = 0; i != crypto::AES256_IVSize; ++i) {
    pad[i + pad_offset] =
        data_map_->chunks[n_2_chunk].pre_hash[i + hash_offset];
  }
}

//bool SelfEncryptor::AttemptProcessQueue() {
//  // Do not queue chunks 0 and 1 till we know we have enough for 3 chunks
//  if ((retrievable_from_queue_ >= chunk_size_ * 3) && !chunk_one_two_q_full_)
//    QueueC0AndC1();
//
//  uint32_t bytes_to_process(ignore_threads_ ? chunk_size_ :
//                            kDefaultByteArraySize_);
//
//  if ((retrievable_from_queue_ >= bytes_to_process) && chunk_one_two_q_full_)
//    ProcessMainQueue();
//
//  return true;
//}
//
//bool SelfEncryptor::QueueC0AndC1() {
//  c0_and_1_chunk_size_ = chunk_size_;
//  // Chunk 0
//  main_encrypt_queue_.Get(chunk0_raw_.get(), chunk_size_);
//  ChunkDetails chunk_data;
//  CryptoPP::SHA512().CalculateDigest(chunk_data.pre_hash,
//                                     chunk0_raw_.get(),
//                                     chunk_size_);
//  chunk_data.size = chunk_size_;
//  data_map_->chunks.push_back(chunk_data);
//
//  // Chunk 1
//  main_encrypt_queue_.Get(chunk1_raw_.get(), chunk_size_);
//  ChunkDetails chunk_data2;
//  CryptoPP::SHA512().CalculateDigest(chunk_data2.pre_hash,
//                                     chunk1_raw_.get() ,
//                                     chunk_size_);
//  chunk_data2.size = chunk_size_;
//  data_map_->chunks.push_back(chunk_data2);
//  chunk_one_two_q_full_ = true;
//  return true;
//}

bool SelfEncryptor::ProcessMainQueue() {
  if (retrievable_from_queue_ < chunk_size_ || chunk_size_ == 0)
    return false;

  uint32_t chunks_to_process = retrievable_from_queue_ / chunk_size_;
  uint32_t old_dm_size = static_cast<uint32_t>(data_map_->chunks.size());
  data_map_->chunks.resize(chunks_to_process + old_dm_size);

#pragma omp parallel for
  for (uint32_t i = 0; i != chunks_to_process; ++i) {
    CryptoPP::SHA512().CalculateDigest(
        data_map_->chunks[i + old_dm_size].pre_hash,
        main_encrypt_queue_.get() + (i * chunk_size_),
        chunk_size_);
    data_map_->chunks[i + old_dm_size].size = chunk_size_;
  }
// check for repeated content
// TODO(dirvine) FIXME ( needs tested )

//   for(uint32_t i = 0; i < chunks_to_process; ++i) {
//     if ((data_map_->chunks[i + old_dm_size].pre_hash ==
//       data_map_->chunks[i + old_dm_size].pre_hash) &&
//       (data_map_->chunks[i + old_dm_size].pre_hash ==
//       data_map_->chunks[i -1 + old_dm_size].pre_hash) &&
//       (data_map_->chunks[i + old_dm_size].pre_hash ==
//       data_map_->chunks[i -2 + old_dm_size].pre_hash)) {
//       if (i == 2) { // only encrypt chunk 2
//         EncryptAChunk(i + old_dm_size, &chunk_vec[i][0], chunk_size_, false);
//       } else {
//         for (int j =0; j != crypto::SHA512::DIGESTSIZE; ++j)
//           data_map_->chunks[i + old_dm_size].hash[j] =
//           data_map_->chunks[i - 1 + old_dm_size].hash[j];
//       }
//     }
//   }
#pragma omp parallel for  // gives over 100Mb write speeds
  for (uint32_t i = 0; i != chunks_to_process; ++i) {
    EncryptAChunk(i + old_dm_size,
                  main_encrypt_queue_.get() + (i * chunk_size_), chunk_size_,
                  false);
  }
  queue_start_position_ += retrievable_from_queue_;
  retrievable_from_queue_ = 0;
  return true;
}

void SelfEncryptor::EncryptAChunk(uint32_t chunk_num,
                                  byte *data,
                                  uint32_t length,
                                  bool re_encrypt) {
  if (data_map_->chunks.size() < chunk_num)
    return;
  if (re_encrypt)  // fix pre enc hash and re-encrypt next 2
    CryptoPP::SHA512().CalculateDigest(data_map_->chunks[chunk_num].pre_hash,
                                       data,
                                       length);

  ByteArray pad(new byte[(3 * crypto::SHA512::DIGESTSIZE) -
                         crypto::AES256_KeySize - crypto::AES256_IVSize]);
  ByteArray key(new byte[crypto::AES256_KeySize]);
  ByteArray iv(new byte[crypto::AES256_IVSize]);
  GetPadIvKey(chunk_num, key, iv, pad);
  CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encryptor(
      key.get(), crypto::AES256_KeySize, iv.get());

  std::string chunk_content;
  CryptoPP::Gzip aes_filter(new CryptoPP::StreamTransformationFilter(encryptor,
                                new XORFilter(
                                    new CryptoPP::StringSink(chunk_content),
                                    pad.get())), 6);
  aes_filter.Put2(data, length, -1, true);

  CryptoPP::SHA512().CalculateDigest(data_map_->chunks[chunk_num].hash,
      const_cast<byte*>(reinterpret_cast<const byte*>(chunk_content.c_str())),
      chunk_content.size());

  std::string post_hash(reinterpret_cast<char*>(
      data_map_->chunks[chunk_num].hash), crypto::SHA512::DIGESTSIZE);
#pragma omp critical
  {  // NOLINT (Fraser)
    if (!chunk_store_->Store(post_hash, chunk_content))
      DLOG(ERROR) << "Could not store " << EncodeToHex(post_hash);
  }

  if (!re_encrypt) {
    data_map_->chunks[chunk_num].size = length;  // keep pre-compressed length
#pragma omp atomic
    data_map_->size += length;
  }
}

void SelfEncryptor::EmptySequencer() {
  if (sequencer_.empty())
    return;
// TODO(dirvine)
    // check if chunks exists that the sequencer should write to
    // i.e. get data map parameters and keep these chunks.num current size etc.
    // as we empty sequencer we grab chunks worth at time and encrypt that chunk
    // we need to check whether we have data for next chunks to encrypt next2
    // so read->chunk / alter / encrypt chunk / enc next 2 (unless ...)
    // divide num chunks with / chunks_size to get current floor
    // floor + chunk_size_ is this range !!
//  uint32_t last_seq_length;
//  uint64_t last_seq_pos = sequencer_.PeekLast(&last_seq_length);

//  uint64_t total_size(last_seq_pos + last_seq_length);
//  uint32_t last_chunk_num(static_cast<uint32_t>(total_size / chunk_size_));
     // after this set current_position_ and q - process last

  while (!sequencer_.empty()) {
    uint32_t chunks_written_to =
        static_cast<uint32_t>(data_map_->chunks.size() * chunk_size_);
    boost::scoped_array<char> data(new char);
    uint32_t length(0);
    uint64_t seq_pos = sequencer_.GetFirst(data.get(), &length);

    if (seq_pos < chunks_written_to) {
      uint32_t c_start((chunk_size_ - seq_pos) % chunk_size_);
      uint32_t c_end((chunk_size_ - (seq_pos + length)) % chunk_size_);
      uint32_t start_chunk(static_cast<uint32_t>(seq_pos / chunk_size_));
      uint32_t end_chunk(
          static_cast<uint32_t>((seq_pos + length) / chunk_size_) + 1);
      uint32_t chunk_array_size((end_chunk - start_chunk) * chunk_size_);
      ByteArray chunk_array(new byte[chunk_array_size]);
      // get chunks
      for (uint32_t i = start_chunk; i < end_chunk; ++i)
        ReadChunk(i, &chunk_array[start_chunk * chunk_size_]);
      // rewrite data in chunks
      for (uint32_t i = c_start; i < chunk_array_size - c_end; ++i )
        chunk_array[i] = data[i];
      // encrypt chunks
      for (uint32_t i = start_chunk; i < end_chunk; ++i)
//        DeleteAChunk(i);
          EncryptAChunk(i,
                        &chunk_array[start_chunk * chunk_size_],
                        chunk_size_, true);


        // TODO(DI) not always required, encrypt next 2
//       for (int i = end_chunk; i <= 2; ++i) {
//         chunk_num = (i + data_map_->chunks.size())
//         %  data_map_->chunks.size();
//         std::string hash(reinterpret_cast<char*>(
//           data_map_->chunks[chunk_num].hash), crypto::SHA512::DIGESTSIZE);
// //         DeleteAChunk(i);
//         EncryptAChunk(chunk_num,const_cast<byte *>
//         (reinterpret_cast<const byte *>
//         (chunk_store_->Get(hash).c_str())),
//                       data_map_->chunks[chunk_num].size,
//                       true);
//         current_position_ += data_map_->chunks[chunk_num].size;
//         // for q to process last
//       }
      continue;
    }
    // need to pad and write data
    if (current_position_ < seq_pos) {  // Nothing done - pad to this point
      boost::scoped_array<char> pad(new char[1]);
      pad[0] = 0;
      for (uint64_t i = current_position_; i < seq_pos; ++i)
        Write(&pad[0], 1, current_position_);
    }
    Write(data.get(), length, seq_pos);
  }
}

bool SelfEncryptor::WriteExtraAndEnc0and1() {
  if (retrievable_from_queue_ != 0) {
    ByteArray i(new byte[retrievable_from_queue_]);
//    main_encrypt_queue_.Get(i.get(), retrievable_from_queue_);
    std::string extra(reinterpret_cast<char*>(i.get()),
                      retrievable_from_queue_);
    data_map_->content = extra;
    data_map_->content_size = retrievable_from_queue_;
    data_map_->size += retrievable_from_queue_;
  }
  // when all that is done, encrypt chunks 0 and 1
  if (chunk_one_two_q_full_) {
#pragma omp sections
    {  // NOLINT (Fraser)
#pragma omp section
      {  // NOLINT (Fraser)
        EncryptAChunk(0, chunk0_raw_.get(), c0_and_1_chunk_size_, false);
      }
#pragma omp section
      {  // NOLINT (Fraser)
        EncryptAChunk(1, chunk1_raw_.get(), c0_and_1_chunk_size_, false);
      }
    }

    chunk0_raw_.reset();
    chunk1_raw_.reset();
    chunk_one_two_q_full_ = false;
  }
//  main_encrypt_queue_.SkipAll();
  data_map_->complete = true;
  return true;
}

bool SelfEncryptor::Read(char* data, uint32_t length, uint64_t position) {
  uint32_t maxbuffersize = kDefaultByteArraySize_;
  uint32_t cachesize =
      static_cast<uint32_t>(std::min(data_map_->size,
                                     static_cast<uint64_t>(maxbuffersize)));

  if (length < cachesize) {
    //  required -
    //  cache already populated and
    //  requested position not less than cache start and
    //  requested position not greater than cache end and
    //  enough info in cache to fulfil request
    if (cache_ &&
        (position > cache_initial_posn_) &&
        (cache_initial_posn_ + cachesize > position) &&
        ((cachesize - (position - cache_initial_posn_)) >= length)) {
      // read data_cache_
      for (uint32_t i = 0; i != length; ++i) {
        BOOST_ASSERT(position - cache_initial_posn_ + i <=
                     std::numeric_limits<uint32_t>::max());
        data[i] = data_cache_[static_cast<uint32_t>(position -
                              cache_initial_posn_) + i];
      }
    } else {
      // populate data_cache_ and read
      Transmogrify(data_cache_.get(), cachesize, position, false);
      cache_initial_posn_ = position;
      for (uint32_t i = 0; i != length; ++i)
        data[i] = data_cache_[i];
      cache_ = true;
    }
  } else {
    // length requested larger than cache size, just go ahead and read
    Transmogrify(data, length, position, false);
  }
  return true;
}

bool SelfEncryptor::Transmogrify(char *data,
                                 uint32_t length,
                                 uint64_t position,
                                 bool /*writing*/) {
  // TODO(JLB) :  ensure that on rewrite, if data is being written to area
  //              currently held in cache, then cache is refreshed after write.
  //              Transmogrify(data_cache_.get(), kDefaultByteArraySize_,
  //                           cache_initial_posn_, false)
  uint64_t run_total(0), all_run_total(0);
  uint32_t start_offset(0), end_cut(0), start_chunk(0), end_chunk(0);
  bool found_start(false);
  bool found_end(false);
  uint32_t num_chunks = static_cast<uint32_t>(data_map_->chunks.size());

  if (num_chunks != 0) {
    for (uint32_t i = 0; i != num_chunks; ++i) {
      if (found_start)
        run_total += data_map_->chunks[i].size;

      if (((all_run_total + data_map_->chunks[i].size) > position) &&
          !found_start) {
        start_chunk = i;
        start_offset = static_cast<uint32_t>(position - all_run_total);
        run_total = all_run_total + data_map_->chunks[i].size - position;
        found_start = true;
      }

      if (run_total >= length) {
        found_end = true;
        end_chunk = i;
        end_cut = length + static_cast<uint32_t>(position - all_run_total);
               // all_run_total - position - length
        break;
      }
      all_run_total += data_map_->chunks[i].size;
    }

    if (!found_end) {
      end_chunk = num_chunks - 1;
      end_cut = static_cast<uint32_t>(
          std::min(position + length -
                   (all_run_total - data_map_->chunks[end_chunk].size),
                   static_cast<uint64_t>(data_map_->chunks[end_chunk].size)));
    }
// this is 2 for loops to allow openmp to thread properly.
// should be refactored to a do loop and openmp fixed
//     if (chunk_one_two_q_full_) {
//      // don't try and get these chunks they're in a q
//      if ((start_chunk < 2) && (end_chunk < 2)) {
//       ReadInProcessData(data, length, position);
//       return true;
//      }
//     }

    if (start_chunk == end_chunk) {
      // get chunk
      ByteArray chunk_data(new byte[data_map_->chunks[start_chunk].size]);
      ReadChunk(start_chunk, chunk_data.get());
      for (uint32_t i = start_offset; i != length + start_offset; ++i)
        data[i - start_offset] = static_cast<char>(chunk_data[i]);
      return read_ok_;
    }

#pragma omp parallel for shared(data)
    for (uint32_t i = start_chunk; i <= end_chunk; ++i) {
      uint64_t pos(0);
      uint32_t this_chunk_size(data_map_->chunks[i].size);

      if (i == start_chunk) {
        if (start_offset != 0) {
          ByteArray chunk_data(new byte[data_map_->chunks[start_chunk].size]);
          ReadChunk(start_chunk, chunk_data.get());
          for (uint32_t j = start_offset; j != this_chunk_size; ++j)
            data[j - start_offset] = static_cast<char>(chunk_data[j]);
        } else {
          ReadChunk(i, reinterpret_cast<byte*>(&data[0]));
        }
      } else if (i == end_chunk) {
        ByteArray chunk_data(new byte[data_map_->chunks[end_chunk].size]);
        ReadChunk(end_chunk, chunk_data.get());

        for (uint32_t j = 0; j != i; ++j)
#pragma omp atomic
          pos += data_map_->chunks[j].size;

        for (uint32_t j = 0; j != end_cut; ++j)
          data[j + pos - position] = static_cast<char>(chunk_data[j]);

      } else {
        for (uint32_t j = 0; j != i; ++j)
#pragma omp atomic
          pos += data_map_->chunks[j].size;

        ReadChunk(i, reinterpret_cast<byte*>(&data[pos - position]));
      }
    }
  }

  uint64_t this_position(0);
#pragma omp barrier
  for (uint32_t i = 0; i != num_chunks; ++i)
    this_position += data_map_->chunks[i].size;

  for (uint32_t i = 0; i != data_map_->content_size; ++i) {
    if ((this_position + i) < (position + length)) {
      data[static_cast<uint32_t>(this_position - position) + i] =
          data_map_->content.c_str()[i];
    }
  }
  // replace any chunk data with most recently written stuff
  ReadInProcessData(data, length, position);
  return read_ok_;
}

void SelfEncryptor::ReadInProcessData(char *data,
                                      uint32_t length,
                                      uint64_t position) {
                                                          // TODO(Fraser#5#): 2011-09-15 - check Chunks 0 and 1

  // check queue
  if (retrievable_from_queue_ != 0)  {
    uint32_t data_offset(0), queue_offset(0), copy_length(0);
    if ((position < queue_start_position_ + retrievable_from_queue_) &&
        (position + length > queue_start_position_)) {
      if (position < queue_start_position_)
        data_offset = static_cast<uint32_t>(queue_start_position_ - position);
      else
        queue_offset = static_cast<uint32_t>(position - queue_start_position_);
      copy_length = std::min(length - data_offset,
                             retrievable_from_queue_ - queue_offset);
    }
    memcpy(data + data_offset, &main_encrypt_queue_[queue_offset], copy_length);
  }

  if (!sequencer_.empty()) {
    SequenceData answer = sequencer_.Peek(position);
    for (uint32_t i = 0; i != answer.second; ++i) {
      data[i] = answer.first[i];
    }
  }
}

bool SelfEncryptor::DeleteAllChunks() {
  for (uint32_t i = 0; i != static_cast<uint32_t>(data_map_->chunks.size());
       ++i) {
    if (!chunk_store_->Delete(reinterpret_cast<char*>(
                              data_map_->chunks[i].hash)))
      return false;
  }
  data_map_->chunks.clear();
  return true;
}

bool SelfEncryptor::Truncate(uint64_t size) {
  uint64_t byte_count(0);
  uint32_t number_of_chunks(static_cast<uint32_t>(data_map_->chunks.size()));
//  bool delete_remainder(false), found_end(false);
  // if (data_map_->complete) {
    // Assume size < data_map.size
    for (uint32_t i = 0; i != number_of_chunks; ++i) {
      uint32_t chunk_size = data_map_->chunks[i].size;
      byte_count += chunk_size;
      if (byte_count > size) {
        // Found chunk with data at position 'size'.
        if (retrievable_from_queue_ != 0)
//          main_encrypt_queue_.SkipAll();
        sequencer_.Clear();
        for (uint32_t j = i + 1; j != number_of_chunks; ++j) {
          if (!chunk_store_->Delete(reinterpret_cast<char*>
                                      (data_map_->chunks[j].hash))) {
            DLOG(ERROR) << "Failed to delete chunk";
            return false;
          }
          data_map_->chunks.pop_back();
        }
        if (byte_count - size == chunk_size) {
          if (!chunk_store_->Delete(reinterpret_cast<char*>
                                      (data_map_->chunks[i].hash))) {
            DLOG(ERROR) << "Failed to delete chunk";
            return false;
          }
          data_map_->chunks.pop_back();
        } else {
          std::shared_ptr<byte> data(new byte[chunk_size]);
          ReadChunk(i, data.get());
          BOOST_ASSERT(byte_count - size <= chunk_size);
//          uint32_t bytes_to_queue(chunk_size -
//                                  static_cast<uint32_t>(byte_count - size));
//          main_encrypt_queue_.Put2(data.get(), bytes_to_queue, 0, true);
          if (!chunk_store_->Delete(reinterpret_cast<char*>
                                      (data_map_->chunks[i].hash))) {
            DLOG(ERROR) << "Failed to delete chunk";
            return false;
          }
          data_map_->chunks.pop_back();
        }
        current_position_ = size;
        data_map_->size = (byte_count - chunk_size);
        data_map_->content.erase();
        data_map_->content_size = 0;
        data_map_->complete = false;
        return true;
      }
    }
    // Check data map content.

  // } else {
//    if (delete_remainder == true) {
//      sequencer_.EraseAll();
//      main_encrypt_queue_.SkipAll();
//    } else {
//      // check content
//    else
//      //check queue;
//    else
//      //check sequencer
//      if (size <= retrievable_from_queue_) {
//
//      }
//    }
  // }
  return true;
}

}  // namespace encrypt
}  // namespace maidsafe
