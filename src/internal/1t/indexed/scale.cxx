#include "scale.hpp"
#include "internal/1t/dense/scale.hpp"

#include "util/tensor.hpp"

namespace tblis
{
namespace internal
{

template <typename T>
void scale(const communicator& comm, const config& cfg,
           T alpha, bool conj_A, const indexed_varray_view<T>& A,
           const dim_vector& idx_A_A)
{
    A.for_each_index(
    [&](const varray_view<T>& local_A)
    {
        scale(comm, cfg, local_A.lengths(), alpha, conj_A, local_A.data(),
              local_A.strides());
    });
}

#define FOREACH_TYPE(T) \
template void scale(const communicator& comm, const config& cfg, \
                    T alpha, bool conj_A, const indexed_varray_view<T>& A, \
                    const dim_vector&);
#include "configs/foreach_type.h"

}
}