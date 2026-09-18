#pragma once

// serpent's reflected path needs GCC 16 with -std=c++26 -freflection, which the rest of this
// benchmark is not compiled with, so it lives in its own translation unit and hands the timings
// back through this header. The work it does is the work every other library here does: the same
// object, the same iteration counts, the same reused buffer.

#include <cstddef>
#include <string>
#include <string_view>

namespace serpent_bench
{
   struct timings
   {
      double roundtrip{};
      double write{};
      double read{};
      std::size_t byte_length{};
      std::string buffer{}; // what the write loop last produced, for the shared validity check
      bool round_trips{};   // serpent's own written JSON decoded back to an equal object
   };

   timings run(std::string_view minified, std::size_t iterations);

   struct abc_timings
   {
      double read{};
      std::size_t byte_length{};
      bool correct{};
   };

   abc_timings run_abc(std::string_view buffer, std::size_t iterations);
}
