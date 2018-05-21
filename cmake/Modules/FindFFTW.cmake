# Find the FFTW library
#
# Usage:
#   find_package(FFTW [REQUIRED])
#
# It sets the following variables:
#   FFTW_FOUND
#   FFTW_INCLUDES
#   FFTW_LIBRARIES


find_path(FFTW_INCLUDES fftw3.h)

find_library(FFTW_LIBRARIES NAMES fftw3)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFTW DEFAULT_MSG
                                  FFTW_INCLUDES FFTW_LIBRARIES)

mark_as_advanced(FFTW_INCLUDES FFTW_LIBRARIES)
