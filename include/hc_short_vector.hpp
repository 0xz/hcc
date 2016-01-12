#pragma once

#ifndef _HC_SHORT_VECTORS_HPP
#define _HC_SHORT_VECTORS_HPP

namespace hc
{

namespace short_vector
{

#ifdef __HCC__
#define __CPU_GPU__ [[cpu]] [[hc]]
#else
#define __CPU_GPU__
#endif

#include "kalmar_short_vectors.inl"

#undef __CPU_GPU__

} // namespace short_vector

} // namespace hc

#endif // _HC_SHORT_VECTORS_H
