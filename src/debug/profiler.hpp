// #pragma once

// #include "log.hpp"

// #include "utils/time.hpp"

// namespace xen {
// class ProfileSession {
//     std::ofstream output;
//     size_t entriesCount = 0;

// public:
//     /*!
//     checks if json file is opened
//     \returns true if json file can be written to, false either
//     */
//     bool is_valid() const;
//     /*!
//     getter for entriesCount
//     \returns number of json entries
//     */
//     size_t GetEntryCount() const;
//     /*!
//     creates json file or clears it if it exists and writes json header to it
//     \param filename file to output json to
//     */
//     void StartSession(MxString const& filename);
//     /*!
//     writes json entry, consisting of process id, start/end time, function name
//     \param function called function name
//     \param begin start timepoint of function execution
//     \param delta duration of function execution
//     */
//     void WriteJsonEntry(char const* function, TimeStep begin, TimeStep delta);
//     /*!
//     ends profile measurement, writing json footer and saving json file to disk
//     */
//     void EndSession();

// private:
//     void WriteJsonHeader();

//     void WriteJsonFooter();
// };

// class Profiler {
//     inline static ProfileSession impl;

// public:
//     static void Start(MxString const& filename) { impl.StartSession(filename); }
//     static void WriteEntry(char const* function, TimeStep begin, TimeStep delta)
//     {
//         impl.WriteJsonEntry(function, begin, delta);
//     }
//     static void Finish() { impl.EndSession(); }
// };

// class ScopeProfiler {
//     Time start;
//     std::string_view function;

// public:
//     ScopeProfiler(char const* function) : start(Time::now()), function(function) {}

//     ~ScopeProfiler()
//     {
//         auto end = Time::now();
//         Profiler::WriteEntry(this->function, this->start, end - start);
//     }
// };

// class ScopeTimer {
//     Time start;
//     std::string_view function;
//     std::string_view invoker;

// public:
//     ScopeTimer(std::string_view invoker, std::string_view function) :
//         start(Time::now()), function(function), invoker(invoker)
//     {
//         Log::info(invoker, " calling ", function);
//     }

//     ~ScopeTimer();
// };

// #define XEN_CONCAT_IMPL(x, y) x##y
// #define XEN_CONCAT(x, y) XEN_CONCAT_IMPL(x, y)

// #if defined(XEN_PROFILING_ENABLED)
// #define MAKE_SCOPE_PROFILER(function) ScopeProfiler XEN_CONCAT(_profiler, __LINE__)(function)
// #define MAKE_SCOPE_TIMER(invoker, function) ScopeTimer XEN_CONCAT(_timer, __LINE__)(invoker, function)
// #else
// #define MAKE_SCOPE_PROFILER(function)
// #define MAKE_SCOPE_TIMER(invoker, function)
// #endif
// }