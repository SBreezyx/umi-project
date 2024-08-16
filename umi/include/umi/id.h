#ifndef UMI_ID_H
#define UMI_ID_H

namespace umi {

using id_type = int;

namespace id {

auto next() -> id_type;

auto peek() -> id_type;

auto reset(id_type seed = id_type{});

}
}

#endif // UMI_ID_H
