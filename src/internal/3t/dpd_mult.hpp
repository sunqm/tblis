#ifndef _TBLIS_INTERNAL_3T_DPD_MULT_HPP_
#define _TBLIS_INTERNAL_3T_DPD_MULT_HPP_

#include "mult.hpp"

namespace tblis
{
namespace internal
{

template <typename T>
void dpd_mult(const communicator& comm, const config& cfg,
              T alpha, const dpd_varray_view<const T>& A,
              const std::vector<unsigned>& idx_A_only,
              const std::vector<unsigned>& idx_A_AB,
              const std::vector<unsigned>& idx_A_AC,
              const std::vector<unsigned>& idx_A_ABC,
                       const dpd_varray_view<const T>& B,
              const std::vector<unsigned>& idx_B_only,
              const std::vector<unsigned>& idx_B_AB,
              const std::vector<unsigned>& idx_B_BC,
              const std::vector<unsigned>& idx_B_ABC,
              T  beta, const dpd_varray_view<      T>& C,
              const std::vector<unsigned>& idx_C_only,
              const std::vector<unsigned>& idx_C_AC,
              const std::vector<unsigned>& idx_C_BC,
              const std::vector<unsigned>& idx_C_ABC);

}
}

#endif