#include "xcint.h"

#include <math.h>
#include <time.h>
#include <assert.h>

#include <cstdlib>
#include <fstream>
#include <algorithm>

#include "rolex.h"
#include "XCint.h"
#include "AOBatch.h"
#include "MemAllocator.h"

#include "xcint_parameters.h"
#include "parameters.h"
#include "xcint_c_parameters.h"

#ifdef ENABLE_OMP
#include "omp.h"
#endif

#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)


xcint_context_t *xcint_new()
{
    return AS_TYPE(xcint_context_t, new XCint());
}
XCint::XCint()
{
    nullify();
}


void xcint_free(xcint_context_t *context)
{
    if (!context) return;
    delete AS_TYPE(XCint, context);
}
XCint::~XCint()
{
    nullify();
}


int xcint_set_functional(xcint_context_t *context,
                         const char   *line)
{
    return AS_TYPE(XCint, context)->set_functional(line);
}


int xcint_set_basis(xcint_context_t *context,
                    const int    basis_type,
                    const int    num_centers,
                    const double center_coordinates[],
                    const int    center_elements[],
                    const int    num_shells,
                    const int    shell_centers[],
                    const int    l_quantum_numbers[],
                    const int    shell_num_primitives[],
                    const double primitive_exponents[],
                    const double contraction_coefficients[])
{
    return AS_TYPE(XCint, context)->set_basis(basis_type,
                                              num_centers,
                                              center_coordinates,
                                              center_elements,
                                              num_shells,
                                              shell_centers,
                                              l_quantum_numbers,
                                              shell_num_primitives,
                                              primitive_exponents,
                                              contraction_coefficients);
}


void XCint::nullify()
{
    reset_time();
}


int XCint::set_functional(const char *line)
{
    fun.set_functional(line);
    return 0;
}


int XCint::set_basis(const int    basis_type,
                     const int    num_centers,
                     const double center_coordinates[],
                     const int    center_elements[],
                     const int    num_shells,
                     const int    shell_centers[],
                     const int    l_quantum_numbers[],
                     const int    shell_num_primitives[],
                     const double primitive_exponents[],
                     const double contraction_coefficients[])
{
    basis.init(basis_type,
               num_centers,
               center_coordinates,
               center_elements,
               num_shells,
               shell_centers,
               l_quantum_numbers,
               shell_num_primitives,
               primitive_exponents,
               contraction_coefficients);

//#define CREATE_UNIT_TEST
#ifdef CREATE_UNIT_TEST
    printf("-------------------->8\n");

    printf("#include \"gtest/gtest.h\"\n");
    printf("#include \"XCint.h\"\n");
    printf("#include \"MemAllocator.h\"\n");
    printf("#include \"xcint_c_api.h\"\n");

    printf("\n");

    printf("TEST(testname, subtestname)\n");
    printf("{\n");

    printf("    XCint xc;\n");
    printf("    xc.set_verbosity(0);\n");

    printf("    int num_centers;\n");
    printf("    int num_shells;\n");

    printf("    double *center_coordinates = NULL;\n");
    printf("    int *center_elements = NULL;\n");
    printf("    int *shell_centers = NULL;\n");
    printf("    int *l_quantum_numbers = NULL;\n");
    printf("    int *shell_num_primitives = NULL;\n");
    printf("    double *primitive_exponents = NULL;\n");
    printf("    double *contraction_coefficients = NULL;\n");
    printf("    size_t block_size;\n");

    printf("    num_centers = %i;\n", num_centers);
    printf("    num_shells = %i;\n", num_shells);

    printf("    block_size = 3*num_centers*sizeof(double);\n");
    printf("    center_coordinates = (double*) MemAllocator::allocate(block_size);\n");
    for (int i = 0; i < 3*num_centers; i++)
    {
        printf("    center_coordinates[%i] = %20.12e;\n", i, center_coordinates[i]);
    }

    printf("    block_size = num_centers*sizeof(int);\n");
    printf("    center_elements = (int*) MemAllocator::allocate(block_size);\n");
    for (int i = 0; i < num_centers; i++)
    {
        printf("    center_elements[%i] = %i;\n", i, center_elements[i]);
    }

    printf("    block_size = num_shells*sizeof(int);\n");
    printf("    shell_centers = (int*) MemAllocator::allocate(block_size);\n");
    printf("    l_quantum_numbers = (int*) MemAllocator::allocate(block_size);\n");
    printf("    shell_num_primitives = (int*) MemAllocator::allocate(block_size);\n");
    for (int i = 0; i < num_shells; i++)
    {
        printf("    shell_centers[%i] = %i;\n", i, shell_centers[i]);
        printf("    l_quantum_numbers[%i] = %i;\n", i, l_quantum_numbers[i]);
        printf("    shell_num_primitives[%i] = %i;\n", i, shell_num_primitives[i]);
    }

    int n = 0;
    for (int i = 0; i < num_shells; i++)
    {
        n += shell_num_primitives[i];
    }

    printf("    block_size = %i*sizeof(double);\n", n);
    printf("    primitive_exponents = (double*) MemAllocator::allocate(block_size);\n");
    printf("    contraction_coefficients = (double*) MemAllocator::allocate(block_size);\n");

    n = 0;
    for (int i = 0; i < num_shells; i++)
    {
        for (int j = 0; j < shell_num_primitives[i]; j++)
        {
            printf("    primitive_exponents[%i] = %20.12e;\n", n, primitive_exponents[n]);
            printf("    contraction_coefficients[%i] = %20.12e;\n", n, contraction_coefficients[n]);
            n++;
        }
    }
#endif // CREATE_UNIT_TEST
    return 0;
}


