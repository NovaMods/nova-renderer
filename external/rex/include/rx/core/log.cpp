#include <time.h> // time_t, tm, strftime
#include <string.h> // strlen

#include "rx/core/log.h" // log
#include "rx/core/string.h" // string
#include "rx/core/vector.h" // vector
#include "rx/core/profiler.h" // g_profiler
#include "rx/core/algorithm/max.h"

#include "rx/core/filesystem/file.h" // file

#include "rx/core/concurrency/mutex.h" // mutex
#include "rx/core/concurrency/condition_variable.h" // condition_variable
#include "rx/core/concurrency/scope_lock.h" // scope_lock
#include "rx/core/concurrency/scope_unlock.h" // scope_unlock
#include "rx/core/concurrency/thread.h" // thread

#include "rx/core/global.h"

namespace rx {

RX_GLOBAL_GROUP("loggers", g_group_loggers);

static constexpr const rx_size k_flush_threshold{1000}; // 1000 messages

static inline const char*
get_level_string(log::level lvl) {
  switch (lvl) {
  case log::level::k_warning:
    return "warning";
  case log::level::k_info:
    return "info";
  case log::level::k_verbose:
    return "verbose";
  case log::level::k_error:
    return "error";
  }
  return nullptr;
}

static inline string
time_stamp(time_t time, const char* fmt) {
  struct tm tm;
#if defined(RX_PLATFORM_WINDOWS)
  localtime_s(&tm, &time);
#else
  localtime_r(&time, &tm);
#endif
  char date[256];
  strftime(date, sizeof date, fmt, &tm);
  date[sizeof date - 1] = '\0';
  return date;
}

struct message {
  message() = default;
  message(const log* _owner, string&& contents_, log::level _level, time_t _time)
    : m_owner{_owner}
    , m_contents{utility::move(contents_)}
    , m_level{_level}
    , m_time{_time}
  {
  }
  const log* m_owner;
  string m_contents;
  log::level m_level;
  time_t m_time;
};

struct logger {
  logger();
  ~logger();

  void write(const log* _owner, string&& contents_, log::level _level, time_t _time);
  bool flush(rx_size _max_padding);

  void process(int _thread_id); // running in |m_thread|

  enum {
    k_running = 1 << 0,
    k_ready = 1 << 1
  };

  filesystem::file m_file;
  rx_size m_max_name_length;
  rx_size m_max_level_length;
  int m_status;
  concurrency::mutex m_mutex;
  vector<message> m_queue; // protected by |m_mutex|
  concurrency::condition_variable m_ready_condition;
  concurrency::condition_variable m_flush_condition;
  concurrency::thread m_thread;

  static RX_GLOBAL<logger> s_logger;
};

RX_GLOBAL<logger> logger::s_logger{"system", "logger"};

logger::logger()
  : m_file{"log.log", "w"}
  , m_max_name_length{0}
  , m_max_level_length{0}
  , m_status{k_running}
  , m_thread{"logger", [this](int id){ process(id); }}
{
  // Calculate the maximum name length for all the loggers.
  g_group_loggers.each([&](global_node* _node) {
    m_max_name_length = algorithm::max(m_max_name_length, strlen(_node->name()));
  });

  // Initialize the loggers.
  g_group_loggers.init();

  // calculate maximum level string length
  const auto level0{strlen(get_level_string(log::level::k_warning))};
  const auto level1{strlen(get_level_string(log::level::k_info))};
  const auto level2{strlen(get_level_string(log::level::k_verbose))};
  const auto level3{strlen(get_level_string(log::level::k_error))};

  m_max_level_length = algorithm::max(level0, level1, level2, level3);

  // signal the logging thread to begin
  {
    concurrency::scope_lock locked{m_mutex};
    m_status |= k_ready;
    m_ready_condition.signal();
  }
}

logger::~logger() {
  // signal shutdown
  {
    concurrency::scope_lock locked(m_mutex);
    m_status &= ~k_running;
    m_flush_condition.signal();
  }

  // join thread
  m_thread.join();

  // deinitialize all loggers
  g_group_loggers.fini();
}

void logger::write(const log* _owner, string&& contents_, log::level _level, time_t _time) {
  concurrency::scope_lock locked{m_mutex};
  m_queue.emplace_back(_owner, utility::move(contents_), _level, _time);

#if defined(RX_DEBUG)
  (void)k_flush_threshold;
  m_flush_condition.signal();
#else
  if (m_queue.size() >= k_flush_threshold) {
    m_flush_condition.signal();
  }
#endif
}

bool logger::flush(rx_size max_padding) {
  m_queue.each_fwd([&](const message& _message) {
    const auto name_string{_message.m_owner->name()};
    const auto level_string{get_level_string(_message.m_level)};
    const auto padding{strlen(name_string) + strlen(level_string) + 1}; // +1 for '/'
    m_file.print("[%s] [%s/%s]%*s | %s\n",
      time_stamp(_message.m_time, "%Y-%m-%d %H:%M:%S"),
      name_string,
      level_string,
      static_cast<int>(max_padding - padding),
      "",
      _message.m_contents);
    return true;
  });

  m_queue.clear();

  return m_file.flush();
}

void logger::process(int) {
  concurrency::scope_lock locked{m_mutex};

  // wait until ready
  m_ready_condition.wait(locked, [this]{ return m_status & k_ready; });

  const auto max_padding{m_max_name_length + m_max_level_length + 1};
  flush(max_padding);

  while (m_status & k_running) {
    flush(max_padding);

    // wait for the next flush operation
    m_flush_condition.wait(locked);
  }

  // flush any contents at thread exit
  flush(max_padding);

  RX_ASSERT(m_queue.is_empty(), "not all contents flushed");
}

void log::write(log::level level, string&& contents_) {
  {
    concurrency::scope_lock lock{m_mutex};
    m_events.signal(level, contents_);
  }

  logger::s_logger->write(this, utility::move(contents_), level, time(nullptr));
}

} // namespace rx
