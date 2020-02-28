#ifndef RX_CORE_PROFILER_H
#define RX_CORE_PROFILER_H
#include "rx/core/global.h"
#include "rx/core/optional.h"

namespace rx {

struct profiler {
  struct cpu_sample {
    cpu_sample(const char* _tag);
    ~cpu_sample();
  };

  struct gpu_sample {
    gpu_sample(const char* _tag);
    ~gpu_sample();
  };

  struct device {
    typedef void (*set_thread_name_fn)(void* _context, const char* _name);
    typedef void (*begin_sample_fn)(void* _context, const char* _tag);
    typedef void (*end_sample_fn)(void* _context);

    constexpr device(void* _context, set_thread_name_fn _set_thread_name_fn,
      begin_sample_fn _begin_sample_fn, end_sample_fn _end_sample_fn);

  private:
    friend struct profiler;

    void* m_context;
    set_thread_name_fn m_set_thread_name_fn;
    begin_sample_fn m_begin_sample_fn;
    end_sample_fn m_end_sample_fn;
  };

  using cpu = device;
  using gpu = device;

  void set_thread_name(const char* _name);

  void bind_gpu(const gpu& _gpu);
  void bind_cpu(const cpu& _cpu);

  void unbind_gpu();
  void unbind_cpu();

  static profiler& instance();

private:
  friend struct cpu_sample;
  friend struct gpu_sample;

  void begin_cpu_sample(const char* _tag);
  void end_cpu_sample();

  void begin_gpu_sample(const char* _tag);
  void end_gpu_sample();

  optional<gpu> m_gpu;
  optional<cpu> m_cpu;

  static RX_GLOBAL<profiler> s_profiler;
};

inline profiler::cpu_sample::cpu_sample(const char* _tag) {
  instance().begin_cpu_sample(_tag);
}

inline profiler::cpu_sample::~cpu_sample() {
  instance().end_cpu_sample();
}

inline profiler::gpu_sample::gpu_sample(const char* _tag) {
  instance().begin_gpu_sample(_tag);
}

inline profiler::gpu_sample::~gpu_sample() {
  instance().end_gpu_sample();
}

inline constexpr profiler::device::device(void* _context,
  set_thread_name_fn _set_thread_name_fn, begin_sample_fn _begin_sample_fn,
  end_sample_fn _end_sample_fn)
  : m_context{_context}
  , m_set_thread_name_fn{_set_thread_name_fn}
  , m_begin_sample_fn{_begin_sample_fn}
  , m_end_sample_fn{_end_sample_fn}
{
  RX_ASSERT(m_set_thread_name_fn, "thread name function missing");
  RX_ASSERT(m_begin_sample_fn, "begin sample function missing");
  RX_ASSERT(m_end_sample_fn, "end sample function missing");
}

inline void profiler::set_thread_name(const char* _name) {
  if (m_gpu) {
    m_gpu->m_set_thread_name_fn(m_gpu->m_context, _name);
  }

  if (m_cpu) {
    m_cpu->m_set_thread_name_fn(m_cpu->m_context, _name);
  }
}

inline void profiler::bind_gpu(const gpu& _gpu) {
  m_gpu = _gpu;
}

inline void profiler::bind_cpu(const cpu& _cpu) {
  m_cpu = _cpu;
}

inline void profiler::unbind_gpu() {
  m_gpu = nullopt;
}

inline void profiler::unbind_cpu() {
  m_cpu = nullopt;
}

inline profiler& profiler::instance() {
  return *s_profiler;
}

inline void profiler::begin_cpu_sample(const char* _tag) {
  if (m_cpu) {
    m_cpu->m_begin_sample_fn(m_cpu->m_context, _tag);
  }
}

inline void profiler::end_cpu_sample() {
  if (m_cpu) {
    m_cpu->m_end_sample_fn(m_cpu->m_context);
  }
}

inline void profiler::begin_gpu_sample(const char* _tag) {
  if (m_gpu) {
    m_gpu->m_begin_sample_fn(m_gpu->m_context, _tag);
  }
}

inline void profiler::end_gpu_sample() {
  if (m_gpu) {
    m_gpu->m_end_sample_fn(m_gpu->m_context);
  }
}

} // namespace rx

#endif // RX_PROFILER_H
