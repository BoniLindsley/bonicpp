// Corresponding header.
#include <bonicpp/mdvector.hpp>

// External dependencies.
#include <catch2/catch_test_macros.hpp>

// Standard libraries.
#include <array>
#include <vector>

TEST_CASE("Slice") {
  SECTION("constructor") {
    SECTION("rank 1") {
      auto shape = std::array<std::size_t, 1>{3};
      auto buffer = std::vector(3, 1);
      auto slice = bonicpp::Slice<int, 1>(buffer.data(), shape.data());
      CHECK(slice.data() == buffer.data());
      CHECK(slice.rank() == 1);
      CHECK(slice.size() == 3);
      CHECK(slice.stride() == 1);
    }
    SECTION("rank 3") {
      auto shape = std::array<std::size_t, 3>{2, 3, 5};
      auto buffer = std::vector(2l * 3 * 5, 1);
      auto slice = bonicpp::Slice<int, 3>(buffer.data(), shape.data());
      CHECK(slice.data() == buffer.data());
      CHECK(slice.rank() == 3);
      CHECK(slice.size() == 30);
      CHECK(slice.stride() == 15);
    }
  }
  SECTION("operator index") {
    SECTION("read rank 3") {
      auto shape = std::array<std::size_t, 3>{2, 1, 3};
      auto buffer = std::vector(6, 7);
      auto slice = bonicpp::Slice<int, 3>(buffer.data(), shape.data());
      CHECK(slice[0][0][0] == 7);
      CHECK(slice[0][0][1] == 7);
      CHECK(slice[0][0][2] == 7);
      CHECK(slice[1][0][0] == 7);
      CHECK(slice[1][0][1] == 7);
      CHECK(slice[1][0][2] == 7);
    }
    SECTION("read write for rank 1") {
      auto shape = std::array<std::size_t, 1>{3};
      auto buffer = std::vector(3, 7);
      auto slice = bonicpp::Slice<int, 1>(buffer.data(), shape.data());
      CHECK(slice[0] == 7);
      CHECK(slice[1] == 7);
      CHECK(slice[2] == 7);
      slice[1] = 9;
      CHECK(slice[1] == 9);
    }
    SECTION("read write for rank 2") {
      auto shape = std::array<std::size_t, 2>{{2, 3}};
      auto buffer = std::vector(6, 7);
      auto slice = bonicpp::Slice<int, 2>(buffer.data(), shape.data());
      CHECK(slice[0][0] == 7);
      CHECK(slice[0][2] == 7);
      CHECK(slice[0][2] == 7);
      CHECK(slice[1][0] == 7);
      CHECK(slice[1][2] == 7);
      CHECK(slice[1][2] == 7);
      slice[1][2] = 10;
      CHECK(slice[1][2] == 10);
    }
  }
  SECTION("begin end iterate") {
    SECTION("rank 1") {
      auto shape = std::array<std::size_t, 1>{{3}};
      auto buffer = std::vector(3, 7);
      auto slice = bonicpp::Slice<int, 1>(buffer.data(), shape.data());
      SECTION("by value read") {
        for (auto value : slice) {
          CHECK(value == 7);
        }
      }
      SECTION("by value does not modify") {
        for (auto value : slice) {
          value = 8;
        }
        for (auto value : slice) {
          CHECK(value == 7);
        }
      }
      SECTION("by lvalue modifies") {
        for (auto& value : slice) {
          value = 8;
        }
        for (auto value : slice) {
          CHECK(value == 8);
        }
      }
    }
    SECTION("rank 2") {
      auto shape = std::array<std::size_t, 2>{{3, 2}};
      auto buffer = std::vector(6, 7);
      auto slice = bonicpp::Slice<int, 2>(buffer.data(), shape.data());
      SECTION("by value read") {
        for (auto row : slice) {
          for (auto value : row) {
            CHECK(value == 7);
          }
        }
      }
      SECTION("by value does not modify") {
        for (auto row : slice) {
          for (auto value : row) {
            // By value. Does not modify.
            value = 9;
          }
        }
        for (auto row : slice) {
          for (auto value : row) {
            CHECK(value == 7);
          }
        }
      }
      SECTION("by lvalue modifies") {
        for (auto row : slice) {
          for (auto& value : row) {
            // By lvalue. Modifies.
            value = 2;
          }
        }
        for (auto row : slice) {
          for (auto& value : row) {
            CHECK(value == 2);
          }
        }
      }
    }
    SECTION("rank 3") {
      auto shape = std::array<std::size_t, 3>{{2, 2, 1}};
      auto buffer = std::vector(4, 5);
      auto slice = bonicpp::Slice<int, 3>(buffer.data(), shape.data());
      for (auto block : slice) {
        for (auto row : block) {
          for (auto value : row) {
            CHECK(value == 5);
          }
        }
      }
    }
  }
}

