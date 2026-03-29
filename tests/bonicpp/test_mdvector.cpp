// Corresponding header.
#include <bonicpp/mdvector.hpp>

// External dependencies.
#include <catch2/catch_test_macros.hpp>

// Standard libraries.
#include <array>
#include <numeric>
#include <vector>

TEST_CASE("SliceIterator") {
  SECTION("constructor rank 1") {
    auto shape = std::array<std::size_t, 1>{3};
    auto buffer = std::vector(3, 0);
    buffer = {1, 5, 9};
    // Constructor.
    auto slice =
        bonicpp::SliceIterator<int, 1>(buffer.data(), shape.data(), 1ul);
    // Copy constructor.
    auto other = slice;
    // Equality.
    CHECK(slice == other);
    CHECK(not(slice != other));
    // Dereference.
    CHECK(*slice == 1ul);
    // Index.
    CHECK(slice[0] == 1ul);
    CHECK(slice[1] == 5ul);
    CHECK(slice[2] == 9ul);
    // Increment.
    ++slice;
    CHECK(*slice == 5ul);
    CHECK(slice[0] == 5ul);
    CHECK(slice[1] == 9ul);
    // Inequality.
    CHECK(slice != other);
    CHECK(not(slice == other));
  }
  SECTION("constructor rank 3") {
    auto shape = std::array<std::size_t, 3>{3, 2, 3};
    auto buffer = std::vector(18, 0);
    std::iota(buffer.begin(), buffer.end(), 1);
    // Constructor.
    auto slice =
        bonicpp::SliceIterator<int, 3>(buffer.data(), shape.data(), 6ul);
    // Copy constructor.
    auto other = slice;
    // Equality.
    CHECK(slice == other);
    CHECK(not(slice != other));
    // Dereference.
    CHECK((*slice)[0][0] == 1ul);
    // Index.
    CHECK(slice[0][0][1] == 2ul);
    CHECK(slice[0][1][0] == 4ul);
    // Increment.
    ++slice;
    CHECK(slice[0][0][1] == 8ul);
    CHECK(slice[0][1][0] == 10ul);
    // Inequality.
    CHECK(slice != other);
    CHECK(not(slice == other));
  }
}

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

TEST_CASE("MDSpan") {
  SECTION("constructor from vector") {
    auto source = std::vector(15, 0);
    std::iota(source.begin(), source.end(), 1);
    auto md_span = bonicpp::MDSpan<int, 1>{source};
    auto slice = md_span.to_slice();
    CHECK(slice[0] == 1);
    CHECK(slice[14] == 15);
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
