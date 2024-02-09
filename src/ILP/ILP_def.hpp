#pragma once

#include "ILP.hpp"

#ifdef USE_DENSE
    using ILP = ilp::IntegerLinearProgramBuilder<ilp::DenseStorage>;
    using Exp = ilp::LinearOperation<ilp::DenseStorage>;
    using Var = ilp::Variable<ilp::DenseStorage>;
#else
    using ILP = ilp::IntegerLinearProgramBuilder<ilp::SparseStorage>;
    using Exp = ilp::LinearOperation<ilp::SparseStorage>;
    using Var = ilp::Variable<ilp::SparseStorage>;
#endif