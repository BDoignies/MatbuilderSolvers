cmake_minimum_required(VERSION 3.15)

project(Matbuilder)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

include(galois)
include_directories(${galois_SOURCE_DIR}/include)
include_directories(src/)

IF (BUILD_CPLEX)
    set(CPLEX_INC "/opt/ibm/ILOG/CPLEX_Studio2211/cplex/include")
    set(CPLEX_INC2 "/opt/ibm/ILOG/CPLEX_Studio2211/concert/include")
    set(CPLEX_LIB "/opt/ibm/ILOG/CPLEX_Studio2211/cplex/lib/x86-64_linux/static_pic")
    set(CPLEX_LIB2 "/opt/ibm/ILOG/CPLEX_Studio2211/concert/lib/x86-64_linux/static_pic")

    add_executable(matbuilder_cplex src/main_cplex.cpp src/utils/GFMatrix.cpp src/Matbuilder/Parser.cpp src/Matbuilder/Solver.cpp src/Matbuilder/Constraints.cpp src/ILP/backends/CPLEX.cpp)

    target_include_directories(matbuilder_cplex PRIVATE ${CPLEX_INC} ${CPLEX_INC2})
    target_link_directories(matbuilder_cplex PRIVATE ${CPLEX_LIB} ${CPLEX_LIB2} )
    target_link_libraries(matbuilder_cplex PRIVATE galois++ concert ilocplex cplex m pthread dl)
ENDIF()

IF (BUILD_GLPK)
    add_executable(matbuilder_glpk src/main_glpk.cpp src/utils/GFMatrix.cpp src/Matbuilder/Parser.cpp src/Matbuilder/Solver.cpp src/Matbuilder/Constraints.cpp src/ILP/backends/GLPK.cpp)
    target_link_libraries(matbuilder_glpk PRIVATE galois++ glpk)
ENDIF()