TEST_CASE("MDVector") {
  SECTION("constructor") {
    SECTION("default") {
      auto md_vector = bonicpp::MDVector<int, 2>{};
      CHECK(md_vector.extent(0) == 0);
      CHECK(md_vector.extent(1) == 0);
      CHECK(md_vector.rank() == 2);
      CHECK(md_vector.size() == 0);
      CHECK(md_vector.stride() == 0);
    }
    SECTION("rank 1") {
      auto md_vector = bonicpp::MDVector<int, 1>{{2}};
      CHECK(md_vector.data() != nullptr);
      CHECK(md_vector.extent(0) == 2);
      CHECK(md_vector.rank() == 1);
      CHECK(md_vector.size() == 2);
      CHECK(md_vector.stride() == 1);
    }
    SECTION("rank 3") {
      auto md_vector = bonicpp::MDVector<int, 3>{{2, 3, 5}};
      CHECK(md_vector.data() != nullptr);
      CHECK(md_vector.extent(0) == 2);
      CHECK(md_vector.extent(1) == 3);
      CHECK(md_vector.extent(2) == 5);
      CHECK(md_vector.rank() == 3);
      CHECK(md_vector.size() == 30);
      CHECK(md_vector.stride() == 15);
    }
  }
  SECTION("fill") {
    auto md_vector = bonicpp::MDVector<int, 3>{{1, 2, 3}};
    md_vector.fill(6);
    CHECK(md_vector.data()[0] == 6);
    CHECK(md_vector.data()[1] == 6);
    CHECK(md_vector.data()[2] == 6);
    CHECK(md_vector.data()[3] == 6);
    CHECK(md_vector.data()[4] == 6);
    CHECK(md_vector.data()[5] == 6);
  }
  SECTION("operator index") {
    SECTION("read rank 3") {
      auto md_vector = bonicpp::MDVector<int, 3>{{2, 1, 3}}.fill(7);
      CHECK(md_vector[0][0][0] == 7);
      CHECK(md_vector[0][0][1] == 7);
      CHECK(md_vector[0][0][2] == 7);
      CHECK(md_vector[1][0][0] == 7);
      CHECK(md_vector[1][0][1] == 7);
      CHECK(md_vector[1][0][2] == 7);
    }
    SECTION("read write for rank 1") {
      auto md_vector = bonicpp::MDVector<int, 1>{{3}}.fill(7);
      CHECK(md_vector[0] == 7);
      CHECK(md_vector[1] == 7);
      CHECK(md_vector[2] == 7);
      md_vector[1] = 9;
      CHECK(md_vector[1] == 9);
    }
    SECTION("read write for rank 2") {
      auto md_vector = bonicpp::MDVector<int, 2>{{2, 3}}.fill(7);
      CHECK(md_vector[0][0] == 7);
      CHECK(md_vector[0][2] == 7);
      CHECK(md_vector[0][2] == 7);
      CHECK(md_vector[1][0] == 7);
      CHECK(md_vector[1][2] == 7);
      CHECK(md_vector[1][2] == 7);
      md_vector[1][2] = 10;
      CHECK(md_vector[1][2] == 10);
    }
  }
  SECTION("begin end iterate") {
    SECTION("rank 1") {
      auto md_vector = bonicpp::MDVector<int, 1>{{3}}.fill(7);
      SECTION("by value read") {
        for (auto value : md_vector) {
          CHECK(value == 7);
        }
      }
      SECTION("by value does not modify") {
        for (auto value : md_vector) {
          value = 8;
        }
        for (auto value : md_vector) {
          CHECK(value == 7);
        }
      }
      SECTION("by lvalue modifies") {
        for (auto& value : md_vector) {
          value = 8;
        }
        for (auto value : md_vector) {
          CHECK(value == 8);
        }
      }
    }
    SECTION("rank 2") {
      auto md_vector = bonicpp::MDVector<int, 2>{{3, 2}}.fill(7);
      SECTION("by value read") {
        for (auto row : md_vector) {
          for (auto value : row) {
            CHECK(value == 7);
          }
        }
      }
      SECTION("by value does not modify") {
        for (auto row : md_vector) {
          for (auto value : row) {
            // By value. Does not modify.
            value = 9;
          }
        }
        for (auto row : md_vector) {
          for (auto value : row) {
            CHECK(value == 7);
          }
        }
      }
      SECTION("by lvalue modifies") {
        for (auto row : md_vector) {
          for (auto& value : row) {
            // By lvalue. Modifies.
            value = 2;
          }
        }
        for (auto row : md_vector) {
          for (auto& value : row) {
            CHECK(value == 2);
          }
        }
      }
    }
    SECTION("rank 3") {
      auto md_vector = bonicpp::MDVector<int, 3>{{2, 2, 1}}.fill(5);
      for (auto slice : md_vector) {
        for (auto row : slice) {
          for (auto value : row) {
            CHECK(value == 5);
          }
        }
      }
    }
  }
}
