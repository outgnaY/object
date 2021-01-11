#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([obj], [1.0])
AC_CONFIG_SRCDIR([obj.c])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([include/obj_config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 gettimeofday])
AC_CHECK_FUNCS([getopt_long], [AC_DEFINE(HAVE_GETOPT_LONG, 1, [Define to 1 if support getopt_long])])

dnl Check if we're a little-endian or a big-endian system
AC_C_BIGENDIAN(
    [AC_DEFINE(ENDIAN_BIG, 1, [machine is bigendian])],
    [AC_DEFINE(ENDIAN_LITTLE, 1, [machine is littleendian])],
    [AC_MSG_ERROR([Cannot detect endianness. Must pass ac_cv_c_bigendian={yes,no} to configure.])]
)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
