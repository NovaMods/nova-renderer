#ifndef RX_CORE_BIT_BUFFER_H
#define RX_CORE_BIT_BUFFER_H
#include "rx/core/types.h"

namespace rx {

struct stream;

// # Bit Buffer
//
// The idea of a bit buffer is to read and write values of arbitrary bit sizes
// into a single stream.
//
// To use just give the bit buffer a stream for reading or writing and begin
// reading or writing integers of arbitrary bit sizes by specifying how many
// bits.
struct bit_buffer {
  static constexpr rx_size k_buffer_size = 4096;

  enum class mode {
    k_rd,
    k_wr
  };

  bit_buffer(stream* _stream, mode _mode);
  ~bit_buffer();

  // Read |_bits| from stream into |data_|. Returns false if the end of the
  // stream is reached before all bits could be read into |data_|.
  bool read(rx_u64& data_, rx_size _bits);

  // Write |_bits| from |_data| into stream. If the buffer is full, this will
  // call flush to make room, if that flush fails, this returns false.
  bool write(rx_u64 _data, rx_size _bits);

  // Flush the currently buffered contents to the stream. Returns false if the
  // steam wouldn't accept all the buffered bytes.
  bool flush();

private:
  bool fetch();

  stream* m_stream;
  mode m_mode;

  struct {
    rx_byte data[k_buffer_size];

    // Number of readable bytes in |data|.
    rx_size size;

    // Read and write cursors in bits.
    rx_size wr;
    rx_size rd;
  } m_buffer;
};

} // namespace rx

#endif // RX_CORE_BIT_BUFFER_H
