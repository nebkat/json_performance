#include "serpent_bench.hpp"

#include <serpent/json.hpp>

#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

// The same test object as every other library in this benchmark, described the way serpent
// describes a type: one annotation, and the compiler supplies the field names and their order.

namespace serpent_bench
{
   struct [[= serpent::serializable {}]] fixed_object_t
   {
      std::vector<int> int_array;
      std::vector<float> float_array;
      std::vector<double> double_array;

      bool operator==(const fixed_object_t&) const = default;
   };

   struct [[= serpent::serializable {}]] fixed_name_object_t
   {
      std::string name0{};
      std::string name1{};
      std::string name2{};
      std::string name3{};
      std::string name4{};

      bool operator==(const fixed_name_object_t&) const = default;
   };

   struct [[= serpent::serializable {}]] nested_object_t
   {
      std::vector<std::array<double, 3>> v3s{};
      std::string id{};

      bool operator==(const nested_object_t&) const = default;
   };

   struct [[= serpent::serializable {}]] another_object_t
   {
      std::string string{};
      std::string another_string{};
      std::string escaped_text{};
      bool boolean{};
      nested_object_t nested_object{};

      bool operator==(const another_object_t&) const = default;
   };

   struct [[= serpent::serializable {}]] obj_t
   {
      fixed_object_t fixed_object{};
      fixed_name_object_t fixed_name_object{};
      another_object_t another_object{};
      std::vector<std::string> string_array{};
      std::string string{};
      double number{};
      bool boolean{};
      bool another_bool{};

      bool operator==(const obj_t&) const = default;
   };

   // The ABC document's keys run "z" to "a"; this reads them into a type that declares them
   // "a" to "z", which is the point of the test.
   struct [[= serpent::serializable {}]] abc_t
   {
      std::vector<int64_t> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
   };

   namespace
   {
      using clock = std::chrono::steady_clock;

      double seconds(clock::time_point t0, clock::time_point t1)
      {
         return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() * 1e-6;
      }

      // Reads a whole document into an object that already exists, so its containers keep the
      // storage they had - the same discipline as the libraries that read into an lvalue. From
      // a std::string, which serpent reads trusting the zero byte after it as glaze reads one.
      template <class T>
      bool read(T& obj, const std::string& buffer)
      {
         return serpent::json::decode_into(buffer, obj);
      }

      // Writes into a buffer that is reused between iterations: cleared, so its capacity stays.
      template <class T>
      bool write(const T& obj, std::string& buffer)
      {
         return serpent::json::write(obj, buffer).has_value();
      }
   }

   timings run(std::string_view minified, std::size_t iterations)
   {
      timings result{};

      std::string buffer{minified};
      obj_t obj{};

      auto t0 = clock::now();

      for (std::size_t i = 0; i < iterations; ++i) {
         if (!read(obj, buffer)) {
            std::cout << "serpent error!\n";
            break;
         }
         if (!write(obj, buffer)) {
            std::cout << "serpent error!\n";
            break;
         }
      }

      auto t1 = clock::now();

      result.roundtrip = seconds(t0, t1);

      // write performance

      t0 = clock::now();

      for (std::size_t i = 0; i < iterations; ++i) {
         if (!write(obj, buffer)) {
            std::cout << "serpent error!\n";
            break;
         }
      }

      t1 = clock::now();

      result.byte_length = buffer.size();
      result.write = seconds(t0, t1);
      result.buffer = buffer;

      // serpent's own check: what it wrote reads back as the object it wrote
      obj_t decoded{};
      result.round_trips = read(decoded, buffer) && decoded == obj;

      // read performance

      t0 = clock::now();

      for (std::size_t i = 0; i < iterations; ++i) {
         if (!read(obj, buffer)) {
            std::cout << "serpent error!\n";
            break;
         }
      }

      t1 = clock::now();

      result.read = seconds(t0, t1);

      return result;
   }

   abc_timings run_abc(std::string_view text, std::size_t iterations)
   {
      abc_timings result{};

      const std::string buffer{text};
      abc_t obj{};

      const auto t0 = clock::now();

      for (std::size_t i = 0; i < iterations; ++i) {
         if (!read(obj, buffer)) {
            std::cout << "serpent error!\n";
            break;
         }
      }

      const auto t1 = clock::now();

      result.byte_length = buffer.size();
      result.read = seconds(t0, t1);

      std::vector<int64_t> expected(1000);
      std::iota(expected.begin(), expected.end(), 0);
      result.correct = obj.a == expected && obj.m == expected && obj.z == expected;

      return result;
   }
}