void XCint::integrate_batch(      double dmat[],
                            const int    get_xc_energy,
                                  double &xc_energy,
                            const int    get_xc_mat,
                                  double xc_mat[],
                                  double &num_electrons,
                            const int    geo_coor[],
                            const bool   use_dmat[],
                            const int    ipoint,
                            const int    geo_derv_order,
                            const int    max_ao_order_g,
                            const int    block_length,
                            const int    num_variables,
                            const int    num_pert,
                            const int    num_fields,
                            const int    mat_dim,
                            const bool   get_gradient,
                            const bool   get_tau,
                            const int    dmat_index[],
                            const double grid_pw[])
{
    AOBatch batch;

    size_t block_size = AO_BLOCK_LENGTH*num_variables*MAX_NUM_DENSITIES*sizeof(double);
    double *n = (double*) MemAllocator::allocate(block_size);
    block_size = AO_BLOCK_LENGTH*num_variables*sizeof(double);
    double *u = (double*) MemAllocator::allocate(block_size);

    std::vector<int> coor;
    double prefactors[5] = {1.0, 2.0, 2.0, 2.0, 0.5};

    bool n_is_used[MAX_NUM_DENSITIES];
    std::fill(&n_is_used[0], &n_is_used[MAX_NUM_DENSITIES], false);

    rolex::start_partial();

    batch.get_ao(basis,
                 get_gradient,
                 max_ao_order_g,
                 &grid_pw[ipoint*4]);

    time_ao += rolex::stop_partial();

    rolex::start_partial();

    if (!n_is_used[0])
    {
        std::fill(&n[0], &n[block_length*num_variables], 0.0);
        n_is_used[0] = true;
    }

    batch.get_density_undiff(mat_dim,
                             get_gradient,
                             get_tau,
                             prefactors,
                             n,
                             dmat,
                             true,
                             true);

    for (int ib = 0; ib < block_length; ib++)
    {
        num_electrons += grid_pw[(ipoint + ib)*4 + 3]*n[ib];
    }

    time_densities += rolex::stop_partial();

    // expectation value contribution
    if (get_xc_energy)
    {

        double *xcin = NULL;
        double *xcout = NULL;

        assert(geo_derv_order < 5);


        //                       *
        //                       *  *
        //                       *  *  *  *
        //                       *  *  *  *  *  *  *  *
        //                       *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
        //           i j k l     0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
        //   ------------------------------------------------------------------
        //    0 0                0
        //    1 i    *           1  0
        //    2 j      *         2     0
        //    3 ij   * *         3  2  1  0
        //    4 k        *       4           0
        //    5 ik   *   *       5  4        1  0
        //    6 jk     * *       6     4     2     0
        //    7 ijk  * * *       7  6  5  4  3  2  1  0
        //    8 l          *     8                       0
        //    9 il   *     *     9  8                    1  0
        //   10 jl     *   *    10     8                 2     0
        //   11 ijl  * *   *    11 10  9  8              3  2  1  0
        //   12 kl       * *    12           8           4           0
        //   13 ikl  *   * *    13 12        9  8        5  4        1  0
        //   14 jkl    * * *    14    12    10     8     6     4     2     0
        //   15 ijkl * * * *    15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0

        // this takes care of the first column
        for (int k = 1; k < MAX_NUM_DENSITIES; k++)
        {
            if (use_dmat[k])
            {
                if (!n_is_used[k])
                {
                    std::fill(&n[k*block_length*num_variables], &n[(k+1)*block_length*num_variables], 0.0);
                    n_is_used[k] = true;
                }
                batch.get_density_undiff(mat_dim,
                                         get_gradient,
                                         get_tau,
                                         prefactors,
                                         &n[k*block_length*num_variables],
                                         &dmat[dmat_index[k]],
                                         false,
                                         false); // FIXME can be true based on dmat, saving possible
            }
        }

        rolex::start_partial();

        #include "ave_contributions.h"

        time_densities += rolex::stop_partial();

        fun.set_order(num_pert);

        size_t block_size = num_variables*fun.dens_offset*block_length*sizeof(double);
        xcin = (double*) MemAllocator::allocate(block_size);
        std::fill(&xcin[0], &xcin[num_variables*fun.dens_offset*block_length], 0.0);

        block_size = fun.dens_offset*block_length*sizeof(double);
        xcout = (double*) MemAllocator::allocate(block_size);

        for (int k = 0; k < MAX_NUM_DENSITIES; k++)
        {
            if (n_is_used[k])
            {
                for (int ivar = 0; ivar < num_variables; ivar++)
                {
                    for (int ib = 0; ib < block_length; ib++)
                    {
                        xcin[ib*num_variables*fun.dens_offset + ivar*fun.dens_offset + k] = n[k*block_length*num_variables + ivar*block_length + ib];
                    }
                }
            }
        }

        rolex::start_partial();

        double sum = 0.0;
        for (int ib = 0; ib < block_length; ib++)
        {
            if (n[ib] > 1.0e-14 and fabs(grid_pw[(ipoint + ib)*4 + 3]) > 1.0e-30)
            {
                xc_eval(fun.fun, &xcin[ib*num_variables*fun.dens_offset], &xcout[ib*fun.dens_offset]);
                sum += xcout[ib*fun.dens_offset + fun.dens_offset - 1]*grid_pw[(ipoint + ib)*4 + 3];
            }
        }
        xc_energy += sum;

        time_fun_derv += rolex::stop_partial();

        MemAllocator::deallocate(xcin);
        MemAllocator::deallocate(xcout);
    }


    // matrix contribution
    if (get_xc_mat)
    {
        int k;

        bool contribution_is_implemented = false;

        if (geo_derv_order == 0) // no geo dervs
        {
            contribution_is_implemented = true;

            rolex::start_partial();

            for (int ifield = 0; ifield < num_fields; ifield++)
            {
                k = (int)pow(2, ifield); // FIXME double check this
                if (!n_is_used[k])
                {
                    std::fill(&n[k*block_length*num_variables], &n[(k+1)*block_length*num_variables], 0.0);
                    n_is_used[k] = true;
                }
                batch.get_density_undiff(mat_dim,
                                         get_gradient,
                                         get_tau,
                                         prefactors,
                                         &n[k*block_length*num_variables],
                                         &dmat[(ifield+1)*mat_dim*mat_dim],
                                         false,
                                         false); // FIXME can be true depending on perturbation (savings possible)
            }

            time_densities += rolex::stop_partial();

            distribute_matrix(block_length,
                              num_variables,
                              num_pert,
                              mat_dim,
                              prefactors,
                              ipoint,
                              n_is_used,
                              n,
                              u,
                              xc_mat,
                              xc_energy,
                              coor,
                              batch,
                              grid_pw);
        }

        if (geo_derv_order > 0) // we have geo dervs
        {
            if (geo_derv_order == 1 && num_fields == 0)
            {
                contribution_is_implemented = true;

                // M    d_nn  n_i
                k = 1;
                if (!n_is_used[k])
                {
                    std::fill(&n[k*block_length*num_variables], &n[(k+1)*block_length*num_variables], 0.0);
                    n_is_used[k] = true;
                }
                coor.push_back(geo_coor[0]);
                batch.get_dens_geo_derv(basis,
                                        mat_dim,
                                        get_gradient,
                                        get_tau,
                                        coor,
                                        &n[k*block_length*num_variables],
                                        true,
                                        &dmat[0]);
                coor.clear();
                distribute_matrix(block_length,
                                  num_variables,
                                  1,
                                  mat_dim,
                                  prefactors,
                                  ipoint,
                                  n_is_used,
                                  n,
                                  u,
                                  xc_mat,
                                  xc_energy,
                                  coor,
                                  batch,
                                  grid_pw);
                n_is_used[1] = false;

                // M_i  d_n
                coor.push_back(geo_coor[0]);
                distribute_matrix(block_length,
                                  num_variables,
                                  0,
                                  mat_dim,
                                  prefactors,
                                  ipoint,
                                  n_is_used,
                                  n,
                                  u,
                                  xc_mat,
                                  xc_energy,
                                  coor,
                                  batch,
                                  grid_pw);
                coor.clear();
            }

            if (geo_derv_order == 2 && num_fields == 0)
            {
                contribution_is_implemented = true;

                // M_ij d_n
                coor.push_back(geo_coor[0]);
                coor.push_back(geo_coor[1]);
                distribute_matrix(block_length,
                                  num_variables,
                                  0,
                                  mat_dim,
                                  prefactors,
                                  ipoint,
                                  n_is_used,
                                  n,
                                  u,
                                  xc_mat,
                                  xc_energy,
                                  coor,
                                  batch,
                                  grid_pw);
                coor.clear();

                // FIXME add shortcut if i == j
                // M_i  d_nn  n_j
                k = 1;
                if (!n_is_used[k])
                {
                    std::fill(&n[k*block_length*num_variables], &n[(k+1)*block_length*num_variables], 0.0);
                    n_is_used[k] = true;
                }
                coor.push_back(geo_coor[1]);
                batch.get_dens_geo_derv(basis,
                                        mat_dim,
                                        get_gradient,
                                        get_tau,
                                        coor,
                                        &n[k*block_length*num_variables],
                                        true,
                                        &dmat[0]);
                coor.clear();
                coor.push_back(geo_coor[0]);
                distribute_matrix(block_length,
                                  num_variables,
                                  1,
                                  mat_dim,
                                  prefactors,
                                  ipoint,
                                  n_is_used,
                                  n,
                                  u,
                                  xc_mat,
                                  xc_energy,
                                  coor,
                                  batch,
                                  grid_pw);
                coor.clear();
                n_is_used[1] = false;

                // M_j  d_nn  n_i
                k = 1;
                if (!n_is_used[k])
                {
                    std::fill(&n[k*block_length*num_variables], &n[(k+1)*block_length*num_variables], 0.0);
                    n_is_used[k] = true;
                }
                coor.push_back(geo_coor[0]);
                batch.get_dens_geo_derv(basis,
                                        mat_dim,
                                        get_gradient,
                                        get_tau,
                                        coor,
                                        &n[k*block_length*num_variables],
                                        true,
                                        &dmat[0]);
                coor.clear();
                coor.push_back(geo_coor[1]);
                distribute_matrix(block_length,
                                  num_variables,
                                  1,
                                  mat_dim,
                                  prefactors,
                                  ipoint,
                                  n_is_used,
                                  n,
                                  u,
                                  xc_mat,
                                  xc_energy,
                                  coor,
                                  batch,
                                  grid_pw);
                coor.clear();
                n_is_used[1] = false;

                // M    d_nnn n_i n_j
                // M    d_nn  n_ij
                // FIXME stupidly recalculating n_i and n_j
                k = 1;
                if (!n_is_used[k])
                {
                    std::fill(&n[k*block_length*num_variables], &n[(k+1)*block_length*num_variables], 0.0);
                    n_is_used[k] = true;
                }
                coor.push_back(geo_coor[0]);
                batch.get_dens_geo_derv(basis,
                                        mat_dim,
                                        get_gradient,
                                        get_tau,
                                        coor,
                                        &n[k*block_length*num_variables],
                                        true,
                                        &dmat[0]);
                coor.clear();
                k = 2;
                if (!n_is_used[k])
                {
                    std::fill(&n[k*block_length*num_variables], &n[(k+1)*block_length*num_variables], 0.0);
                    n_is_used[k] = true;
                }
                coor.push_back(geo_coor[1]);
                batch.get_dens_geo_derv(basis,
                                        mat_dim,
                                        get_gradient,
                                        get_tau,
                                        coor,
                                        &n[k*block_length*num_variables],
                                        true,
                                        &dmat[0]);
                coor.clear();
                k = 3;
                if (!n_is_used[k])
                {
                    std::fill(&n[k*block_length*num_variables], &n[(k+1)*block_length*num_variables], 0.0);
                    n_is_used[k] = true;
                }
                coor.push_back(geo_coor[0]);
                coor.push_back(geo_coor[1]);
                batch.get_dens_geo_derv(basis,
                                        mat_dim,
                                        get_gradient,
                                        get_tau,
                                        coor,
                                        &n[k*block_length*num_variables],
                                        true,
                                        &dmat[0]);
                coor.clear();
                distribute_matrix(block_length,
                                  num_variables,
                                  2,
                                  mat_dim,
                                  prefactors,
                                  ipoint,
                                  n_is_used,
                                  n,
                                  u,
                                  xc_mat,
                                  xc_energy,
                                  coor,
                                  batch,
                                  grid_pw);
                n_is_used[1] = false;
                n_is_used[2] = false;
                n_is_used[3] = false;
            }

            if (geo_derv_order == 1 && num_fields == 1)
            {
                contribution_is_implemented = true;

                // M_i  d_nn  n_a
                k = 1;
                if (!n_is_used[k])
                {
                    std::fill(&n[k*block_length*num_variables], &n[(k+1)*block_length*num_variables], 0.0);
                    n_is_used[k] = true;
                }
                batch.get_density_undiff(mat_dim,
                                         get_gradient,
                                         get_tau,
                                         prefactors,
                                         &n[k*block_length*num_variables],
                                         &dmat[1*mat_dim*mat_dim],
                                         false,
                                         false); // FIXME can be true depending on perturbation (savings possible)
                coor.push_back(geo_coor[0]);
                distribute_matrix(block_length,
                                  num_variables,
                                  1,
                                  mat_dim,
                                  prefactors,
                                  ipoint,
                                  n_is_used,
                                  n,
                                  u,
                                  xc_mat,
                                  xc_energy,
                                  coor,
                                  batch,
                                  grid_pw);
                coor.clear();

                // M    d_nnn n_i n_a
                // M    d_nn  n_ia
                k = 2;
                if (!n_is_used[k])
                {
                    std::fill(&n[k*block_length*num_variables], &n[(k+1)*block_length*num_variables], 0.0);
                    n_is_used[k] = true;
                }
                coor.push_back(geo_coor[0]);
                batch.get_dens_geo_derv(basis,
                                        mat_dim,
                                        get_gradient,
                                        get_tau,
                                        coor,
                                        &n[k*block_length*num_variables],
                                        true,
                                        &dmat[0]);
                coor.clear();
                k = 3;
                if (!n_is_used[k])
                {
                    std::fill(&n[k*block_length*num_variables], &n[(k+1)*block_length*num_variables], 0.0);
                    n_is_used[k] = true;
                }
                coor.push_back(geo_coor[0]);
                batch.get_dens_geo_derv(basis,
                                        mat_dim,
                                        get_gradient,
                                        get_tau,
                                        coor,
                                        &n[k*block_length*num_variables],
                                        true,
                                        &dmat[1*mat_dim*mat_dim]);
                coor.clear();
                distribute_matrix(block_length,
                                  num_variables,
                                  2,
                                  mat_dim,
                                  prefactors,
                                  ipoint,
                                  n_is_used,
                                  n,
                                  u,
                                  xc_mat,
                                  xc_energy,
                                  coor,
                                  batch,
                                  grid_pw);
                n_is_used[1] = false;
                n_is_used[2] = false;
                n_is_used[3] = false;
            }
        }

        if (!contribution_is_implemented)
        {
            fprintf(stderr, "ERROR: XCint matrix contribution for geo_derv_order=%i and num_fields=%i not implemented\n", geo_derv_order, num_fields);
            exit(-1);
        }
    }

    MemAllocator::deallocate(n);
    MemAllocator::deallocate(u);
}


