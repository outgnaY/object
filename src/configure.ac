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

trylibeventdir=""
AC_ARG_WITH(libevent,
       [  --with-libevent=PATH     Specify path to libevent installation ],
       [
                if test "x$withval" != "xno" ; then
                        trylibeventdir=$withval
                fi
       ]
)

dnl ------------------------------------------------------
dnl libevent detection.  swiped from Tor.  modified a bit.

LIBEVENT_URL=https://www.monkey.org/~provos/libevent/

AC_CACHE_CHECK([for libevent directory], ac_cv_libevent_dir, [
  saved_LIBS="$LIBS"
  saved_LDFLAGS="$LDFLAGS"
  saved_CPPFLAGS="$CPPFLAGS"
  le_found=no
  for ledir in $trylibeventdir "" $prefix /usr/local ; do
    LDFLAGS="$saved_LDFLAGS"
    LIBS="-levent $saved_LIBS"

    # Skip the directory if it isn't there.
    if test ! -z "$ledir" -a ! -d "$ledir" ; then
       continue;
    fi
    if test ! -z "$ledir" ; then
      if test -d "$ledir/lib" ; then
        LDFLAGS="-L$ledir/lib $LDFLAGS"
      else
        LDFLAGS="-L$ledir $LDFLAGS"
      fi
      if test -d "$ledir/include" ; then
        CPPFLAGS="-I$ledir/include $CPPFLAGS"
      else
        CPPFLAGS="-I$ledir $CPPFLAGS"
      fi
    fi
    # Can I compile and link it?
    AC_TRY_LINK([#include <sys/time.h>
#include <sys/types.h>
#include <event.h>], [ event_init(); ],
       [ libevent_linked=yes ], [ libevent_linked=no ])
    if test $libevent_linked = yes; then
       if test ! -z "$ledir" ; then
         ac_cv_libevent_dir=$ledir
         _myos=`echo $target_os | cut -f 1 -d .`
         AS_IF(test "$SUNCC" = "yes" -o "x$_myos" = "xsolaris2",
               [saved_LDFLAGS="$saved_LDFLAGS -Wl,-R$ledir/lib"],
               [AS_IF(test "$GCC" = "yes",
                     [saved_LDFLAGS="$saved_LDFLAGS -Wl,-rpath,$ledir/lib"])])
       else
         ac_cv_libevent_dir="(system)"
       fi
       le_found=yes
       break
    fi
  done
  LIBS="$saved_LIBS"
  LDFLAGS="$saved_LDFLAGS"
  CPPFLAGS="$saved_CPPFLAGS"
  if test $le_found = no ; then
    AC_MSG_ERROR([libevent is required.  You can get it from $LIBEVENT_URL

      If it's already installed, specify its path using --with-libevent=/dir/
])
  fi
])
LIBS="-levent $LIBS"
if test $ac_cv_libevent_dir != "(system)"; then
  if test -d "$ac_cv_libevent_dir/lib" ; then
    LDFLAGS="-L$ac_cv_libevent_dir/lib $LDFLAGS"
    le_libdir="$ac_cv_libevent_dir/lib"
  else
    LDFLAGS="-L$ac_cv_libevent_dir $LDFLAGS"
    le_libdir="$ac_cv_libevent_dir"
  fi
  if test -d "$ac_cv_libevent_dir/include" ; then
    CPPFLAGS="-I$ac_cv_libevent_dir/include $CPPFLAGS"
  else
    CPPFLAGS="-I$ac_cv_libevent_dir $CPPFLAGS"
  fi
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
