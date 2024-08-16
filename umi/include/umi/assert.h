#ifndef UMI_ASSERT_H
#define UMI_ASSERT_H

namespace umi {

constexpr auto dassert(bool cond) {
#ifdef NDEBUG
  if (!cond)
#endif
}

}

#endif // UMI_ASSERT_H
