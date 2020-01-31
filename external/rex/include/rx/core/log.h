#ifndef RX_CORE_LOG_H
#define RX_CORE_LOG_H
#include "rx/core/global.h"
#include "rx/core/string.h"
#include "rx/core/event.h"

#include "rx/core/concurrency/mutex.h"
#include "rx/core/concurrency/scope_lock.h"

namespace rx {

struct log {
  enum class level {
    k_warning,
    k_info,
    k_verbose,
    k_error
  };

  using event_type = event<void(level, string)>;

  log(const char* _name, const char* _file_name, int _line);

  template<typename... Ts>
  void operator()(level _level, const char* _fmt, Ts&&... _args);

  const char* name() const;
  const char* file_name() const;
  int line() const;

  event_type::handle on_write(event_type::delegate&& callback_);

private:
  void write(level _level, string&& contents_);

  const char* m_name;
  const char* m_file_name;
  int m_line;

  concurrency::mutex m_mutex;
  event_type m_events;
};

inline log::log(const char* _name, const char* _file_name, int _line)
  : m_name{_name}
  , m_file_name{_file_name}
  , m_line{_line}
{
}

template<typename... Ts>
inline void log::operator()(log::level _level, const char* _format, Ts&&... _arguments) {
  write(_level, string::format(_format, utility::forward<Ts>(_arguments)...));
}

inline const char* log::name() const {
  return m_name;
}

inline const char* log::file_name() const {
  return m_file_name;
}

inline int log::line() const {
  return m_line;
}

inline log::event_type::handle log::on_write(function<void(level, string)>&& callback_) {
  concurrency::scope_lock lock{m_mutex};
  return m_events.connect(utility::move(callback_));
}

#define RX_LOG(_name, _identifier) \
  static RX_GLOBAL<::rx::log> _identifier{"loggers", (_name), (_name), __FILE__, __LINE__}

} // namespace rx

#endif // RX_CORE_LOG_H
