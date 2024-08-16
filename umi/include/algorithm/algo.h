#ifndef UMI_ALGO_H
#define UMI_ALGO_H

#include <string>
#include <unordered_set>
#include <vector>

namespace nonstd {

template<typename T>
constexpr auto intersection(
  std::unordered_set<T> s1,
  const std::unordered_set<T> &s2
) -> std::unordered_set<T> {
  for (const auto &e: s2) s1.insert(e);
  return s1;
}

template<
  template<typename, typename ...> typename OutContainer = std::vector,
  typename InSequence,
  typename Fn
>
constexpr auto map(InSequence &&seq, Fn &&fn) {
  using T = std::remove_cvref_t<decltype(*seq.begin())>;
  auto result = OutContainer<std::invoke_result_t<Fn, T>>{};
  for (auto &&e: seq) {
    if constexpr (requires(OutContainer<T> c, T t) { c.push_back(t); }) {
      result.push_back(fn(e));
    } else {
      result.insert(fn(e));
    }
  }

  return result;
}

template<typename InSequence, typename Fn, typename OutSequence = std::remove_reference_t<InSequence>>
constexpr auto filter(InSequence &&seq, Fn &&fn) -> OutSequence {
  auto result = OutSequence{};
  for (auto &&e: seq) {
    if (fn(e)) {
      result.push_back(e);
    }
  }
  return result;
}

/*  template<typename K, typename V, typename H>
  constexpr auto value_or(
      const std::unordered_map<K, V, H> &mp, const K &key,
      const V &or_value = V{}
    ) -> V {
    auto found = mp.find(key);
    return found == mp.end() ? or_value : found->second;
  }*/

template<std::integral T>
constexpr auto iota(T start, T end) -> std::vector<T> {
  auto res = std::vector<T>{};
  res.reserve(end - start);

  for (std::integral auto i = start; i < end; ++i) {
    res.push_back(i);
  }

  return res;
}


/*  template<typename K, typename V>
  constexpr auto merge(const std::unordered_map<K, V> &mp1,
                       const std::unordered_map<K, V> &mp2) -> std::unordered_map<K, V> {
    auto mp = std::unordered_map<K, V>{};

    for (const auto &item: mp1) mp.insert(item);
    for (const auto &item: mp2) mp.insert(item);

    return mp;
  }*/

template<typename InSequence, typename OutSequence = std::vector<std::string>>
constexpr auto quoted(
  const InSequence &seq,
  const std::string &quote = "\""
) -> OutSequence {
  auto quoted_strings = OutSequence{};

  for (const auto &s: seq) {
    quoted_strings.push_back(quote + s + quote);
  }

  return quoted_strings;
}

constexpr auto quoted(
  const std::string &word,
  const std::string &quote = "\""
) -> std::string {
  return quote + word + quote;
}

template<typename T>
constexpr auto
extend(std::vector<T> &v, const std::vector<T> &new_values) -> void {
  for (const auto &e: new_values) {
    v.push_back(e);
  }
}

consteval auto newlined(const std::string &s) -> std::string {
  return s + "\n";
}

/*  constexpr auto mismatch(
      const std::string &s1,
      const std::string &s2
  ) -> std::int64_t {
    const auto wb = s1.begin(), nwb = s2.begin();
    auto [it1, it2] = std::mismatch(wb, s1.end(), nwb);
    return std::min(std::distance(wb, it1), std::distance(nwb, it2));
  }*/

template<typename T, typename Fn>
auto enumerate(const std::vector<T> &v, Fn &&f) -> void
requires std::invocable<Fn, decltype(v.size()), T> {
  const auto len = v.size();
  for (auto i = 0; i < len; ++i) {
    f(i, v[i]);
  }
}

}

#endif // UMI_ALGO_H