void xcint_integrate(xcint_context_t *context,
                     const int    mode,
                     const int    num_points,
                     const double grid_pw[],
                     const int    num_pert,
                     const int    pert[],
                     const int    comp[],
                     const int    num_dmat,
                     const int    dmat_to_pert[],
                     const int    dmat_to_comp[],
                           double dmat[],
                     const int    get_xc_energy,
                           double &xc_energy,
                     const int    get_xc_mat,
                           double xc_mat[],
                           double &num_electrons)
{
    return AS_TYPE(XCint, context)->integrate(mode,
                                              num_points,
                                              grid_pw,
                                              num_pert,
                                              pert,
                                              comp,
                                              num_dmat,
                                              dmat_to_pert,
                                              dmat_to_comp,
                                              dmat,
                                              get_xc_energy,
                                              xc_energy,
                                              get_xc_mat,
                                              xc_mat,
                                              num_electrons);
}
void XCint::integrate(const int    mode,
                      const int    num_points,
                      const double grid_pw[],
                      const int    num_pert,
                      const int    pert[],
                      const int    comp[],
                      const int    num_dmat,
                      const int    dmat_to_pert[],
                      const int    dmat_to_comp[],
                            double dmat[],
                      const int    get_xc_energy,
                            double &xc_energy,
                      const int    get_xc_mat,
                            double xc_mat[],
                            double &num_electrons)
{
    int num_variables;
    int max_ao_order_g;
    size_t block_size;

    double a;
    double prefactors[5] = {1.0, 2.0, 2.0, 2.0, 0.5};

    double *n = NULL;
    double *u = NULL;

    bool contribution_is_implemented;

    std::vector<int> coor;

    rolex::start_global();

    int rank = 0;
    int num_proc = 1;

    int mat_dim;
    if (rank == 0) mat_dim = basis.get_num_ao();

    int geo_derv_order = 0;
    int num_fields = 0;
    for (int i = 0; i < num_pert; i++)
    {
        if (pert[i] == XCINT_PERT_GEO) geo_derv_order++;
        if (pert[i] == XCINT_PERT_EL)  num_fields++;
    }

    int *geo_coor = NULL;
    if (geo_derv_order > 0)
    {
        block_size = geo_derv_order*sizeof(int);
        geo_coor = (int*) MemAllocator::allocate(block_size);
        for (int i = 0; i < num_pert; i++)
        {
            if (pert[i] == XCINT_PERT_GEO) geo_coor[i] = comp[2*i]; // FIXME
        }
    }

    assert(mode == XCINT_MODE_RKS);

    xc_energy = 0.0;
    num_electrons = 0.0;

    if (get_xc_mat) std::fill(&xc_mat[0], &xc_mat[mat_dim*mat_dim], 0.0);

    bool get_gradient;
    bool get_tau;

    max_ao_order_g = 0;

    if (fun.is_tau_mgga)
    {
        num_variables = 5;
        get_gradient = true;
        get_tau = true;
        max_ao_order_g++;
    }
    else if (fun.is_gga)
    {
        num_variables = 4;
        get_gradient = true;
        get_tau = false;
        max_ao_order_g++;
    }
    else
    {
        num_variables = 1;
        get_gradient = false;
        get_tau = false;
    }

    for (int i = 0; i < geo_derv_order; i++)
    {
        max_ao_order_g++;
    }

//  FIXME
//  this causes the geo_off array to become
//  too small and leads to out of bounds access
//  basis.set_geo_off(max_ao_order_g);

    rolex::start_partial();

    bool *use_dmat = NULL;
    int  *dmat_index = NULL;

    block_size = MAX_NUM_DENSITIES*sizeof(int);

    dmat_index = (int*) MemAllocator::allocate(block_size);
    std::fill(&dmat_index[0], &dmat_index[MAX_NUM_DENSITIES], 0);

    use_dmat = (bool*) MemAllocator::allocate(block_size);
    std::fill(&use_dmat[0], &use_dmat[MAX_NUM_DENSITIES], false);

    assert(num_dmat <= MAX_NUM_DENSITIES);

    for (int k = 0; k < num_dmat; k++)
    {
        use_dmat[dmat_to_pert[k]] = true;
        dmat_index[dmat_to_pert[k]] = k*mat_dim*mat_dim;
    }

    assert(num_pert < 7);

    int block_length;

#ifdef ENABLE_OMP
    int num_threads = 0;

    #pragma omp parallel
    {
        if (omp_get_thread_num() == 0) num_threads = omp_get_num_threads();
    }

    double *xc_energy_buffer = NULL;
    if (get_xc_energy) xc_energy_buffer = (double*) MemAllocator::allocate(num_threads*sizeof(double));

    double *num_electrons_buffer = (double*) MemAllocator::allocate(num_threads*sizeof(double));

    double *xc_mat_buffer = NULL;
    if (get_xc_mat)
    {
        xc_mat_buffer = (double*) MemAllocator::allocate(num_threads*mat_dim*mat_dim*sizeof(double));
        std::fill(&xc_mat_buffer[0], &xc_mat_buffer[num_threads*mat_dim*mat_dim], 0.0);
    }

#ifdef CREATE_UNIT_TEST
    for (int i = 0; i < mat_dim*mat_dim; i++)
    {
        if (fabs(dmat[i]) < 1.0e-10) dmat[i] = 0.0;
    }
#endif // CREATE_UNIT_TEST

    #pragma omp parallel
    {
        int ithread = omp_get_thread_num();

        double xc_energy_local = 0.0;
        double num_electrons_local = 0.0;

        double *xc_mat_local = NULL;
        if (get_xc_mat) xc_mat_local = &xc_mat_buffer[ithread*mat_dim*mat_dim];

        #pragma omp for schedule(dynamic)
#else
        double xc_energy_local = xc_energy;
        double num_electrons_local = num_electrons;
        double *xc_mat_local = NULL;
        if (get_xc_mat) xc_mat_local = &xc_mat[0];
#endif
        for (int ibatch = 0; ibatch < num_points/AO_BLOCK_LENGTH; ibatch++)
        {
            int ipoint = ibatch*AO_BLOCK_LENGTH;

            block_length = AO_BLOCK_LENGTH;

            integrate_batch(dmat,
                            get_xc_energy,
                            xc_energy_local,
                            get_xc_mat,
                            xc_mat_local,
                            num_electrons_local,
                            geo_coor,
                            use_dmat,
                            ipoint,
                            geo_derv_order,
                            max_ao_order_g,
                            block_length,
                            num_variables,
                            num_pert,
                            num_fields,
                            mat_dim,
                            get_gradient,
                            get_tau,
                            dmat_index,
                            grid_pw);
        }

#ifdef ENABLE_OMP
        if (get_xc_energy) xc_energy_buffer[ithread] = xc_energy_local;
        num_electrons_buffer[ithread] = num_electrons_local;
    }

    for (size_t ithread = 0; ithread < num_threads; ithread++)
    {
        num_electrons += num_electrons_buffer[ithread];
        if (get_xc_energy) xc_energy += xc_energy_buffer[ithread];
        if (get_xc_mat)
        {
            // FIXME consider using blas daxpy for this
            for (int i = 0; i < mat_dim*mat_dim; i++)
            {
                xc_mat[i] += xc_mat_buffer[ithread*mat_dim*mat_dim + i];
            }
        }
    }

    MemAllocator::deallocate(num_electrons_buffer);
    MemAllocator::deallocate(xc_energy_buffer);
    MemAllocator::deallocate(xc_mat_buffer);
#else
    xc_energy = xc_energy_local;
    num_electrons = num_electrons_local;
#endif

    MemAllocator::deallocate(use_dmat);
    MemAllocator::deallocate(dmat_index);
    MemAllocator::deallocate(geo_coor);

    if (get_xc_mat)
    {
        // symmetrize result matrix
        if (rank == 0)
        {
            for (int k = 0; k < mat_dim; k++)
            {
                for (int l = 0; l < k; l++)
                {
                    a = xc_mat[k*mat_dim + l] + xc_mat[l*mat_dim + k];
                    xc_mat[k*mat_dim + l] = 0.5*a;
                    xc_mat[l*mat_dim + k] = 0.5*a;
                }
            }
        }
#ifdef CREATE_UNIT_TEST
    printf("    double hfx, mu, beta; // we don't care about it here\n");

    printf("    xc.set_functional(\"lda\", hfx, mu, beta);\n"); // FIXME
    printf("    xc.set_basis(XCINT_BASIS_SPHERICAL,\n");
    printf("                 num_centers,\n");
    printf("                 center_coordinates,\n");
    printf("                 center_elements,\n");
    printf("                 num_shells,\n");
    printf("                 shell_centers,\n");
    printf("                 l_quantum_numbers,\n");
    printf("                 shell_num_primitives,\n");
    printf("                 primitive_exponents,\n");
    printf("                 contraction_coefficients);\n");
    printf("    xc.generate_grid(1.0e-12,\n");
    printf("                     86,\n");
    printf("                     302,\n");
    printf("                     num_centers,\n");
    printf("                     center_coordinates,\n");
    printf("                     center_elements,\n");
    printf("                     num_shells,\n");
    printf("                     shell_centers,\n");
    printf("                     l_quantum_numbers,\n");
    printf("                     shell_num_primitives,\n");
    printf("                     primitive_exponents);\n");

    printf("    int mat_dim = %i;\n", mat_dim);

    printf("    double *dmat = NULL;\n");
    printf("    double *xc_mat = NULL;\n");
    printf("    block_size = mat_dim*mat_dim*sizeof(double);\n");
    printf("    dmat = (double*) MemAllocator::allocate(block_size);\n");
    printf("    std::fill(&dmat[0], &dmat[mat_dim*mat_dim], 0.0);\n");
    printf("    xc_mat = (double*) MemAllocator::allocate(block_size);\n");

    for (int i = 0; i < mat_dim*mat_dim; i++)
    {
        if (fabs(dmat[i]) > 1.0e-10) printf("    dmat[%i] = %20.12e;\n", i, dmat[i]);
    }

    printf("    double xc_energy = 0.0;\n");
    printf("    double num_electrons = 0.0;\n");
    printf("    int dmat_to_pert[1]  = {0};\n");
    printf("    int dmat_to_comp[1]  = {0};\n");
    printf("    xc.integrate(XCINT_MODE_RKS,\n");
    printf("                 0,\n");
    printf("                 0,\n");
    printf("                 0,\n");
    printf("                 1,\n");
    printf("                 dmat_to_pert,\n");
    printf("                 dmat_to_comp,\n");
    printf("                 dmat,\n");
    printf("                 false,\n");
    printf("                 xc_energy,\n");
    printf("                 true,\n");
    printf("                 xc_mat,\n");
    printf("                 num_electrons);\n");

    printf("    ASSERT_NEAR(num_electrons, %20.12e, 1.0e-11);\n", num_electrons);

    double dot = 0.0;
    for (int i = 0; i < mat_dim*mat_dim; i++)
    {
        dot += xc_mat[i]*dmat[i];
    }
    printf("    double dot = 0.0;\n");
    printf("    for (int i = 0; i < mat_dim*mat_dim; i++)\n");
    printf("    {\n");
    printf("        dot += xc_mat[i]*dmat[i];\n");
    printf("    }\n");
    printf("    ASSERT_NEAR(dot, %20.12e, 1.0e-11);\n", dot);

    printf("    MemAllocator::deallocate(dmat);\n");
    printf("    MemAllocator::deallocate(xc_mat);\n");
    printf("    MemAllocator::deallocate(center_coordinates);\n");
    printf("    MemAllocator::deallocate(center_elements);\n");
    printf("    MemAllocator::deallocate(shell_centers);\n");
    printf("    MemAllocator::deallocate(l_quantum_numbers);\n");
    printf("    MemAllocator::deallocate(shell_num_primitives);\n");
    printf("    MemAllocator::deallocate(primitive_exponents);\n");
    printf("    MemAllocator::deallocate(contraction_coefficients);\n");

    printf("}\n");

    printf("-------------------->8\n");
#endif // CREATE_UNIT_TEST
    }

    time_total += rolex::stop_global();
}


