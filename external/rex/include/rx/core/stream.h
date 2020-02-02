#ifndef RX_CORE_STREAM_H
#define RX_CORE_STREAM_H

#include "rx/core/optional.h"
#include "rx/core/concepts/interface.h"

namespace rx {

struct stream
  : concepts::interface
{
  enum class whence {
    k_set,     // Beginning of stream.
    k_current, // Current position
    k_end      // End of stream.
  };

  // Read |_size| bytes from stream into |_data|.
  virtual rx_u64 read(rx_byte* _data, rx_u64 _size) = 0;

  // Write |_size| bytes from |_data| into stream.
  virtual rx_u64 write(const rx_byte* _data, rx_u64 _size) = 0;

  // Seek to |_where| in stream relative to |_whence|.
  virtual bool seek(rx_s64 _where, whence _whence) = 0;

  // Flush any buffered contents in the stream out.
  virtual bool flush() = 0;

  // Query the size of the stream. This must return the size regardless what
  // state the stream is. Streams which do not support querying the size should
  // return nullopt.
  virtual optional<rx_u64> size() = 0;
};

} // namespace rx

#endif // RX_CORE_STREAM_H
