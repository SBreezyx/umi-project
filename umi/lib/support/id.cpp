#include "umi/id.h"


namespace umi::id {

static auto _id = id_type{};

auto next() -> id_type {
  return _id++;
}

auto peek() -> id_type {
  return _id;
}

auto reset(id_type seed) {
  _id = seed;
}

}