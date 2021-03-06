set(AO_CHUNK_LENGTH "32" CACHE STRING "AO chunk length")
set(MAX_GEO_DIFF_ORDER "5" CACHE STRING "Maximum geometric differentiation order")
set(MAX_L_VALUE "5" CACHE STRING "Maximum L value")

configure_file(
    ${PROJECT_SOURCE_DIR}/balboa/parameters.h.in
    ${PROJECT_BINARY_DIR}/generated/balboa_parameters.h
    @ONLY
    )