void XCint::distribute_matrix(const int              block_length,
                              const int              num_variables,
                              const int              num_pert,
                              const int              mat_dim,
                              const double           prefactors[],
                              const int              w_off,
                              const bool             n_is_used[],
                              const double           n[],
                                    double           u[],
                                    double           xc_mat[],
                                    double           &xc_energy,
                              const std::vector<int> coor,
                                    AOBatch     &batch,
                              const double           grid_pw[])
{
    rolex::start_partial();

    int off;

    double *xcin = NULL;
    double *xcout = NULL;
    size_t block_size;

    fun.set_order(num_pert + 1);

    block_size = num_variables*fun.dens_offset*block_length*sizeof(double);
    xcin = (double*) MemAllocator::allocate(block_size);
    std::fill(&xcin[0], &xcin[num_variables*fun.dens_offset*block_length], 0.0);

    block_size = fun.dens_offset*block_length*sizeof(double);
    xcout = (double*) MemAllocator::allocate(block_size);

    std::fill(&u[0], &u[block_length*num_variables], 0.0);

    for (int k = 0; k < MAX_NUM_DENSITIES; k++)
    {
        if (n_is_used[k])
        {
            for (int ivar = 0; ivar < num_variables; ivar++)
            {
                for (int ib = 0; ib < block_length; ib++)
                {
                    xcin[ib*num_variables*fun.dens_offset + ivar*fun.dens_offset + k] = n[k*block_length*num_variables + ivar*block_length + ib];
                }
            }
        }
    }

    for (int ivar = 0; ivar < num_variables; ivar++)
    {
        for (int jvar = 0; jvar < num_variables; jvar++)
        {
            off = jvar*fun.dens_offset + (int)pow(2, num_pert);
            if (ivar == jvar)
            {
                for (int ib = 0; ib < block_length; ib++)
                {
                    xcin[off + ib*num_variables*fun.dens_offset] = 1.0;
                }
            }
            else
            {
                for (int ib = 0; ib < block_length; ib++)
                {
                    xcin[off + ib*num_variables*fun.dens_offset] = 0.0;
                }
            }
        }

        off = ivar*block_length;
        std::fill(&xcout[0], &xcout[fun.dens_offset*block_length], 0.0);
        for (int ib = 0; ib < block_length; ib++)
        {
            if (n[ib] > 1.0e-14 and fabs(grid_pw[(w_off + ib)*4 + 3]) > 1.0e-30)
            {
                xc_eval(fun.fun, &xcin[ib*num_variables*fun.dens_offset], &xcout[ib*fun.dens_offset]);
                u[off + ib] += xcout[(ib+1)*fun.dens_offset - 1]*grid_pw[(w_off + ib)*4 + 3];
            }
        }
    }

    time_fun_derv += rolex::stop_partial();

    rolex::start_partial();

    bool distribute_gradient;
    bool distribute_tau;

    if (fun.is_tau_mgga)
    {
        distribute_gradient = true;
        distribute_tau = true;
    }
    else if (fun.is_gga)
    {
        distribute_gradient = true;
        distribute_tau = false;
    }
    else
    {
        distribute_gradient = false;
        distribute_tau = false;
    }

    if (coor.size() == 0)
    {
        batch.distribute_matrix_undiff(mat_dim,
                                       distribute_gradient,
                                       distribute_tau,
                                       prefactors,
                                       u,
                                       xc_mat);
    }
    else
    {
        batch.get_dens_geo_derv(basis,
                                mat_dim,
                                distribute_gradient,
                                distribute_tau,
                                coor,
                                u,
                                false,
                                xc_mat);
    }

    for (int ib = 0; ib < block_length; ib++)
    {
        xc_energy += xcout[ib*fun.dens_offset]*grid_pw[(w_off + ib)*4 + 3];
    }

    MemAllocator::deallocate(xcin);
    MemAllocator::deallocate(xcout);

    time_matrix_distribution += rolex::stop_partial();
}


void XCint::reset_time()
{
    time_total = 0.0;
    time_ao = 0.0;
    time_fun_derv = 0.0;
    time_densities = 0.0;
    time_matrix_distribution = 0.0;
}
